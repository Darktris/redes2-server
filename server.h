#ifndef _SERVER_H
#define _SERVER_H

#define DATA_SIZE 65500
int server_start(uint16_t port, , void*(*handler)(void*));
int addConnection(int socketd);
int rmvConnection(int socketd);
int isConnected(int socketd);
int blockConnection(int socketd);
int unblockConnection(int socketd);

#endif 
