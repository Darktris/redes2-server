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
#include "tcpsocket.h"
#include "server.h"

pthread_mutex_t mutex_conn;
fd_set connections, readable;

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
	i=FD_ISSET(socketd, &connections);
	pthread_mutex_unlock(&mutex_conn);
	return i;
}

int connection_block(int socketd) {
	return rmvConnection(socketd);
}

int connection_unblock(int socketd) {
	if(!isConnected(socketd)) {
		return addConnection(socketd);
	}
	return -1;
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
	pthread_mutex_init(&mutex_conn, NULL);
	if(server_tcpsocket_open(port, &socketd) < 0) {
		return -1;
	}
	FD_ZERO(&connections);
	FD_SET(socketd, &connections);

	while(1) {
		readable = connections;
		if(select(FD_SETSIZE, &readable,NULL,NULL,NULL) < 0) {
			return -2;
		}
		for(i=0; i < FD_SETSIZE; i++) {
			if(isConnected(i)) {

				/* Atencion a conexion entrante */
				if(i==socketd) {
					if(tcpsocket_accept(socketd,&args)) {
						return -3;
					}
                    printf("Conexion aceptada");
					addConnection(args.acceptd);
				} 
				/* Atencion al resto de sockets */
				else {
                    thread_data = malloc(sizeof(conn_data));
                    thread_data->msg = malloc(DATA_SIZE);
                    bzero(thread_data->msg,DATA_SIZE);
                    
					switch(tcpsocket_rcv(i, data, DATA_SIZE, &len)) {
						case TCPCONN_CLOSED:
							rmvConnection(i);
							close(i);
							break;
						case TCPOK:
                            printf("%s", data);
							//connection_block(i);
							thread_data->len=len;
                            thread_data->more=more;
                            pthread_create(&t, NULL, handler, thread_data);
							break;
						case TCPERR_RECV:
                            perror("");
							return -4;
                        case TCPERR_ARGS:
                            return -5;
					}
				}
			}
		}
	}
}
