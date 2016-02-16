#include "G-2301-01-P1-server.h"
#include <G-2301-01-P1-irc.h>
#include <G-2301-01-P1-daemon.h>
#include <redes2/irc.h>
#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>
#define NCOMMANDS 99999
#define MAX_USERS 100
#define MAX_USERNAME 20
#define MAX_NICK 20
#define IRCSVROK 0
#define IRCSVRERR_MALLOC -1
#define IRCSVRERR_MAXLEN -2
#define IRCSVRERR_ARGS -3
typedef int(*comm_t)(char*, void*);

comm_t commands[NCOMMANDS];
int process_command(char* command, void* data);

char** users;
char** nicks;

int set_user(int socketd, char* user) {
    if(user==NULL) 
        return IRCSVRERR_ARGS;
    if(strlen(user)>MAX_USERNAME)
        return IRCSVRERR_MAXLEN;
    if(users[socketd]==NULL) {
        users[socketd]=malloc(sizeof(MAX_USERNAME));
       if(users[socketd]==NULL) {
            return IRCSVRERR_MALLOC;
       } 
    }
    strcpy(users[socketd], user);
    return IRCSVROK;
}

int set_nick(int socketd, char* nick) {
    if(nick==NULL) {
        if(nicks[socketd]!=NULL) {
            free(nicks[socketd]);
        }
        nicks[socketd]=NULL;
        return IRCSVROK; 
    }

    if(strlen(nick)>MAX_NICK)
        return IRCSVRERR_MAXLEN;
    if(nicks[socketd]==NULL) {
        nicks[socketd]=malloc(sizeof(MAX_NICK));
       if(nicks[socketd]==NULL) {
            return IRCSVRERR_MALLOC;
       } 
    }
    strcpy(nicks[socketd], nick);
    return IRCSVROK;
}

char* get_nick(int socketd) {
    return nicks[socketd];
}

char* get_user(int socketd) {
    return users[socketd];
}

void* handler(void* data) {
	conn_data* thread_data = (conn_data*) data;
	char* command="im not empty";
    char* err;
    if(pthread_detach(pthread_self())!=0) {
		perror("");
	}
	syslog(LOG_INFO, "Mensaje: %s",(char*) thread_data->msg);
	command = IRC_UnPipelineCommands (thread_data->msg, &err, NULL);
    syslog(LOG_INFO, "unpipelined: %s; command=%s", command, err);
    do { 
        if(strlen(command)>1) {
            process_command(command, data);
        }
        if(err!=0) free(err); //??
        command = IRC_UnPipelineCommands(NULL, &err, command);
        syslog(LOG_INFO, "unpipelined: %s; command=%s", command, err);
    } while(command!=NULL);
    
    
	syslog(LOG_INFO, "Negra caderona <3");
    connection_unblock(thread_data->socketd);
	free(thread_data->msg);
	free(thread_data);
    return 0;	
}


int init_commands() {
    int i=0;
    for(i=0;i<NCOMMANDS;i++)
        commands[i]=no_command;
    commands[NICK]=nick;
    commands[USER]=user;
}

int init_memspace() {
    users=malloc(sizeof(char*)*MAX_USERS);
    if(users==NULL) 
        return IRCSVRERR_MALLOC;
    nicks=malloc(sizeof(char*)*MAX_USERS);
    if(nicks==NULL) {
        free(users);
        return IRCSVRERR_MALLOC;
    }
    bzero(users, sizeof(char*)*MAX_USERS);
    bzero(nicks, sizeof(char*)*MAX_USERS);
    return IRCSVROK;
}

int process_command(char* command, void* data) {
    long ret;
    int i;
    syslog(LOG_INFO, "Command: [%s]", command);

    if(command==NULL) return -1;
    ret = IRC_CommandQuery (command);
    if(ret==IRCERR_NOCOMMAND||ret==IRCERR_UNKNOWNCOMMAND) {
        syslog(LOG_INFO,"Unknown command: Go call Eloy");
        return -1;
    }

    return commands[ret](command, data);
}


int main(int argc, char** argv) {
	int ret;
    init_commands();   
	if(argc!=2) return -1;
	daemonize("G-2301-01-irc");
	ret = server_launch(atoi(argv[1]), handler, NULL);
	printf("Retorno del servidor: %d\n",ret);
	syslog(LOG_INFO, "Retorno del servidor: %d",ret);
    server_stop();
}
