#include <G-2301-01-P1-tcp.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#define L 74
#define HOST "localhost"
#define PORT 7071

int main() {
    char data[L];
    int i=0;
    size_t len;
    printf("Mensaje:");
    for(i=0; i<L; i++) {
        data[i]=(i+48);
        printf("%c", data[i]);
    }
    printf("\n");
    if(client_tcpsocket_open(PORT, &i, HOST)<0) 
        perror("tcpsocket_open");
    if(tcpsocket_snd(i, data, L) <0) 
        perror("snd");
    bzero(data, L);
    if(tcpsocket_rcv(i, data, L, &len)<0) 
        perror("rcv");
    tcpsocket_close(i);
    printf("Recibido:");
    for(i=0;i<len;i++) {
        printf("%c", data[i]);
    }
}
