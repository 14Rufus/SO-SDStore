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

void help(int fdwr) {
    char help[] = "./sdstored status\n./sdstored proc-file input-filename output-filename transf-id-1 transf-id-2 ...\n";
    write(fdwr, help, sizeof(help));
}

int executar(char* input,char* output,char** transf,int numTransf) {
//   printf("%s %s %d\n", input,output,numFiltros);
  for (int i = 0; i < numTransf; ++i)
  {
    // printf("%s\n", filtros[i]);
  }

  int fd_output = open(output, O_CREAT | O_WRONLY | O_TRUNC, 0600);
  int fd_input = open(input, O_RDONLY);
  char* path = malloc(1024 * sizeof(char));
  strcpy(path,"SDStore-transf/");

//   printf("%d %d\n", fd_output, fd_input);

    int n = numTransf;                         // número de comandos == número de filhos
    int p[n-1][2];                              // -> matriz com os fd's dos pipes
    int status[n];                              // -> array que guarda o return dos filhos


    if(numTransf == 1) {
      transf[0] = strcat(path,transf[0]);
      dup2(fd_input,0);
      close(fd_input);
      dup2(fd_output,1);
      close(fd_output);

      execlp(transf[0],transf[0],NULL);

      return 0;
    }


    // criar os pipes conforme o número de comandos
    for (int i = 0; i < n-1; i++) {
        if (pipe(p[i]) == -1) {
            perror("Pipe não foi criado");
            return -1;
        }
    }

    // criar processos filhos para executar cada um dos comandos
    for (int i = 0; i < n; i++) {
      transf[i] = strcat(path,transf[i]);

        if (i == 0) {
            switch(fork()) {
                case -1:
                    perror("Fork não foi efetuado");
                    return -1;
                case 0:
                    // codigo do filho 0

                    dup2(fd_input,0);
                    close(p[i][0]);

                    dup2(p[i][1],1);
                    close(p[i][1]);

                    execlp(transf[i],transf[i],NULL);

                    _exit(0);
                default:
                    close(p[i][1]);
            }
        }
        else if (i == n-1) {
            switch(fork()) {
                case -1:
                    perror("Fork não foi efetuado");
                    return -1;
                case 0:
                    // codigo do filho n-1

                    dup2(p[i-1][0],0);
                    close(p[i-1][0]);

                    dup2(fd_output,1);
                    close(p[i][1]);

                    execlp(transf[i],transf[i],NULL);

                    _exit(0);
                default:
                    close(p[i-1][0]);
            }
        }
        else {
            switch(fork()) {
                case -1:
                    perror("Fork não foi efetuado");
                    return -1;
                case 0:
                    // codigo do filho i

                    dup2(p[i-1][0],0);
                    close(p[i-1][0]);

                    dup2(p[i][1],1);
                    close(p[i][1]);


                    execlp(transf[i],transf[i],NULL);

                    _exit(0);
                default:
                    close(p[i-1][0]);
                    close(p[i][1]);
            }
        }

    }

    for (int i = 0; i < n; i++)
    {
        wait(&status[i]);

        if (WIFEXITED(status[i])) {
            // printf("[PAI]: filho terminou com %d\n", WEXITSTATUS(status[i]));
        }
    }
    return 1;
}

int main(int argc, char const *argv[]) {


    char *buffer = malloc(1024 * sizeof(char));
    char **comando;

    while (read(0,buffer,sizeof(char) * 1024) ){
        
        int comandoSize = 0;
        comando = separaString(buffer, comandoSize);
        
        
        if(!strcmp(comando[0],"proc-file") && comandoSize > 4){

          char** transformacoes = NULL; //<------ falta preencher mas preciso de perceber a estrutura faaaaaaaaaaaaaaaaaaaaaaaaaaaaack
          int pid;

          if(!(pid = fork())){
            
                        //EXECUTAR TAREFA
                        executar(comando[1],comando[2],transformacoes,comandoSize - 4);
                        //sleep(3);
                        _exit(0);


        }
          
        
        else help(1);

    }
    return 0;
}


