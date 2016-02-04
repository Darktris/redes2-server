#ifndef _SERVER_H
#define _SERVER_H

#include <pthread.h>
#include "tcpsocket.h"
#define DATA_SIZE 65500
typedef struct {
    int socketd;
    void* msg;
    size_t len;
    void* more;
} conn_data;
int server_launch(uint16_t port,  void*(*handler)(void*), void* more);
int connection_block(int socketd);
int connection_unblock(int socketd);

#endif 
