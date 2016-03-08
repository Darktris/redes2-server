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
char* timeout;

char running=1;
pthread_t t_timeout;
pthread_mutex_t m_users;
pthread_mutex_t m_nicks;
pthread_mutex_t m_timeout;

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
    pthread_mutex_lock(&m_users);
    if(user==NULL) {
        if(users[socketd]) free(users[socketd]);  
        users[socketd]=NULL;
        pthread_mutex_unlock(&m_users);
        return IRCSVROK;
    } 
    if(strlen(user)>MAX_USERNAME) {
        pthread_mutex_unlock(&m_users);
        return IRCSVRERR_MAXLEN;
    }
    if(users[socketd]==NULL) {
        users[socketd]=malloc(sizeof(MAX_USERNAME));
        if(users[socketd]==NULL) {
            pthread_mutex_unlock(&m_users);
            return IRCSVRERR_MALLOC;
        } 
    }
    syslog(LOG_INFO,"set_user: socketd=%d user=%s", socketd, user);
    strcpy(users[socketd], user);
    pthread_mutex_unlock(&m_users);
    return IRCSVROK;
}

/**
  @brief Asocia a un socketd el nick
  @param socketd: Socket del usuario
  @param user: Nick del usuario
  @return IRCSVROK en caso adecuado, IRCSVERR (<0) en otro caso
*/
int set_nick(int socketd, char* nick) {
    pthread_mutex_lock(&m_nicks);
    if(nick==NULL) {
        if(nicks[socketd]) free(nicks[socketd]);
        nicks[socketd]=NULL;
        pthread_mutex_unlock(&m_nicks);
        return IRCSVROK; 
    }

    if(strlen(nick)>MAX_NICK) {
        pthread_mutex_unlock(&m_nicks);
        return IRCSVRERR_MAXLEN;
    }
    if(nicks[socketd]==NULL) {
        nicks[socketd]=malloc(sizeof(MAX_NICK)+1);
        if(nicks[socketd]==NULL) {
            pthread_mutex_unlock(&m_nicks);
            return IRCSVRERR_MALLOC;
        } 
    }
    pthread_mutex_unlock(&m_nicks);
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
    pthread_mutex_lock(&m_users);
    /* Zona critica */
    syslog(LOG_INFO, "get_socketd=%s", user);
    for(i=0;i<MAX_USERS;i++) {
        if(users[i]!=NULL) {
            if(strcmp(user, users[i]) == 0) {
                pthread_mutex_unlock(&m_users);
                return i;
            }
        }
    }
    pthread_mutex_unlock(&m_users);
    return 0;

}

/**
  @brief Busca el socket de un usuario por nick
  @param nick: Nick del usuario
  @return el socket del usuario
*/
int get_socketd_bynick(char* nick) {
    int i;
    if(nick==NULL) return 0;
    pthread_mutex_lock(&m_nicks);
    syslog(LOG_INFO, "get_socketd=%s", nick);
    for(i=0;i<MAX_USERS;i++) {
        if(nicks[i]!=NULL) {
            if(strcmp(nick, nicks[i]) == 0) {
                pthread_mutex_unlock(&m_nicks);
                return i;
            }
        }
    }
    pthread_mutex_unlock(&m_nicks);
    return 0;

}

/**
  @brief Devuelve el nick de un usuario
  @param socketd: El socket del usuario
  @return El nick del usuario
*/
char* get_nick(int socketd) {
    char* ret;
    pthread_mutex_lock(&m_nicks);
    ret = nicks[socketd];
    pthread_mutex_unlock(&m_nicks);
    return ret;
}

/**
  @brief Devuelve el user de un usuario
  @param socketd: El socket del usuario
  @return El user del usuario
*/
char* get_user(int socketd) {
    char* ret;
    pthread_mutex_lock(&m_users);
    ret = users[socketd];
    pthread_mutex_unlock(&m_users);
    return ret;
}

/**
  @brief Actualiza el contador de timeout de un socketd
  @param socketd: El socket del usuario
  @return IRCSVROK
*/
int update_timeout(int socketd) {
    pthread_mutex_lock(&m_timeout);
    timeout[socketd]=0;
    pthread_mutex_unlock(&m_timeout);
    return IRCSVROK;
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
    update_timeout(thread_data->socketd);
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
    } while(next!=NULL);
    
    
	//syslog(LOG_INFO, "Negra caderona <3");
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
    timeout=malloc(sizeof(char)*MAX_USERS);
    if(timeout==NULL) {
        free(users);
        free(nicks);
        return IRCSVRERR_MALLOC;
    }
    bzero(users, sizeof(char*)*MAX_USERS);
    bzero(nicks, sizeof(char*)*MAX_USERS);
    bzero(timeout, sizeof(char)*MAX_USERS);
    if(pthread_mutex_init(&m_users, NULL)) {
        free(users);
        free(nicks);
        free(timeout);
        return IRCSVRERR_MALLOC;
    }
    if(pthread_mutex_init(&m_nicks, NULL)) {
        free(users);
        free(nicks);
        free(timeout);
        pthread_mutex_destroy(&m_users);
        return IRCSVRERR_MALLOC;
    } 
    if(pthread_mutex_init(&m_users, NULL)) {
        free(users);
        free(nicks);
        free(timeout);
        pthread_mutex_destroy(&m_users);
        pthread_mutex_destroy(&m_nicks);
        return IRCSVRERR_MALLOC;
    }
    return IRCSVROK;
}

/**
  @brief Borra las estructuras auxiliares del servidor
*/
int free_memspace() {
    int i;
    for(i=0;i<MAX_USERS;i++) {
        if(users[i]) free(users[i]);
        if(nicks[i]) free(nicks[i]);
    }
    free(users);
    free(nicks);
    free(timeout);
    pthread_mutex_destroy(&m_timeout);
    pthread_mutex_destroy(&m_nicks);
    pthread_mutex_destroy(&m_nicks);
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

void* timeout_thread(void* data) {
    int i;
    conn_data d;
    char ping_msg[]="PING LAG1234567890\r\n";
    while(running) {
        printf("Timeout try\n");
        //pthread_mutex_lock(&m_timeout);
        //pthread_mutex_lock(&m_users);
        
        for(i=0;i<MAX_USERS;i++) {
            if((timeout[i]==1)&&get_user(i)) {
                connection_block(i);
                tcpsocket_snd(i, ping_msg, strlen(ping_msg));
                connection_unblock(i);
            }

            if((timeout[i]==2)&&get_user(i)) { 
                d.socketd=i;
                quit("QUIT :Timeout\r\n", &d);
            }
            (timeout[i])++;
        }
        //pthread_mutex_unlock(&m_users);
        //pthread_mutex_unlock(&m_timeout);
        sleep(IRCSVR_TIMEOUT);
    }
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

	if(init_memspace()<0) {
        printf("Critical error initializing structures\n");
        return -2;
    }
    if(argc!=2) return -1;
    set_do_on_disconnect(do_on_disconnect);
	//daemonize("G-2301-01-irc");
    if(pthread_create(&t_timeout, NULL, timeout_thread, NULL)!=0) {
        printf("Critical error initializing structures\n");
        return -2;
    }
	ret = server_launch(atoi(argv[1]), handler, NULL);
	printf("Retorno del servidor: %d\n",ret);
	syslog(LOG_INFO, "Retorno del servidor: %d",ret);
    server_stop();
    running = 0;
    free_memspace();
}
