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

int server(uint16_t port) {
	int socketd, conn_socket;
	int set_size=1, i=0;
	tcpsocket_args args;
	char data[65500]={0};
	size_t len;

	pthread_mutex_init(&mutex_conn, NULL);
	if(openTCPSocket(port, &socketd) < 0) {
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
			if(FD_ISSET(i, &readable)) {

				/* Atencion a conexion entrante */
				if(i==socketd) {
					if(acceptTCPSocket(socketd,&args)) {
						return -3;
					}

					FD_SET(args.acceptd, &connections);
				} 
				/* Atencion al resto de sockets */
				else {
					//pthread_create
                    bzero(data,65500);
					switch(rcvTCPSocket(i, data, 65500, &len)) {
						case TCPCONN_CLOSED:
							close(i);
							FD_CLR(i,&connections);
							break;
						case TCPOK:
							// Atencion del mensaje
							if(sendTCPSocket(i, "PONG", 4) < 0) {
								return -5;
							}
							printf("%s [%d]\n", data, len);
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
}

int rmvConnection(int socketd) {
	pthread_mutex_lock(&mutex_conn);
	FD_CLR(socketd, &connections);
	pthread_mutex_unlock(&mutex_conn);
}

int isConnected(int socketd) {
	pthread_mutex_lock(&mutex_conn);
	FD_ISSET(socketd, &connections);
	pthread_mutex_unlock(&mutex_conn);
}
int blockSocket(int socketd) {
	
	return 0;
}
int main(int argc, char** argv) {
    printf("Resultado llamada al servidor %d\n",server(atoi(argv[1])));
}
