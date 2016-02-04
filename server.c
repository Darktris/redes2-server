/* vim: set ts=4 sw=4 et: */
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <strings.h>
#include <signal.h>
#include "tcpsocket.h"
#include "server.h"

pthread_mutex_t mutex_conn;
fd_set connections, readable, blocked;

int addConnection(int socketd) {
	pthread_mutex_lock(&mutex_conn);
	FD_SET(socketd, &connections);
	pthread_mutex_unlock(&mutex_conn);
	return 0;
}

int rmvConnection(int socketd) {
	pthread_mutex_lock(&mutex_conn);
	FD_CLR(socketd, &connections);
	pthread_mutex_unlock(&mutex_conn);
	return 0;
}

int isConnected(int socketd) {
	int i=0;
	pthread_mutex_lock(&mutex_conn);
	i=FD_ISSET(socketd, &readable);
	pthread_mutex_unlock(&mutex_conn);
	return i;
}

int connection_block(int socketd) {
    pthread_mutex_lock(&mutex_conn);
    FD_SET(socketd, &blocked);
    pthread_mutex_unlock(&mutex_conn);
    return 0;
}

int connection_unblock(int socketd) {
    pthread_mutex_lock(&mutex_conn);
    FD_CLR(socketd, &blocked);
    pthread_mutex_unlock(&mutex_conn);
    return 0;
}

int connection_isblocked(int socketd) {
    int i;
    pthread_mutex_lock(&mutex_conn);
    i=FD_ISSET(socketd, &blocked);
    pthread_mutex_unlock(&mutex_conn);
    return i;
}


void sigint_handler(int sig) {
    pthread_mutex_destroy(&mutex_conn);
    exit(1);
}
int server_launch(uint16_t port, void*(*handler)(void*), void* more) {
	int socketd, conn_socket;
	int set_size=1, i=0;
	tcpsocket_args args;
	char* data;
	size_t len;
    pthread_t t;
    conn_data* thread_data;

    signal(SIGINT, sigint_handler);
	pthread_mutex_init(&mutex_conn, NULL);
	if(server_tcpsocket_open(port, &socketd) < 0) {
		return -1;
	}
	FD_ZERO(&connections);
    FD_ZERO(&blocked);
	FD_SET(socketd, &connections);

	while(1) {
        readable = connections;
		if(select(FD_SETSIZE, &readable,NULL,NULL,NULL) < 0) {
			return -2;
		}
		for(i=0; i < FD_SETSIZE; i++) {
			if(isConnected(i) && !connection_isblocked(i)) {

				/* Atencion a conexion entrante */
				if(i==socketd) {
					if(tcpsocket_accept(socketd,&args)) {
						return -3;
					}
                    printf("Conexion aceptada %d\n", args.acceptd);
					addConnection(args.acceptd);

				} 
				/* Atencion al resto de sockets */
				else {
                    thread_data = malloc(sizeof(conn_data));
                    thread_data->msg = malloc(DATA_SIZE);
                    bzero(thread_data->msg,DATA_SIZE);
                    thread_data->socketd=i; 
					switch(tcpsocket_rcv(i, thread_data->msg, DATA_SIZE, &thread_data->len)) {
						case TCPCONN_CLOSED:
							rmvConnection(i);
							close(i);
							break;
						case TCPOK:
                            printf("Conexion ya abierta %d\n", i);
                            thread_data->more=more;
							connection_block(i);
                            if(pthread_create(&t, NULL, handler, thread_data)!=0) {
                                return -6;
                            }
							break;
						case TCPERR_RECV:
							return -4;
                        case TCPERR_ARGS:
                            return -5;
					}
				}
			}
		}
	}
}
