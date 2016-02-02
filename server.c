#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/time.h>
#include "tcp-socket.h"

pthread_mutex_t mutex_setfd;
fd_set connections, read;
int server(uint16_t port) {
    int socketd, conn_socket;
    int set_size=1, i=0;
    tcpsocket_args args;
    char data[65500]={0};
    size_t len;

    pthread_mutex_init(&mutex_setfd, NULL);
    if(openTCPSocket(port, &socketd) < 0) {
        return -1;
    }
    FD_ZERO(&connections);
    FD_SET(socketd, &connections);

    while(1) {
        read = connections;
        if(select(FD_SETSIZE, &read,NULL,NULL,NULL) < 0) {
            return -2;
        }
        for(i=0; i < FD_SETSIZE; i++) {
            if(FD_ISSET(i, &read)) {

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
                    if(rcvTCPSocket(i, data, 65500, &len) < 0) {
                        return -4;
                    }
                    printf("%s", data);
                }
            }
        }
    }

}
int main(int argc, char** argv) {
    server(atoi(argv[1]));
}

