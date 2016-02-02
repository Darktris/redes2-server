/* vim: set ts=4 sw=4 et: */
/**
  @file tcp-socket.c
  @brief Libreria de manejo de sockets TCP
  @author Sergio Fuentes  <sergio.fuentesd@estudiante.uam.es>
  @author Daniel Perdices <daniel.perdices@estudiante.uam.es>
  @date 2016/02/01
  */
#include <stdio.h>          
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "tcp-socket.h"

/**
  @brief Abre un socket TCP 
  @param serverPort: Puerto desde el que se desea escuchar
  @param *socketd: Puntero al descriptor del socket
  @return TCPOK si todo fue correcto
          TCPERR_ARGS/SOCKET/BIND/LISTEN en caso de error con estás funciones
*/
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

/**
  @brief Acepta una conexion socket y lanza un hilo de atencion a la conexión 
  @param socketd: Descriptor del socket escuchado
  @param *handleConnection: rutina de atención a la peticion
  @param *conn_struct: estructura de datos de la conexion
  @return TCPOK si todo fue correcto
          TCPERR_ARGS/ACCEPT/PTHREAD en caso de error con estás funciones
*/
//int acceptTCPSocket(int socketd, void* (*handleConnection)(void*), tcpsocket_args* conn_struct) {
//    int acceptd=0;
//
//    /* Control de errores */
//    if(socketd==-1 || handleConnection==NULL) {
//        return TCPERR_ARGS;
//    }
//
//    /* Acepta la conexión */
//    acceptd=accept(socketd,(struct sockaddr *)&(conn_struct->client),&(conn_struct->clientlen));
//    if(acceptd==-1) {
//        return TCPERR_ACCEPT;
//    }
//
//    /* Lanza un hilo orientado a esa conexión */
//    if(pthread_create(&(conn_struct->user_thread),NULL, handleConnection, (void*)conn_struct)!=0) {
//        return TCPERR_PTHREAD;
//    }
//
//    return TCPOK;
//}

/**
  @brief Acepta una conexion socket
  @param socketd: Descriptor del socket escuchado
  @param *conn_struct: estructura de datos de la conexion
  @return TCPOK si todo fue correcto
          TCPERR_ARGS/ACCEPT/PTHREAD en caso de error con estás funciones
*/
int acceptTCPSocket(int socketd, tcpsocket_args* conn_struct) {
    int acceptd=0;

    /* Control de errores */
    if(socketd==-1 || conn_struct==NULL) {
        return TCPERR_ARGS;
    }

    /* Acepta la conexión */
    conn_struct->acceptd=accept(socketd,(struct sockaddr *)&(conn_struct->client),&(conn_struct->clientlen));
    if(acceptd==-1) {
        return TCPERR_ACCEPT;
    }

    return TCPOK;
}


/**
  @brief Envia a traves del socket TCP los datos
  @param socketd: Descriptor del socket escuchado
  @param *data: puntero a los datos que se han de enviar
  @param len: tamano de los datos a enviar
  @return TCPOK si todo fue correcto
          TCPERR_ARGS/SEND en caso de error con estás funciones
*/
int sendTCPSocket(int socketd, void* data, size_t len) {
    int flag;
    int sent;	
 
    /* Control de errores */
    if(socketd==-1 || data==NULL || !len) {
        return TCPERR_ARGS;
    }
   /* Envia los datos por el socket hasta que se acaban los datos */
    do {
        sent=send(socketd,data,len,0); 
        if(sent==-1) {
            return TCPERR_SEND;
        }
        flag=((len-sent)!=0);
    } while(flag);
    return TCPOK;
}

/**
  @brief Recibe a traves del socket TCP
  @param socketd: Descriptor del socket escuchado
  @param *data: puntero a donde se almacenan los datos
  @param maxlen: tamaño de la zona de memoria
  @param *len: puntero a la variable de longitud recibida
  @return TCPOK si todo fue correcto
          TCPERR_ARGS/RECV en caso de error con estás funciones
*/
int rcvTCPSocket(int socketd, void* data, size_t maxlen, size_t* len) {
    size_t n;
    
    /* Control de errores */
    if(socketd==-1 || data==NULL || !maxlen || len==NULL) {
        return TCPERR_ARGS;
    }
    
    /* Recepción de los datos */
    n=recv(socketd,data,maxlen,0);
    if(n==-1) {
        return TCPERR_RECV;
    }
    *len=n;
    return TCPOK;
}
