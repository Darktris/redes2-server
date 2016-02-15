#include "G-2301-01-P1-server.h"
#include <G-2301-01-P1-irc.h>
#include <G-2301-01-P1-daemon.h>
#include <redes2/irc.h>
#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>
#define NCOMMANDS 99999
typedef int(*comm_t)(char*, void*);

comm_t commands[NCOMMANDS];
int process_command(char* command, void* data);



void* handler(void* data) {
	conn_data* thread_data = (conn_data*) data;
	char* command="im not empty";
    char* err;
    if(pthread_detach(pthread_self())!=0) {
		perror("");
	}
	syslog(LOG_INFO, "Mensaje: %s",(char*) thread_data->msg);
	command = IRC_UnPipelineCommands (thread_data->msg, &err, NULL);
    syslog(LOG_INFO, "unpipelined: %s", command);
    while(command!=NULL&&strlen(command)>1) {
        process_command(command, data);
        if(err!=0) free(err); //??
        command = IRC_UnPipelineCommands(NULL, &err, command);
    }
    
    
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
