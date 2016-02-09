#ifndef _SERVER_H
#define _SERVER_H

#include <pthread.h>
#include "tcpsocket.h"
#define DATA_SIZE 65

typedef struct {
    int socketd;
    void* msg;
    size_t len;
    void* more;
} conn_data;

#define SERVOK 0
#define SERVERR_PTHREAD -1
#define SERVERR_SIGNAL -2
#define SERVERR_SOCKET -3
#define SERVERR_ACCEPT -4
#define SERVERR_MALLOC -5
#define SERVERR_ARGS - -6
#define SERVERR_SELECT -7
#define SERVERR_RCV -8
#define SERVERR_NRUN -9

int server_launch(uint16_t port,  void*(*handler)(void*), void* more);
int connection_block(int socketd);
int connection_unblock(int socketd);

#endif 
