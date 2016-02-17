#include <G-2301-01-P1-irc.h>
#include <redes2/irc.h>
#include <syslog.h>
#include <G-2301-01-P1-server.h>
#include "G-2301-01-P1-irc_server.h" 
int nick(char* command, void* more) {
    char* prefix, *nick;
    conn_data* data = (conn_data*) more;
    
    if(get_user(data->socketd)==NULL) {    
        syslog(LOG_INFO, "Nick called");
        IRCParse_Nick (command,&prefix,&nick);
        set_nick(data->socketd, nick);
        syslog(LOG_INFO, "nick: prefix=%s nick=%s", prefix, nick);
    }else {

    }
}

int user(char* command, void*more) {
    char* prefix, *user, *modehost, *server, *realname,  *comm=NULL, *nick;
    conn_data* data = (conn_data*) more;
    char *msg;
    nick = get_nick(data->socketd);
    IRCParse_User (command, &prefix, &user, &modehost, &server, &realname);
    set_user(data->socketd,user);
    syslog(LOG_INFO, "user: prefix=%s user=%s mode=%s server=%s realn=%s", prefix, user, modehost, server, realname);
    switch(IRCTADUser_Add(user, nick, realname, NULL, server, get_ip_from_connection(data->socketd))) {
        case IRC_OK:
            syslog(LOG_INFO, "user: User successfully added");
            break;
        default:
            syslog(LOG_INFO, "Error IRCTADUser_Add");
    }

    IRCMsg_RplWelcome (&comm,IRCSVR_NAME, nick,nick, user, server);
    syslog(LOG_INFO, "user: Reply=[%s]", comm);
    tcpsocket_snd(data->socketd, comm, strlen(comm));
}

int list(char* command, void* more) {
    char* prefix, *channel, *target;
    conn_data* data = (conn_data*) more;
    char *msg;
    

}

int ping(char* command, void* more) {
    conn_data* data = (conn_data*) more;
    char *msg, *comm, *prefix, *server, *server2;
    IRCParse_Ping (command, &prefix, &server, &server2);
    syslog(LOG_INFO, "ping: prefix=%s server=%s server2=%s", prefix, server, server2);
    IRCMsg_Pong (&comm,IRCSVR_NAME,server,server2);
    syslog(LOG_INFO, "ping: Sending=[%s]", comm);
    tcpsocket_snd(data->socketd, comm, strlen(comm));
}

int pong(char* command, void* more) {
    return 0;
}

int join(char* command, void* more) {
    conn_data* data = (conn_data*) more;
    char* prefix, *channel, *key, *msg;
    IRCParse_Join (command, &prefix, &channel, &key, &msg);
    switch(IRCTAD_JoinChannel (channel, get_user(data->socketd), "w", key)) {
        case IRCERR_NOVALIDUSER:
            break;
        case IRCERR_NOVALIDCHANNEL:
            break;
        case IRCERR_USERSLIMITEXCEEDED:
            break;
        case IRCERR_NOENOUGHMEMORY:
            break;
        case IRCERR_INVALIDCHANNELNAME:
            break;
        case IRCERR_NAMEINUSE:
            break;
        case IRCERR_BANEDUSERONCHANNEL:
            break;
        case IRCERR_NOINVITEDUSER:
            break;
        case IRC_OK:
            /* Devolver mensaje Join con prefix el prefix del usuario */
           break; 
    
    }
}
int no_command(char* command, void* more) {
    syslog(LOG_INFO, "NYI: %s", command);
    return 0;
}
