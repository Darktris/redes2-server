#include "server.h"
#include <stdlib.h>
#include <stdio.h>
void* echo(void* data) {
    conn_data* thread_data = (conn_data*) data;
    pthread_detach(pthread_self());
    printf("Mensaje: %s\n", thread_data->msg);
    tcpsocket_snd(thread_data->socketd, thread_data->msg, thread_data->len);
    connection_unblock(thread_data->socketd);
    free(thread_data->msg);
    free(thread_data);
    return 0;
}

int main(int argc, char** argv) {
    if(argc!=2) return -1;
    printf("Retorno del servidor %d",server_launch(atoi(argv[1]), echo, NULL));
}
