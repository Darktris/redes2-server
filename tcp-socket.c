#include <stdio.h>          
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "tcp-socket.h"

int openTCPSocket(uint16_t serverPort, int* socketd) { 
    struct sockaddr_in server; 

    /* Control de errores */
    if(socketd==NULL || serverPort==0) {
        return TCPERR_ARGS;
    }
    /* Creacion del socket */
    if ((*socketd=socket(AF_INET, SOCK_STREAM, 0)) == -1 ) {  
        return TCPERR_SOCKET;
    }

    /* Datos del servidor */
    server.sin_family = AF_INET;         
    server.sin_port = htons(serverPort); 
    server.sin_addr.s_addr = INADDR_ANY; 

    //bzero(&(server.sin_zero),8); 

    /* Unimos el socket al proceso */
    if(bind(*socketd,(struct sockaddr*)&server, sizeof(server)) == -1) {
        return TCPERR_BIND;
    }     

    /* Empieza a escuchar conexiones */
    if(listen(*socketd,BACKLOG) == -1) {  
        return TCPERR_LISTEN;
    }

    return TCPOK;
}


int acceptTCPSocket(int socketd, void* (*handleConnection)(void*), tcpsocket_args* user_struct) {
    int acceptd=0;

    /* Control de errores */
    if(socketd==-1 || handleConnection==NULL) {
        return TCPERR_ARGS;
    }
    
    acceptd=accept(socketd,(struct sockaddr *)&(user_struct->client),&(user_struct->clientlen));
    if(acceptd==-1) {
        return TCPERR_ACCEPT;
    }

    if(pthread_create(&(user_struct->user_thread),NULL, handleConnection, (void*)user_struct)!=0) {
        return TCPERR_PTHREAD;
    }
    
    return TCPOK;
}
