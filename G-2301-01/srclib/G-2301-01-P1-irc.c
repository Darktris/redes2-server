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
        set_nick(data->socketd, nick);
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
                    sprintf(comm, ":%s NICK :%s\r\n", prefix2, nick);
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
    switch(IRCTADUser_Add(user, nick, realname, NULL, server, get_ip_from_connection(data->socketd))) {
        case IRC_OK:
            set_user(data->socketd,user);
            syslog(LOG_INFO, "user: User successfully added");
            break;
        default:
            syslog(LOG_INFO, "Error IRCTADUser_Add");
    }

    IRCMsg_RplWelcome (&comm,IRCSVR_NAME, nick,nick, user, server);
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
    char comm[512], *comm2;
    time_t ts;
    IRCParse_List (command, &prefix, &channel, &target);
    IRCTADChan_GetList (&list, &n, NULL);
    for(i=0;i<n;i++) {
        if(channel!=NULL) {
            if(strcmp(channel,list[i])==0) {
                topic = IRCTADChan_GetTopic(channel, &ts);
                u = IRCTADChan_GetNumberOfUsers(channel);
                //IRCMsg_RplList (&comm, IRCSVR_NAME, get_nick(data->socketd), list[i], "0",topic);
                sprintf(comm, ":%s 322 %s %s %ld :%s\r\n", IRCSVR_NAME, get_nick(data->socketd), list[i],u, topic?topic:NULL); 
                tcpsocket_snd(data->socketd, comm, strlen(comm));
            }    
        } else {
            topic = IRCTADChan_GetTopic(list[i], &ts);
            u = IRCTADChan_GetNumberOfUsers(list[i]);
            //IRCMsg_RplList (&comm, IRCSVR_NAME, get_nick(data->socketd), list[i],(char* )10 ,topic);
            sprintf(comm, ":%s 322 %s %s %ld :%s\r\n", IRCSVR_NAME, get_nick(data->socketd), list[i],u, topic?topic:""); 
            tcpsocket_snd(data->socketd, comm, strlen(comm));
        }
    }

    IRCMsg_RplListEnd(&comm2, IRCSVR_NAME,get_nick(data->socketd));   
    tcpsocket_snd(data->socketd, comm2, strlen(comm2));
    free(comm2);
    //free(comm);

}

int ping(char* command, void* more) {
    conn_data* data = (conn_data*) more;
    char *msg, comm[512]={0}, *prefix, *server, *server2;
    IRCParse_Ping (command, &prefix, &server, &server2);
    //IRCMsg_Pong (&comm,IRCSVR_NAME,server,server2);
    sprintf(comm, ":%s PONG %s :%s\r\n", IRCSVR_NAME, IRCSVR_NAME,  server); 
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
    if(channel==NULL) {
        syslog(LOG_INFO, "join: no channel");
        IRCMsg_ErrNeedMoreParams (&comm, IRCSVR_NAME, get_nick(data->socketd), command);
        tcpsocket_snd(data->socketd, comm ,strlen(comm));
        if(comm) free(comm);
        if(prefix) free(prefix);
        if(key) free(key);
        if(msg) free(msg);
    } else {
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
                IRCMsg_Join (&comm, prefix2, NULL, NULL, channel);
                tcpsocket_snd(data->socketd, comm, strlen(comm));
                if(comm) free(comm);
                if(prefix) free(prefix);
                if(key) free(key);
                if(msg) free(msg);
                if(channel) free(channel);
                if(prefix2) free(prefix2);
                break; 

        }
    }
}

int privmsg(char* command, void* more) {
    conn_data* data = (conn_data*) more;
    char* prefix, *target, *msg, *prefix2, *comm;
    char** list;
    long n, i;
    int socketd;

    IRCParse_Privmsg (command, &prefix, &target, &msg);
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
            syslog(LOG_INFO, "part: invalid user");
            break;
        default:
            IRC_ComplexUser1459 (&prefix2, get_nick(data->socketd), get_user(data->socketd), IRCTADUser_GetHostByUser(get_user(data->socketd)), NULL);
            IRCMsg_Part (&comm, prefix2, channel, msg);
            tcpsocket_snd(data->socketd, comm, strlen(comm));
            if(IRCTAD_ListUsersOnChannel (channel, &list, &n)!=IRCERR_NOVALIDCHANNEL) {
                for(i=0;i<n;i++) {
                    socketd = get_socketd(list[i]);
                    connection_block(socketd);
                    tcpsocket_snd(socketd, comm, strlen(comm));
                    connection_unblock(socketd);
 }             
            }else {
                syslog(LOG_INFO,"part: novalidchannel");
            }
            
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
    if(l) IRCMsg_RplNamReply (&comm, IRCSVR_NAME, get_nick(data->socketd), "=", channel, l);
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
    char* prefix2, comm[512];
    long mode;
    conn_data* data = (conn_data*) more;
    IRCParse_Topic (command, &prefix, &channel, &topic);
    printf("topic = %s", topic);
    mode = IRCTAD_GetUserModeOnChannel (channel, get_user(data->socketd));
    IRC_ComplexUser1459 (&prefix2, get_nick(data->socketd), get_user(data->socketd), IRCTADUser_GetHostByUser(get_user(data->socketd)), NULL);
    /* Set topic */
    if(topic) {
        if((mode&IRCUMODE_LOCALOPERATOR==IRCUMODE_LOCALOPERATOR)) {
            IRCTADChan_SetTopic (channel,topic);
            //IRCMsg_Topic(&comm, prefix2, channel,topic);
            sprintf(comm, ":%s TOPIC %s :%s\r\n", prefix2, channel, topic);
            syslog(LOG_INFO, "topic: rply=%s", comm);
            tcpsocket_snd(data->socketd, comm, strlen(comm));
        }else {
            syslog(LOG_INFO, "topic: no permss");
        } 
    } else { /*Get topic */
        time_t t;
            topic = IRCTADChan_GetTopic(channel, &t);
            //IRCMsg_RplTopic(&comm, prefix2, channel,topic);
            sprintf(comm, ":%s TOPIC %s :%s\r\n", prefix2, channel, topic);
            syslog(LOG_INFO, "topic: rply=%s", comm);
            tcpsocket_snd(data->socketd, comm, strlen(comm));

    }
}

int whois(char* command, void* more) {
    char* prefix, *channel, *topic;
    char* prefix2, *comm;
    long mode;
    conn_data* data = (conn_data*) more;

}

int quit(char* command, void* more) {
    char* prefix, *msg;
    char* prefix2, *comm;
    char** list, **users;
    long n, u, i, j;
    int socketd;
    conn_data* data = (conn_data*) more;
    IRCParse_Quit (command, &prefix, &msg);
    IRC_ComplexUser1459 (&prefix2, get_nick(data->socketd), get_user(data->socketd), IRCTADUser_GetHostByUser(get_user(data->socketd)), NULL);
    IRCMsg_Quit (&comm, prefix2, msg);
    IRCTAD_ListChannelsOfUser (get_user(data->socketd), &list, &n);
   for(i=0;i<n;i++) {
        IRCTAD_ListUsersOnChannel (list[i], &users, &u);
        for(j=0;j<u;j++) {
            socketd = get_socketd(users[j]);
            if(socketd!=data->socketd) {
                connection_block(socketd);
                tcpsocket_snd(socketd, comm, strlen(comm));
                connection_unblock(socketd);
            }
        }
        IRCTAD_FreeListUsersOnChannel (users, u);
    }
   
    IRCTAD_Quit(get_user(data->socketd));
    set_user(data->socketd, NULL);
    set_nick(data->socketd, NULL);
}
