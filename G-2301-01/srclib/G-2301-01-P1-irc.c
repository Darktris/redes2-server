#include <G-2301-01-P1-irc.h>
#include <redes2/irc.h>
#include <syslog.h>
#include <G-2301-01-P1-server.h>
int nick(char* command, void* more) {
    char* prefix, *nick;
    conn_data* data = (conn_data*) more;
    
    
    syslog(LOG_INFO, "Nick called");
    IRCParse_Nick (command,&prefix,&nick);

    switch(IRCTADUser_Add (nick, nick, nick, NULL, "", get_ip_from_connection(data->socketd))) {
        case IRC_OK:
            break;
        default:
            syslog(LOG_INFO, "Error in IRCTADUser_Add");
    
    }
    syslog(LOG_INFO, "nick: prefix=%s nick=%s", prefix, nick);
}

int user(char* command, void*more) {
    char* prefix, *user, *modehost, *server, *realname;
    conn_data* data = (conn_data*) more;
    IRCParse_User (command, &prefix, &user, &modehost, &server, &realname);
    syslog(LOG_INFO, "user: prefix=%s user=%s mode=%s server=%s realn=%s", prefix, user, modehost, server, realname);
    IRCMsg_RplWelcome (&command,prefix,"dani","", user, server);
    tcpsocket_snd(data->socketd, &command, strlen(command));
}

int no_command(char* command, void* more) {
    syslog(LOG_INFO, "NYI: %s", command);
    return 0;
}
