//cliente

#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>

#define MAX_LINE_SIZE 1024

#define INFO 0
#define STATUS 1
#define PROCFILE 2
#define END 3

#define NOP 0
#define BCOMP 1
#define BDECOMP 2
#define GCOMP 3
#define GDECOMP 4
#define ENCRYPT 5
#define DECRYPT 6

typedef struct message
{ 
  int pid;
  int rep;
	int type;
  int priority;
	char input[20];
  char output[20];
  int nrTransf;
  int transformations[20];

} Message;


typedef struct tarefa
{ 
  int nrTarefa; 
  int fd_outFIFO;
  int timeStamp;
	char* input;
  char* output;
  int nrTransf;
  int transformations[20];

} Tarefa;

typedef struct transform 
{
  int max;
  int curr; //current
  char* executavel;
} Transform;

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

Message novoMessage(char** pedido, int comandoSize){
    Message new;
    new.rep = 0;
    
    if (!strcmp(pedido[1], "status")){
        new.type = STATUS;
    } 
    
    else if (!strcmp(pedido[1], "info")){
       new.type = INFO;
    }

    else if(!strcmp(pedido[1], "proc-file")){
        new.type = PROCFILE;
        strcpy(new.input, pedido[2]);
        strcpy(new.output, pedido[3]);
        int i;

        if (!strcmp(pedido[4], "-p")){
            new.priority = atoi(pedido[5]);
            i = 6;
        } else {
            new.priority = 0;
            i = 4;
            
            }
        
        int a = i;
        for(; i < comandoSize; i++){
            
            if (!strcmp("nop", pedido[i])){
                new.transformations[i-a] = NOP;
            } 
            
            else if (!strcmp("bcompress", pedido[i])){
                new.transformations[i-a] = BCOMP;
            } 
            
            else if (!strcmp("bdecompress", pedido[i])){
                new.transformations[i-a] = BDECOMP;
            } 
            
            else if (!strcmp("gcompress", pedido[i])){
                new.transformations[i-a] = GCOMP;
            } 
            
            else if (!strcmp("gdecompress", pedido[i])){
                new.transformations[i-a] = GDECOMP;
            } 
            
            else if (!strcmp("encrypt", pedido[i])){
                new.transformations[i-a] = ENCRYPT;
            } 
            
            else if (!strcmp("decrypt", pedido[i])){
                new.transformations[i-a] = DECRYPT;
            }

        }
        new.nrTransf = comandoSize-a;

    }
    

    return new;
}


int fd_Clients_Server;


int main (int argc, char** argv){
    char* buffer = malloc(sizeof(char) * 1024);

    

    char* fifo_name = itoa(getpid());
    if (mkfifo(fifo_name, 0666))
        perror("Mkfifo");
    
    
    if ((fd_Clients_Server = open("fifo_Clients_Server", O_WRONLY)) == -1) {
        perror("Error opening fifo\n");
        unlink(fifo_name);
        return -1;
    }


    Message new = novoMessage(argv, argc);

    new.pid = getpid();

    write(fd_Clients_Server, &new, sizeof(Message));
    close(fd_Clients_Server);


    int n = 0;
    int fdServidorCliente = open(fifo_name, O_RDONLY);
    int pid;
    int status;
    
    if((pid = fork()) == 0){
        
        while((n = read(fdServidorCliente,buffer,MAX_LINE_SIZE)) > 0){
            write(1,buffer, 1024);            
            memset(buffer,0,MAX_LINE_SIZE); //Limpa o espaço de memória das strings usadas
        }
         close(fdServidorCliente);
        _exit(0);
    }

    
    
    wait(&status);
    close(fdServidorCliente);
    
    
    unlink(fifo_name);
    return 0;
}