/* vim: set ts=4 sw=4 et: */
/**
  @file G-2301-01-P1-irc_server.c
  @brief Servidor IRC
  @author Sergio Fuentes  <sergio.fuentesd@estudiante.uam.es>
  @author Daniel Perdices <daniel.perdices@estudiante.uam.es>
  @date 2016/02/10
  */
#include "G-2301-01-P1-server.h"
#include "G-2301-01-P1-irc_server.h"
#include <G-2301-01-P1-irc.h>
#include <G-2301-01-P1-tools.h>
#include <G-2301-01-P1-daemon.h>
#include <redes2/irc.h>
#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>
typedef int(*comm_t)(char*, void*);

comm_t commands[NCOMMANDS];

char** users;
char** nicks;

void repair_command(char* command) {
    char* i=command;
    while(*i!='\r'&&*(i+1)!='\n') i++;
    i+=2;
    *i='\0';
}

/**
  @brief Asocia a un socketd el usuario
  @param socketd: Socket del usuario
  @param user: Nombre del usuario
  @return IRCSVROK en caso adecuado, IRCSVERR (<0) en otro caso
*/
int set_user(int socketd, char* user) {
    /*int sd = get_socketd_byuser(user);
    if(sd) set_user(sd, NULL);*/
    if(user==NULL) {
      if(users[socketd]) free(users[socketd]);  
      users[socketd]=NULL;
      return IRCSVROK;
    } 
    if(strlen(user)>MAX_USERNAME)
        return IRCSVRERR_MAXLEN;
    if(users[socketd]==NULL) {
        users[socketd]=malloc(sizeof(MAX_USERNAME));
       if(users[socketd]==NULL) {
            return IRCSVRERR_MALLOC;
       } 
    }
    syslog(LOG_INFO,"set_user: socketd=%d user=%s", socketd, user);
    strcpy(users[socketd], user);
    return IRCSVROK;
}

/**
  @brief Asocia a un socketd el nick
  @param socketd: Socket del usuario
  @param user: Nick del usuario
  @return IRCSVROK en caso adecuado, IRCSVERR (<0) en otro caso
*/
int set_nick(int socketd, char* nick) {
    /*int sd = get_socketd_bynick(nick);
    if(sd) set_nick(sd, NULL);*/
    if(nick==NULL) {
        if(nicks[socketd]) free(nicks[socketd]);
        nicks[socketd]=NULL;
        return IRCSVROK; 
    }

    if(strlen(nick)>MAX_NICK)
        return IRCSVRERR_MAXLEN;
    if(nicks[socketd]==NULL) {
        nicks[socketd]=malloc(sizeof(MAX_NICK)+1);
       if(nicks[socketd]==NULL) {
            return IRCSVRERR_MALLOC;
       } 
    }
    strcpy(nicks[socketd], nick);
    return IRCSVROK;
}

/**
  @brief Asocia a un socketd el nick
  @param socketd: Socket del usuario
  @param user: Nick del usuario
  @return IRCSVROK en caso adecuado, IRCSVERR (<0) en otro caso
*/
int get_socketd_byuser(char* user) {
    int i;
    if(user==NULL) return 0;
    syslog(LOG_INFO, "get_socketd=%s", user);
    for(i=0;i<MAX_USERS;i++) {
        if(users[i]!=NULL) {
            if(strcmp(user, users[i]) == 0) return i;
        }
    }
    return 0;

}

/**
  @brief Busca el socket de un usuario por nick
  @param nick: Nick del usuario
  @return el socket del usuario
*/
int get_socketd_bynick(char* nick) {
    int i;
    if(user==NULL) return 0;
    syslog(LOG_INFO, "get_socketd=%s", nick);
    for(i=0;i<MAX_USERS;i++) {
        if(nicks[i]!=NULL) {
            if(strcmp(nick, nicks[i]) == 0) return i;
        }
    }
    return 0;

}

/**
  @brief Devuelve el nick de un usuario
  @param socketd: El socket del usuario
  @return El nick del usuario
*/
char* get_nick(int socketd) {
    return nicks[socketd];
}

/**
  @brief Devuelve el user de un usuario
  @param socketd: El socket del usuario
  @return El user del usuario
*/
char* get_user(int socketd) {
    return users[socketd];
}

/**
  @brief Manejador de las conexiones
  @param data: Datos de la conexion y el mensaje TCP
  @return IRCSVR_OK
*/
void* handler(void* data) {
	conn_data* thread_data = (conn_data*) data;
	char* command="im not empty";
    char* next;
    if(pthread_detach(pthread_self())!=0) {
		perror("");
	}
	syslog(LOG_INFO, "Mensaje: [%s], len=%lu",(char*) thread_data->msg, strlen(thread_data->msg));
	next = IRC_UnPipelineCommands (thread_data->msg, &command, NULL);
    repair_command(command);
    syslog(LOG_INFO, "unpipelined: %s, len=%lu", command, strlen(command));
    do { 
        repair_command(command);
        if(strlen(command)>1) {
            process_command(command, data);
        }
        //if(command!=NULL) free(command); //??
        next = IRC_UnPipelineCommands(NULL, &command, next);
        syslog(LOG_INFO, "unpipelined: %s", command);
    } while(next!=NULL);
    
    
	syslog(LOG_INFO, "Negra caderona <3");
    connection_unblock(thread_data->socketd);
  	free(thread_data->msg);
  	free(thread_data);
    return IRCSVROK;	
}

void do_on_disconnect(void* data) {
	conn_data* thread_data = (conn_data*) data;
    quit("QUIT\r\n", data);
}
/**
  @brief Inicializa las llamadas de atencion de los comandos
*/
int init_commands() {
    int i=0;
    for(i=0;i<NCOMMANDS;i++)
        commands[i]=no_command;
    commands[NICK]=nick;
    commands[USER]=user;
    commands[LIST]=list;
    commands[PING]=ping;
    commands[PONG]=pong;
    commands[JOIN]=join;
    commands[PRIVMSG]=privmsg;
    commands[PART]=part;
    commands[NAMES]=names;
    commands[TOPIC]=topic;
    commands[QUIT]=quit;
    commands[MOTD]=motd;
    commands[AWAY]=away;
    commands[WHOIS]=whois;
    commands[MODE]=mode;
    commands[KICK]=kick;
}

/**
  @brief Inicializa las estructuras
*/
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

/**
  @brief Procesa un comando
  @param command: El comando
  @param data: Datos de las conexion
  @return IRCSVROK en caso adecuado, IRCSVERR (<0) en otro caso
*/
int process_command(char* command, void* data) {
    long ret;
    int i;
    syslog(LOG_INFO, "Command: [%s]", command);
    conn_data* d = (conn_data*) data;
    if(command==NULL) return -1;
    ret = IRC_CommandQuery (command);
    if(ret==IRCERR_NOCOMMAND||ret==IRCERR_UNKNOWNCOMMAND) {
        char *comm, type[12]={0};
        sscanf(command, "%s %*[^\r]\r\n", type);
        syslog(LOG_INFO, "No command: %s", command);
        IRCMsg_ErrUnKnownCommand (&comm, IRCSVR_NAME, get_nick(d->socketd), type);
        tcpsocket_snd(d->socketd, comm, strlen(comm));
        free(comm);

        syslog(LOG_INFO,"Unknown command: Go call Eloy");
        return -1;
    }

    return commands[ret](command, data);
}

/**
  @brief Llamada principal del servidor
  @param argc: Num de argumentos
  @param argv: Argumentos
  @return 0
*/
int main(int argc, char** argv) {
	int ret;
    init_commands();   
	init_memspace();
    if(argc!=2) return -1;
    set_do_on_disconnect(do_on_disconnect);
	//daemonize("G-2301-01-irc");
	ret = server_launch(atoi(argv[1]), handler, NULL);
	printf("Retorno del servidor: %d\n",ret);
	syslog(LOG_INFO, "Retorno del servidor: %d",ret);
    server_stop();
    close(3);
    perror("");
}
