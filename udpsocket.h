/* vim: set ts=4 sw=4 et: */
#ifndef _UDPSOCKET_H
#define _UDPSOCKET_H

#include <stdio.h>          
#include <stdlib.h>
#include <pthread.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* Estructuras de datos */
typedef struct {
    int socketd, acceptd; 
} udpsocket_args ;
typedef struct {
    socklen_t len;
    struct sockaddr_in addr;
} udpsocket_side;

/* Valores de retorno */
#define UDPCLOSED 1
#define UDPOK 0
#define UDPERR_ARGS -1
#define UDPERR_SOCKET -2
#define UDPERR_BIND -3
#define UDPERR_PTHREAD -5
#define UDPERR_ACCEPT -6
#define UDPERR_SEND -7
#define UDPERR_RECV -8

/* Constantes de relevancia */
#define BACKLOG 5

/* Funciones */
int server_udpsocket_open(uint16_t port, int* socketd);
int client_udpsocket_open(uint16_t port, int* socketd);
int udpsocket_snd(int socketd, void* data, size_t len, udpsocket_side* side);
int udpsocket_rcv(int socketd, void* data, size_t max, size_t* len, udpsocket_side* src);

#endif
