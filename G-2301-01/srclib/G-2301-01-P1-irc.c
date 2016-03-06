#include <G-2301-01-P1-irc.h>
#include <redes2/irc.h>
#include <syslog.h>
#include <G-2301-01-P1-server.h>
#include <G-2301-01-P1-tools.h>
#include "G-2301-01-P1-irc_server.h" 

int nick(char* command, void* more) {
    char* prefix, *nick, comm[512]={0}, *prefix2, *msg;
    conn_data* data = (conn_data*) more;
    IRCParse_Nick (command,&prefix,&nick, &msg);

    if(get_user(data->socketd)==NULL) {    
        set_nick(data->socketd, nick);
        if(nick) free(nick);
        if(prefix) free(prefix);
    }else {

        switch(IRCTADUser_SetNickByUser (nick, get_user(data->socketd))) {
            case IRCERR_INVALIDNICK:
                syslog(LOG_INFO, "nick: invalid nick");
                set_nick(data->socketd, nick);
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
                set_nick(data->socketd, nick);
                break;
        }


    }
}

int user(char* command, void*more) {
    char* prefix, *user, *modehost, *server, *realname,  *comm=NULL, *nick;
    conn_data* data = (conn_data*) more;
    char *msg;
    char t_user[20];
    nick = get_nick(data->socketd);
    IRCParse_User (command, &prefix, &user, &modehost, &server, &realname);
    switch(IRCTADUser_Add(user, nick, realname, NULL, server, get_ip_from_connection(data->socketd))) {
        case IRC_OK:
            set_user(data->socketd,user);
            syslog(LOG_INFO, "user: User successfully added user=%s",user);
            break;
        case IRCERR_NOENOUGHMEMORY:
            syslog(LOG_INFO, "user: malloc");
            break;
        case IRCERR_USERUSED: 
            set_user(data->socketd, user);
            syslog(LOG_INFO, "user: user being used user=%s",user);
            break;
        case IRCERR_NICKUSED:
            syslog(LOG_INFO, "user: nick being used");
            break;
        case IRCERR_REALNAMEUSED:
            syslog(LOG_INFO, "user: realn being used");
            break;
        case IRCERR_INVALIDUSER:
            syslog(LOG_INFO, "user: invalid user");
            break;
        case IRCERR_INVALIDNICK:
            syslog(LOG_INFO, "user: invalid nick");
            break;
        case IRCERR_INVALIDREALNAME:
            syslog(LOG_INFO, "user: invalid realname");
            break;
        case IRCERR_INVALIDHOST:
            syslog(LOG_INFO, "user: invalid host");
            break;
        case  IRCERR_INVALIDIP:
            syslog(LOG_INFO, "user: invalid ip");
            break;
        default:
            syslog(LOG_INFO, "Error IRCTADUser_Add");
    }

    IRCMsg_RplWelcome (&comm,IRCSVR_NAME, nick,nick, user, server);
    tcpsocket_snd(data->socketd, comm, strlen(comm));
    motd("motd\r\n", more);
    if(prefix) free(prefix);
    if(user) free(user);
    if(modehost) free(modehost);
    if(server) free(server);
    if(realname) free(realname);
    if(comm) free(comm);
    //IRCTAD_ShowAll();
}

int list(char* command, void* more) {
    char* prefix, *channel, *target;
    char** list;
    long n, i, u, mode;
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
            mode = IRCTADChan_GetMode(list[i]);
            if(!(mode&IRCMODE_SECRET)) { 
                topic = IRCTADChan_GetTopic(list[i], &ts);
                u = IRCTADChan_GetNumberOfUsers(list[i]);
                //IRCMsg_RplList (&comm, IRCSVR_NAME, get_nick(data->socketd), list[i],(char* )10 ,topic);
                sprintf(comm, ":%s 322 %s %s %ld :%s\r\n", IRCSVR_NAME, get_nick(data->socketd), list[i],u, topic?topic:""); 
                tcpsocket_snd(data->socketd, comm, strlen(comm));
            }
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
    IRCParse_Ping (command, &prefix, &server, &server2, &msg);
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
    char comm2[512], *type;
    int numparameters;
    char** parameters;
    IRCParse_Join (command, &prefix, &channel, &key, &msg);
    printf("join: channel=%s key=%s", channel, key);
    IRC_ComplexUser1459 (&prefix2, get_nick(data->socketd), get_user(data->socketd), IRCTADUser_GetHostByUser(get_user(data->socketd)), NULL);
    if(channel==NULL) {
        syslog(LOG_INFO, "join: no channel");
    //    IRCMsg_ErrNeedMoreParams (&comm, IRCSVR_NAME, get_nick(data->socketd), command);
        sprintf(comm2, ":%s 461 %s %s :Needed more parameters\r\n", IRCSVR_NAME, get_nick(data->socketd), "JOIN");
        syslog(LOG_INFO, "join: rply=%s",comm2);
        tcpsocket_snd(data->socketd, comm2 ,strlen(comm2));
        perror("");
        //if(comm) free(comm);
        if(prefix) free(prefix);
        if(key) free(key);
        if(msg) free(msg);
    } else {
        switch(IRCTAD_JoinChannel(channel,get_user(data->socketd),"w", key)) {
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
            case -9223372036316856320:
                printf("join: passw failed\n");
                IRCMsg_ErrBadChannelKey (&comm, prefix2, get_nick(data->socketd), channel);
                tcpsocket_snd(data->socketd, comm, strlen(comm));
                break;
            case IRC_OK:
                /* Devolver mensaje Join con prefix el prefix del usuario */
                // prefijo complejo  + mensaje join + rpl_topic + rpl_namereply
                syslog(LOG_INFO, "join: joined successfully");
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
    char* prefix, *target, *msg, *prefix2, *comm, *comm2;
    char* away;
    char** list;
    long n, i;
    int socketd;

    IRCParse_Privmsg (command, &prefix, &target, &msg);
    IRC_ComplexUser1459 (&prefix2, get_nick(data->socketd), get_user(data->socketd), IRCTADUser_GetHostByUser(get_user(data->socketd)), NULL);
    IRCMsg_Privmsg (&comm, prefix2,target,msg);
    if(IRCTAD_ListUsersOnChannel (target, &list, &n)!=IRCERR_NOVALIDCHANNEL) {
        for(i=0;i<n;i++) {
            socketd = get_socketd_byuser(list[i]);
            if(socketd!=data->socketd) {
                connection_block(socketd);
                tcpsocket_snd(socketd, comm, strlen(comm));
                connection_unblock(socketd);
            }
        }
    } else {
        if(get_socketd_bynick(target)!=0) {
            syslog(LOG_INFO,"privmsg: to user");
            away = IRCTAD_GetAway(target);
            if(away==NULL) {
                IRCMsg_RplAway(&comm2, IRCSVR_NAME, get_nick(data->socketd), target, away);
                tcpsocket_snd(data->socketd, comm2, strlen(comm2));
                free(comm2);
            }
            tcpsocket_snd(get_socketd_bynick(target), comm, strlen(comm));
        } else {
            syslog(LOG_INFO, "privmsg: no such nick");
            free(comm);
            IRCMsg_ErrNoSuchNick (&comm, prefix2, get_nick(data->socketd), target);
            tcpsocket_snd(data->socketd, comm, strlen(comm));
        }
    }

}

int mode(char* command, void* more) {
    conn_data* data = (conn_data*) more;
    char* prefix, *channel, *mode, *user;
    char* prefix2, *comm;
    long umode;
    IRCParse_Mode(command, &prefix, &channel, &mode, &user);
    syslog(LOG_INFO,"mode: channel=%s mode=%s user=%s", channel, mode, user);
    if(mode==NULL) return 0;
    umode=IRCTAD_GetUserModeOnChannel(channel, get_user(data->socketd));
    if(umode&IRCUMODE_LOCALOPERATOR==IRCUMODE_LOCALOPERATOR) {
        switch(IRCTADChan_SetMode(channel, mode) ){
            case IRCERR_INVALIDCHANNELNAME:
                break;
            case IRC_OK:
                switch(mode[1]) {
                    case '+':
                        switch (mode[2]) {
                            case 'k':
                                printf("Setting channel password %s:%s\n", channel, user);
                                IRCTADChan_SetPassword (channel, user);
                                break;
                            default:
                                break;
                        }
                        break;
                    case '-':
                        break;
                } 
                IRC_ComplexUser1459 (&prefix2, get_nick(data->socketd), get_user(data->socketd), IRCTADUser_GetHostByUser(get_user(data->socketd)), NULL);
                IRCMsg_Mode (&comm, prefix2,channel, mode, get_user(data->socketd));
                tcpsocket_snd(data->socketd, comm, strlen(comm));
        } 
    }
}

int part(char* command, void* more) {
    conn_data* data = (conn_data*) more;
    char*comm, *prefix, *prefix2, *channel, *msg;
    char comm2[512]={0};
    char **list;
    long n, i;
    int socketd;
    printf("%s", get_user(data->socketd));
    IRCParse_Part(command, &prefix, &channel, &msg);
    switch(IRCTAD_PartChannel (channel, get_user(data->socketd))) {
        case IRCERR_INVALIDUSER:
            syslog(LOG_INFO, "part: invalid user");
            break;
        case IRCERR_NOVALIDCHANNEL:
            sprintf(comm2, ":%s 403 %s %s :No such channel\r\n", IRCSVR_NAME, get_nick(data->socketd) ,channel);
            tcpsocket_snd(data->socketd, comm2, strlen(comm2));
            syslog(LOG_INFO,"part: novalidchannel");
            break;
        default:
            IRC_ComplexUser1459 (&prefix2, get_nick(data->socketd), get_user(data->socketd), IRCTADUser_GetHostByUser(get_user(data->socketd)), NULL);
            IRCMsg_Part (&comm, prefix2, channel, msg);
            tcpsocket_snd(data->socketd, comm, strlen(comm));
            if(IRCTAD_ListUsersOnChannel (channel, &list, &n)!=IRCERR_NOVALIDCHANNEL) {
                for(i=0;i<n;i++) {
                    socketd = get_socketd_byuser(list[i]);
                    connection_block(socketd);
                    tcpsocket_snd(socketd, comm, strlen(comm));
                    connection_unblock(socketd);
                }             
            }else {
                syslog(LOG_INFO,"part: invalid list");
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
    conn_data* data = (conn_data*) more;
    char *comm, type[12]={0};
    sscanf(command, ":%*s %s %*s\r\n", type);
    syslog(LOG_INFO, "No command: %s", command);
    IRCMsg_ErrUnKnownCommand (&comm, IRCSVR_NAME, get_nick(data->socketd), type);
    tcpsocket_snd(data->socketd, comm, strlen(comm));
    free(comm);
    return 0;
}

int topic(char* command, void* more) {
    char* prefix, *channel, *topic;
    char* prefix2, *comm;
    long mode, cmode;
    conn_data* data = (conn_data*) more;
    IRCParse_Topic (command, &prefix, &channel, &topic);
    printf("topic = %s", topic);
    mode = IRCTAD_GetUserModeOnChannel (channel, get_user(data->socketd));
    IRC_ComplexUser1459 (&prefix2, get_nick(data->socketd), get_user(data->socketd), IRCTADUser_GetHostByUser(get_user(data->socketd)), NULL);
    /* Set topic */
    if(topic) {
        cmode = IRCTADChan_GetMode (channel);
        if(cmode & IRCMODE_TOPICOP) {
            /* Topic protected */
            if((mode&IRCUMODE_LOCALOPERATOR==IRCUMODE_LOCALOPERATOR)) {
                IRCTADChan_SetTopic (channel,topic);
                IRCMsg_Topic(&comm, prefix2, channel,topic);
                syslog(LOG_INFO, "topic: rply=%s", comm);
                tcpsocket_snd(data->socketd, comm, strlen(comm));
            }else {
                /* No op */
                IRCMsg_ErrChanOPrivsNeeded (&comm, IRCSVR_NAME, get_nick(data->socketd), channel);
                tcpsocket_snd(data->socketd, comm, strlen(comm));
                syslog(LOG_INFO, "topic: no permss");
            } 
        } else {
            /* Topic NOT protected */
            IRCTADChan_SetTopic (channel,topic);
            IRCMsg_Topic(&comm, prefix2, channel,topic);
            syslog(LOG_INFO, "topic: rply=%s", comm);
            tcpsocket_snd(data->socketd, comm, strlen(comm));
        }
    } else { /*Get topic */
        time_t t;
        topic = IRCTADChan_GetTopic(channel, &t);
        if(topic==NULL) {
            IRCMsg_RplNoTopic (&comm, IRCSVR_NAME, get_nick(data->socketd), channel, "No hay Topic definido para el canal");

        }else {


            IRCMsg_RplTopic(&comm, prefix2,get_nick(data->socketd) , channel,topic);
        }
        syslog(LOG_INFO, "topic: rply=%s", comm);
        tcpsocket_snd(data->socketd, comm, strlen(comm));

    }
}

int whois(char* command, void* more) {
    char* prefix, nick2[20]={0}; 
    char* prefix2, *comm;
    long mode;
    conn_data* data = (conn_data*) more;
    char l[400]={0}, *maskarray;
    long n=1, i;
    char** list;

    //IRCParse_Whois(command, &prefix, &nick2, &maskarray);
    sscanf(command,"%*s %s", nick2);
    syslog(LOG_INFO, "whois: nick2=%s mask=%s", nick2, maskarray);
    if(strlen(nick2)<1) {
        IRCMsg_ErrNoNickNameGiven(&comm, IRCSVR_NAME, get_nick(data->socketd));
        tcpsocket_snd(data->socketd, comm, strlen(comm));
        free(comm);

    } else {
        if(IRCTADUser_GetUserByNick(nick2)==NULL) {
            IRCMsg_ErrNoSuchNick (&comm, IRCSVR_NAME, get_nick(data->socketd), nick2);
            tcpsocket_snd(data->socketd, comm, strlen(comm));
            free(comm);
        }else {
            IRCMsg_RplWhoIsUser(&comm, IRCSVR_NAME, get_nick(data->socketd), nick2, IRCTADUser_GetUserByNick(nick2), IRCTADUser_GetHostByNick(nick2), IRCTADUser_GetRealnameByNick(nick2));
            tcpsocket_snd(data->socketd, comm, strlen(comm));
            syslog(LOG_INFO, "whois: rply=%s", comm);
            if(comm) free(comm);

            IRCMsg_RplWhoIsServer(&comm, IRCSVR_NAME, get_nick(data->socketd), nick2, IRCSVR_NAME, IRCSVR_INFO);
            tcpsocket_snd(data->socketd, comm, strlen(comm));
            syslog(LOG_INFO, "whois: rply=%s", comm);
            if(comm) free(comm);

            IRCTAD_ListChannelsOfUser(IRCTADUser_GetUserByNick(nick2), &list, &n);    
            for(i=0;i<n;i++) {
                mode=IRCTAD_GetUserModeOnChannel(list[i], IRCTADUser_GetUserByNick(nick2));
                if(mode&IRCUMODE_LOCALOPERATOR==IRCUMODE_LOCALOPERATOR) strcat(l, "@");
                strcat(l, list[i]);
                strcat(l, " ");
            }
            IRCMsg_RplWhoIsChannels(&comm, IRCSVR_NAME, get_nick(data->socketd), nick2, l, NULL); 
            syslog(LOG_INFO, "whois: n=%ld l=%s", n, l);
            tcpsocket_snd(data->socketd, comm, strlen(comm));
            syslog(LOG_INFO, "whois: rply=%s", comm);
            if(comm) free(comm);


    }
            IRCMsg_RplEndOfWhoIs(&comm, IRCSVR_NAME, get_nick(data->socketd), nick2);
            tcpsocket_snd(data->socketd, comm, strlen(comm));
            syslog(LOG_INFO, "whois: rply=%s", comm);
            if(comm) free(comm);

    }
}
int quit(char* command, void* more) {
    char* prefix, *msg;
    char* prefix2, *comm;
    char** list, **users;
    long n, u, i, j;
    int socketd;

    //IRCTAD_ShowAll();
    conn_data* data = (conn_data*) more;
    IRCParse_Quit (command, &prefix, &msg);
    if(msg==NULL) msg="Como he venido, me voy.";
    IRC_ComplexUser1459 (&prefix2, get_nick(data->socketd), get_user(data->socketd), IRCTADUser_GetHostByUser(get_user(data->socketd)), NULL);
    IRCMsg_Quit (&comm, prefix2, msg);
    syslog(LOG_INFO, "quit: rply=%s, user=%s", comm, get_user(data->socketd));
    IRCTAD_ListChannelsOfUser (get_user(data->socketd), &list, &n);
    /*for(i=0;i<n;i++) {
        syslog(LOG_INFO, "quit: list[i]=%s", list[i]);
        IRCTAD_ListUsersOnChannel (list[i], &users, &u);
        for(j=0;j<u;j++) {
            socketd = get_socketd_byuser(users[j]);
            if(socketd!=data->socketd) {
                connection_block(socketd);
                tcpsocket_snd(socketd, comm, strlen(comm));
                connection_unblock(socketd);
            }
        }
        IRCTAD_FreeListUsersOnChannel (users, u);
        IRCTAD_PartChannel(list[i], get_user(data->socketd));
    }*/
    tcpsocket_snd(data->socketd, comm, strlen(comm));
    if(IRCTAD_Quit(get_user(data->socketd))<0) 
        syslog(LOG_INFO, "quit: no possible");
    //IRCTADUser_DeleteByUser(get_user(data->socketd));
    //IRCTAD_ShowAll();
    set_user(data->socketd, NULL);
    set_nick(data->socketd, NULL);
    connection_rmv(data->socketd);
    //close(data->socketd);
    tcpsocket_close(data->socketd);

}

int motd(char* command, void* more) {
    char* prefix, *comm;
    conn_data *data = (conn_data*) more;
    FILE* f_msg;
    char msg[256], *nick;
    nick = get_nick(data->socketd);
    f_msg = fopen(IRCSVR_MOTD, "r");

    IRCMsg_RplMotdStart(&comm, IRCSVR_NAME, get_nick(data->socketd), IRCSVR_NAME);
    tcpsocket_snd(data->socketd, comm, strlen(comm));
    free(comm);
    while(fgets(msg,256,f_msg)) {
        msg[strlen(msg)-1]=0;
        IRCMsg_RplMotd (&comm, IRCSVR_NAME, nick, msg);
        tcpsocket_snd(data->socketd, comm, strlen(comm));
        free(comm);
    }
    fclose(f_msg);
    IRCMsg_RplEndOfMotd(&comm, IRCSVR_NAME, nick);
    tcpsocket_snd(data->socketd, comm, strlen(comm));
    free(comm);


}

int away(char* command, void*more) {
    char* prefix, *comm, *msg;
    conn_data *data = (conn_data*) more;

    IRCParse_Away(command, &prefix, &msg);
    IRCMsg_RplNowAway(&comm, IRCSVR_NAME, get_nick(data->socketd));
    tcpsocket_snd(data->socketd, comm, strlen(comm));
    IRCTAD_SetAway(get_user(data->socketd), msg);

}

int kick(char* command, void* more) {
    char* prefix, *comm, *msg, *channel, *user, *comment, *prefix2;
    conn_data *data = (conn_data*) more;
    long n, i, mode;
    int socketd;
    char** list;
    IRCParse_Kick (command, &prefix, &channel, &user, &comment);
    mode = IRCTAD_GetUserModeOnChannel (channel, get_user(data->socketd));
    IRC_ComplexUser1459 (&prefix2, get_nick(data->socketd), get_user(data->socketd), IRCTADUser_GetHostByUser(get_user(data->socketd)), NULL);
    syslog(LOG_INFO, "kick: user=%s channel=%s", user, channel);
    if((mode&IRCUMODE_LOCALOPERATOR==IRCUMODE_LOCALOPERATOR)) {
        switch(IRCTAD_KickUserFromChannel (channel, user)) {
            default:
                IRCMsg_Kick (&comm, prefix2, channel, user, comment);
                syslog(LOG_INFO, "kick: rply=%s", comm);
                if(IRCTAD_ListUsersOnChannel (channel, &list, &n)!=IRCERR_NOVALIDCHANNEL) {
                    for(i=0;i<n;i++) {
                        socketd = get_socketd_byuser(list[i]);
                        connection_block(socketd);
                        tcpsocket_snd(socketd, comm, strlen(comm));
                        connection_unblock(socketd);
                    }                
                }
                tcpsocket_snd(get_socketd_byuser(user), comm, strlen(comm));

        }


    } else {
        IRCMsg_ErrChanOPrivsNeeded (&comm, IRCSVR_NAME, get_nick(data->socketd), channel);
        tcpsocket_snd(data->socketd, comm, strlen(comm));
    
    }

}
