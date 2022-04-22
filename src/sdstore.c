//cliente
//bue da codigo

#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>

int fdClienteServidor;
int fdServidorCliente;

char* itoa(int i){
    char const digit[] = "0123456789";
    int p = 0;
    char *b = malloc(sizeof(char) * 10);
    int shifter = i;

    do{
        p++;
        shifter = shifter/10;
    }while(shifter);

    b[p] = '\0';

    do{
        p--;
        b[p] = digit[i%10];
        i = i/10;
    }while(i);

    return b;
}

int main (int argc, char** argv){
    char* buffer = malloc(sizeof(char) * 1024);


    int i = 1;

    if(i < argc) {
        strcpy(buffer,argv[i]);
        i++;
    }

    while(i < argc) {
        strcat(buffer," ");
        strcat(buffer,argv[i]);
        i++;
    }

    strcat(buffer," ");
    strcat(buffer,itoa(getpid()));


    if ((fdClienteServidor = open("client_server_pipe", O_WRONLY)) == -1) {
        perror("Error opening fifo\n");
        return -1;
    }


    if(argc == 1)
        strcpy(buffer,"info");


    write(fdClienteServidor,buffer,sizeof(char)*strlen(buffer));

    close(fdClienteServidor);
    int r;

    if(argc > 4) {
        sleep(100);
        return 0;
    }

    fdServidorCliente = open("server_client_pipe", O_RDONLY);

    int n = 0;
    while((n = read(fdServidorCliente,buffer,1024 * sizeof(char))) > 0){
      write(1,buffer,n * sizeof(char));
    }

    close(fdServidorCliente);


    return 0;
}