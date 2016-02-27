#include <G-2301-01-P1-irc.h>
#include <redes2/irc.h>
#include <syslog.h>
#include <G-2301-01-P1-server.h>
#include <G-2301-01-P1-tools.h>
#include "G-2301-01-P1-irc_server.h" 

int nick(char* command, void* more) {
    char* prefix, *nick, comm[512]={0}, *prefix2;
    conn_data* data = (conn_data*) more;
    IRCParse_Nick (command,&prefix,&nick);

    if(get_user(data->socketd)==NULL) {    
        syslog(LOG_INFO, "Nick called");
        set_nick(data->socketd, nick);
        syslog(LOG_INFO, "nick: prefix=%s nick=%s", prefix, nick);
        if(nick) free(nick);
        if(prefix) free(prefix);
    }else {
        
          switch(IRCTADUser_SetNickByUser (nick, get_user(data->socketd))) {
               case IRCERR_INVALIDNICK:
                    syslog(LOG_INFO, "nick: invalid nick");
                    break;
               case IRC_OK:
                    IRC_ComplexUser1459 (&prefix2, get_nick(data->socketd), get_user(data->socketd), IRCTADUser_GetHostByUser(get_user(data->socketd)), NULL);
                    //IRCMsg_Nick(&comm, prefix2, nick);
                    sprintf(comm, "%s NICK :%s\r\n", prefix2, nick);
                    tcpsocket_snd(data->socketd, comm, strlen(comm));
                    set_nick(data->socketd, nick);
                    break;
                default: 
                    syslog(LOG_INFO, "nick: invalid user");
                    break;
           }
        

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
    switch(IRCTADUser_Add(get_user(data->socketd), nick, realname, NULL, server, get_ip_from_connection(data->socketd))) {
        case IRC_OK:
            syslog(LOG_INFO, "user: User successfully added");
            break;
        default:
            syslog(LOG_INFO, "Error IRCTADUser_Add");
    }

    IRCMsg_RplWelcome (&comm,IRCSVR_NAME, nick,nick, user, server);
    syslog(LOG_INFO, "user: Reply=[%s]", comm);
    tcpsocket_snd(data->socketd, comm, strlen(comm));
    if(prefix) free(prefix);
    if(user) free(user);
    if(modehost) free(modehost);
    if(server) free(server);
    if(realname) free(realname);
    if(comm) free(comm);
}

int list(char* command, void* more) {
    char* prefix, *channel, *target;
    char** list;
    long n, i, u;
    char* topic, num[10];
    conn_data* data = (conn_data*) more;
    char * comm;
    time_t ts;
    IRCParse_List (command, &prefix, &channel, &target);
    syslog(LOG_INFO, "list: prefix=%s channel=%s target=%s", prefix, channel, target);
    IRCTADChan_GetList (&list, &n, NULL);
    IRCTAD_ShowAll();
    for(i=0;i<n;i++) {
        printf("\n %s \n", list[i]);
        if(channel!=NULL) {
            if(strcmp(channel,list[i])==0) {
                topic = IRCTADChan_GetTopic(channel, &ts);
                sprintf(num, "%ld", u);
                IRCMsg_RplList (&comm, IRCSVR_NAME, get_nick(data->socketd), list[i], "0",topic);
                syslog(LOG_INFO, "list: rply=%s", comm);
                tcpsocket_snd(data->socketd, comm, strlen(comm));
                free(comm);
            }    
        } else {
            topic = IRCTADChan_GetTopic(list[i], &ts);
            topic = "Default topic biatch";
            IRCMsg_RplList (&comm, IRCSVR_NAME, get_nick(data->socketd), list[i],(char* )10 ,topic);
            syslog(LOG_INFO, "list: rply=%s", comm);
            tcpsocket_snd(data->socketd, comm, strlen(comm));
            free(comm);
        }
    }

    IRCMsg_RplListEnd(&comm, IRCSVR_NAME,get_nick(data->socketd));   
    syslog(LOG_INFO, "list: rply=%s", comm);
    tcpsocket_snd(data->socketd, comm, strlen(comm));
    free(comm);

}

int ping(char* command, void* more) {
    conn_data* data = (conn_data*) more;
    char *msg, comm[512]={0}, *prefix, *server, *server2;
    IRCParse_Ping (command, &prefix, &server, &server2);
    syslog(LOG_INFO, "ping: prefix=%s server=%s server2=%s", prefix, server, server2);
    //IRCMsg_Pong (&comm,IRCSVR_NAME,server,server2);
    sprintf(comm, ":%s PONG %s :%s\r\n", IRCSVR_NAME, IRCSVR_NAME,  server); 
    syslog(LOG_INFO, "ping: Sending=[%s]", comm);
    tcpsocket_snd(data->socketd, comm, strlen(comm));
    //if(comm) free(comm);
    if(prefix) free(prefix);
    if(server) free(server);
    if(server2) free(server2);
}

int pong(char* command, void* more) {
    return 0;
}

int join(char* command, void* more) {
    conn_data* data = (conn_data*) more;
    char* prefix, *prefix2, *channel, *key, *msg, *comm, *mode;
    IRCParse_Join (command, &prefix, &channel, &key, &msg);
    syslog(LOG_INFO, "join: prefix=%s chan=%s key=%s msg=%s user=%s", prefix, channel, key, msg, get_user(data->socketd));
    syslog(LOG_INFO, "join: chan=%lu user=%lu", strlen(channel), strlen(get_user(data->socketd)));
    printf("user2 %s\n", get_user(data->socketd));
    switch(IRCTAD_JoinChannel(channel,get_user(data->socketd),"w", NULL)) {
        case IRCERR_NOVALIDUSER:
            syslog(LOG_INFO, "join: novaliduser");
            break;
        case IRCERR_NOVALIDCHANNEL:
            syslog(LOG_INFO, "join: novalidchannel");
            break;
        case IRCERR_USERSLIMITEXCEEDED:
            syslog(LOG_INFO, "join: usrlimit");
            break;
        case IRCERR_NOENOUGHMEMORY:
            syslog(LOG_INFO, "join: nomem");
            break;
        case IRCERR_INVALIDCHANNELNAME:
            syslog(LOG_INFO, "join: invalidchannelname");
            break;
        case IRCERR_NAMEINUSE:
            syslog(LOG_INFO, "join: nameinuse");
            break;
        case IRCERR_BANEDUSERONCHANNEL:
            syslog(LOG_INFO, "join: banned");
            break;
        case IRCERR_NOINVITEDUSER:
            syslog(LOG_INFO, "join: noinviteuser");
            break;
        case IRC_OK:
            /* Devolver mensaje Join con prefix el prefix del usuario */
            // prefijo complejo  + mensaje join + rpl_topic + rpl_namereply
            syslog(LOG_INFO, "join: joined successfully");
            IRC_ComplexUser1459 (&prefix2, get_nick(data->socketd), get_user(data->socketd), IRCTADUser_GetHostByUser(get_user(data->socketd)), NULL);
            syslog(LOG_INFO, "user: prefix=%s", prefix2);
            IRCMsg_Join (&comm, prefix2, NULL, NULL, channel);
            syslog(LOG_INFO, "user: join_rply=%s",comm);
            tcpsocket_snd(data->socketd, comm, strlen(comm));
            printf("HEEEEEY");
           break; 
    
    }
    syslog(LOG_INFO, "join: finished");
}

int privmsg(char* command, void* more) {
    conn_data* data = (conn_data*) more;
    char* prefix, *target, *msg, *prefix2, *comm;
    char** list;
    long n, i;
    int socketd;

    IRCParse_Privmsg (command, &prefix, &target, &msg);
    syslog(LOG_INFO, "privmsg: prefix=%s, tar=%s msg=%s", prefix, target, msg);
    IRC_ComplexUser1459 (&prefix2, get_nick(data->socketd), get_user(data->socketd), IRCTADUser_GetHostByUser(get_user(data->socketd)), NULL);
    IRCMsg_Privmsg (&comm, prefix2,target,msg);
    if(IRCTAD_ListUsersOnChannel (target, &list, &n)!=IRCERR_NOVALIDCHANNEL) {
        for(i=0;i<n;i++) {
            socketd = get_socketd(list[i]);
            if(socketd!=data->socketd) {
                connection_block(socketd);
                tcpsocket_snd(socketd, comm, strlen(comm));
                connection_unblock(socketd);
            }
        }
    } else {
        if(get_socketd(target)!=0) {
            syslog(LOG_INFO, "privmsg: sending=%s", comm);
            tcpsocket_snd(get_socketd(target), comm, strlen(comm));
        } else {
            syslog(LOG_INFO,"privmsg: no dest user");
        }
    }

}


int mode(char* command, void* more) {
    conn_data* data = (conn_data*) more;
    char* prefix, *channel, *mode, *user;
    IRCParse_Mode(command, &prefix, &channel, &mode, &user);
    switch(IRCTADChan_SetMode(channel, mode) ){
            case IRCERR_INVALIDCHANNELNAME:
                break;
            case IRC_OK:
             exit(0);   
            } 
}


int part(char* command, void* more) {
    conn_data* data = (conn_data*) more;
    char*comm, *prefix, *prefix2, *channel, *msg;
    char **list;
    long n, i;
    int socketd;
    IRCParse_Part(command, &prefix, &channel, &msg);
    switch(IRCTAD_PartChannel (channel, get_user(data->socketd))) {
        case IRCERR_INVALIDUSER:
            break;
        default:
            IRC_ComplexUser1459 (&prefix2, get_nick(data->socketd), get_user(data->socketd), IRCTADUser_GetHostByUser(get_user(data->socketd)), NULL);
            IRCMsg_Part (&comm, prefix2, channel, msg);
            if(IRCTAD_ListUsersOnChannel (channel, &list, &n)!=IRCERR_NOVALIDCHANNEL) {
                for(i=0;i<n;i++) {
                    socketd = get_socketd(list[i]);
                    if(socketd!=data->socketd) {
                        connection_block(socketd);
                        tcpsocket_snd(socketd, comm, strlen(comm));
                        connection_unblock(socketd);
                    }
                }
            }
            tcpsocket_snd(data->socketd, comm ,strlen(comm));

    }

}

int names(char* command, void* more) {
    conn_data* data = (conn_data*) more;
    char *prefix, *channel, *target, *comm;
    char** list;
    long i,n;
    char *l;
    IRCParse_Names (command, &prefix, &channel, &target);
    if(IRCTAD_ListUsersOnChannel (channel, &list, &n)!=IRCERR_NOVALIDCHANNEL) {
        l=calloc(sizeof(MAX_NICK)*MAX_USERS_INCHAN, 1);
        for(i=0;i<n;i++) {
            l=strcat(l, list[i]);
        }
    }
    IRCMsg_RplNamReply (&comm, IRCSVR_NAME, get_nick(data->socketd), "=", channel, l);
    syslog(LOG_INFO, "names: rply %s, l=%s, prefx=%s channel=%s ", comm, l, prefix, channel);
    tcpsocket_snd(data->socketd, comm, strlen(comm));
    IRCMsg_RplEndOfNames (&comm, IRCSVR_NAME, get_nick(data->socketd), channel);
    tcpsocket_snd(data->socketd, comm, strlen(comm));
} 

int no_command(char* command, void* more) {
    syslog(LOG_INFO, "NYI: %s", command);
    return 0;
}

int topic(char* command, void* more) {
    char* prefix, *channel, *topic;
    char* prefix2, *comm;
    conn_data* data = (conn_data*) more;
    IRCParse_Topic (command, &prefix, &channel, &topic);
    switch(IRCTAD_GetUserModeOnChannel (channel, get_user(data->socketd))) {
        case IRCUMODE_OPERATOR:
        case IRCUMODE_LOCALOPERATOR:
            IRCTADChan_SetTopic (channel,topic);
            IRC_ComplexUser1459 (&prefix2, get_nick(data->socketd), get_user(data->socketd), IRCTADUser_GetHostByUser(get_user(data->socketd)), NULL);
            IRCMsg_Topic(&comm, prefix2, channel,topic);
            syslog(LOG_INFO, "topic: rply=%s",comm); 
            tcpsocket_snd(data->socketd, comm, strlen(comm));
            break;
        case IRCERR_NOVALIDCHANNEL:
            break;
    } 

}

int whois(char* command, void* more) {


}
