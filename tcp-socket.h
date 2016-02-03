#ifndef __TCP_SOCKET_H__
#define __TCP_SOCKET_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


/* Estructuras de datos */
typedef struct {
    int socketd, acceptd; 
    socklen_t clientlen; 
    struct sockaddr_in client;
//    void* moreargs;
//    pthread_t user_thread;
}tcpsocket_args ;

/* Valores de retorno */
#define TCPCONN_CLOSED 1
#define TCPOK 0
#define TCPERR_ARGS -1
#define TCPERR_SOCKET -2
#define TCPERR_BIND -3
#define TCPERR_LISTEN -4
#define TCPERR_PTHREAD -5
#define TCPERR_ACCEPT -6
#define TCPERR_SEND -7
#define TCPERR_RECV -8

/* Constantes de relevancia */
#define BACKLOG 5

/* Funciones */
int openTCPSocket(uint16_t serverPort, int* socketd);
//int acceptTCPSocket(int socketd, void* (handleConnection)(void*), tcpsocket_args* user_struct);
int acceptTCPSocket(int socketd, tcpsocket_args* conn_struct);
int sendTCPSocket(int socketd, void* data, size_t len);
int rcvTCPSocket(int socketd, void* data, size_t maxlen, size_t* len);
#endif
