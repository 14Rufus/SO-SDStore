//servidor

#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
//#include <errno.h>
#include <time.h>
#include <signal.h>
#include "sdstored.h"
#include "aux.c"
#include "aux.h"



ListaT listaTransf = NULL;
ListaP pendentes = NULL;
ListaP execucao = NULL;

//int comandoSize = 0; -> include "aux.c"




//struct Pedidos
struct listaP {
  int pid;
  int pidCliente;
  int numeroTarefa;
  char* tarefa;
  struct listaP *prox;
};


//struct Transformações
struct listaT {
  int max;
  int curr;
  char* filtro;
  char* executavel;
  int *pids;
  struct listaT *prox;
};



int main(int argc, char const *argv[]) {


    char *buffer = malloc(1024 * sizeof(char));
    char **comando;

    while ( read(0,buffer,sizeof(char) * 1024) ){
        
        comandoSize = 0;
        comando = separaString(buffer);

    }




    



    return 0;
}

