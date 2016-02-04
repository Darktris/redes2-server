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
#include "tcp-socket.h"

pthread_mutex_t mutex_conn;
fd_set connections, readable;

int server(uint16_t port, void*(*handler)(void*)) {
	int socketd, conn_socket;
	int set_size=1, i=0;
	tcpsocket_args args;
    pthread_t t;
	char data[DATA_SIZE]={0};
	size_t len;

    /* Inicializacion de mutex del fd_set */
	pthread_mutex_init(&mutex_conn, NULL);
	if(openTCPSocket(port, &socketd) < 0) {
		return SERVERR_OPEN;
	}

    /* Inicializacion del fd_set */
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
					if(acceptTCPSocket(socketd,&args)) {
						return -3;
					}

					addConnection(args.acceptd);
				}
 
				/* Atencion al resto de sockets */
				else {
					//pthread_create
                    			bzero(data, DATA_SIZE);
					switch(rcvTCPSocket(i, data, DATA_SIZE, &len)) {
						case TCPCONN_CLOSED:
							rmvConnection(i);
							close(i);
							break;
						case TCPOK:
							blockConnection(i);
							if(sendTCPSocket(i, data, len) < 0) {
								return -5;
							}
							printf("%s [%d]\n", data, len);
							pthread_create(t, NULL, handler, conn_data);
							break;
						default:
							return -4;
					}
				}
			}
		}
	}
}


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

int blockConnection(int socketd) {
	return rmvConnection(socketd);
}

int unblockConnection(int socketd) {
	if(!isConnected(socketd)) {
		return addConnection(socketd);
	}
	return -1;
}
int main(int argc, char** argv) {
    printf("Resultado llamada al servidor %d\n",server(atoi(argv[1]), NULL));
}