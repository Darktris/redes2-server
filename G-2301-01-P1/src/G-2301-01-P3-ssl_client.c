#include <stdio.h>
#include <G-2301-01-P1-tcp.h>
#include <G-2301-01-P3-ssl.h>
void show_use(){
}
int main(int argc, char** argv) {
    unsigned long port;
    int socketd;
    char buf[8192];
    int len;
    if(argc<3) {
        show_use();
        return 0;
    }

    if(sscanf(argv[1], "%lu", &port) != 1) {
        show_use();
        return 0;
    }


    inicializar_nivel_SSL();
    if(fijar_contexto_SSL(FILE_CLIENT_PKEY, FILE_CLIENT_CERTIFICATE)<0) {
        FILE* f = fopen("/tmp/err_ssl", "w");
        //ERR_print_errors_fp(f);
        fclose(f);
        return 0;
    }

    client_tcpsocket_open(port, &socketd, argv[2]);
    conectar_canal_seguro_SSL(socketd);
    
    while(1) {
        gets(buf);
        enviar_datos_SSL(socketd, buf, strlen(buf));
	bzero(buf, 8192);
        recibir_datos_SSL(socketd, buf, 8191, &len);
        puts(buf);
    }
}

