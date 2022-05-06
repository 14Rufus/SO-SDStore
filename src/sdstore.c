//cliente
//bue da codigo

#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>

#define MAX_LINE_SIZE 1024

int fd_Clients_Server;

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

    

    char* fifo_name = itoa(getpid());
    if (mkfifo(fifo_name, 0666))
        perror("Mkfifo");
    
    
    
    int i = 1;

    strcat(buffer,itoa(getpid()));

    while(i < argc) {
        strcat(buffer," ");
        strcat(buffer,argv[i]);
        i++;
    }

    
    
    if ((fd_Clients_Server = open("fifo_Clients_Server", O_WRONLY)) == -1) {
        perror("Error opening fifo\n");
        unlink(fifo_name);
        return -1;
    }
    
    write(fd_Clients_Server,buffer,MAX_LINE_SIZE);
    close(fd_Clients_Server);


    int n = 0;
    int fdServidorCliente = open(fifo_name, O_RDONLY);
    
    while((n = read(fdServidorCliente,buffer,MAX_LINE_SIZE)) > 0){
        write(1,buffer,n * sizeof(char)); 
        memset(buffer,0,MAX_LINE_SIZE); //Limpa o espaço de memória das strings usadas
    }
    
    close(fdServidorCliente);
    unlink(fifo_name);
    return 0;
}