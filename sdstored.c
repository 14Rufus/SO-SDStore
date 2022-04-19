//servidor
//proc-file teste.txt bli.txt gcompress gdecompress
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

//struct Pedidos
typedef struct listaP *ListaP;


//struct Transformações
typedef struct listaT *ListaT;

ListaT listaTransf = NULL;
ListaP pendentes = NULL;
ListaP execucao = NULL;

int comandoSize = 0;
int numeroTarefa = 1;




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
  int curr; //current
  char* executavel;
  int *pids;
  struct listaT *prox;
};

void help(int fdwr) {
    char help[] = "./sdstored status\n./sdstored proc-file input-filename output-filename transf-id-1 transf-id-2 ...\n";
    write(fdwr, help, sizeof(help));
}



char** separaString(char* buffer){
    char** comando = NULL;
    int nrComandos = 1;
    
    for (int i = 0; buffer[i]!='\n'; i++)
    {
        if (buffer[i] == ' ') nrComandos++;
    }

    buffer = strtok(buffer, "\n");
    
    comando = (char**) realloc(comando, (nrComandos + 1) * sizeof(char*));

    char* token = strtok(buffer," ");
    comando[0] = strdup(token);

    int nrComando = 1; 
    while ((token = strtok(NULL," ")) != NULL)
    {
        comando[nrComando] = strdup(token);
        nrComando++;
    }
    comandoSize = nrComandos;
    return comando;
}

    ssize_t readln (int fd, char *buffer, size_t size) {

    int resultado = 0, i = 0;

    ssize_t read_bytes = 0;

    while ((resultado = read (fd, &buffer[i], 1)) > 0 && i < size) {
        if (buffer[i] == '\n') {
            i += resultado;
            return i;
        }

        i += resultado;
    }

    return i;
}

ListaT adicionaT (int max, char* executavel,ListaT l) {
    ListaT aux = l;
    ListaT new = malloc(sizeof(ListaT));
    new->max = max;
    new->curr = 0;
    new->executavel = malloc(sizeof(char) * strlen(executavel));
    new->pids = malloc(sizeof(int)*max);
    strcpy(new->executavel,executavel);
    new->prox = NULL;

    if (!l) {
        return new;
    }

    for (; aux->prox; aux = aux->prox);

    aux->prox = new;

    return l;
}



void lerConfig(char* file) {
    char *buffer = malloc(1024 * sizeof(char));
    char *filtro;
    char *exec;
    char *max;
    int fd_config = open(file, O_RDONLY);
    int i = 0,n,o = 0,i2 = 0;
    char* token;


    while ((n = readln(fd_config,buffer,1024 * sizeof(char))) > 0) {
        token = strtok(buffer," ");
        filtro = strdup(token);
        token = strtok(NULL," ");
        exec = strdup(token);
        token = strtok(NULL," ");
        max = strdup(token);

        //printf("%s %s %s\n", filtro,exec,max);
        listaTransf = adicionaT(atoi(max),exec,listaTransf);
    }
}



int executar(char* input,char* output,char** transf,int numTransf) {
    char* path = malloc(1024 * sizeof(char));
    strcpy(path,"SDStore-transf/");

//   printf("%d %d\n", fd_output, fd_input);

    int n = numTransf;                         // número de comandos == número de filhos
    int p[n-1][2];                              // -> matriz com os fd's dos pipes
    int status[n];                              // -> array que guarda o return dos filhos


    if(numTransf == 1) {
        transf[0] = strcat(path,transf[0]);
        int fd_output = open(output, O_CREAT | O_WRONLY | O_TRUNC, 0600);
        int fd_input = open(input, O_RDONLY);
        dup2(fd_input,0);
        close(fd_input);
        dup2(fd_output,1);
        close(fd_output);
    
        
        execlp(transf[0],transf[0],NULL);

        return 0;
    }

    //proc-file teste.txt bli.txt gcompress gdecompress
    // criar processos filhos para executar cada um dos comandos
    for (int i = 0; i < n; i++) {

        if (i == 0) {
            
            if (pipe(p[i]) == -1) {
                perror("Pipe não foi criado");
                return -1;
            }
            
            switch(fork()) {
                case -1:
                    perror("Fork não foi efetuado");
                    return -1;
                case 0:
                    // codigo do filho 0
                    transf[i] = strcat(path,transf[i]);
                    
                    close(p[i][0]);

                    dup2(p[i][1],1);
                    close(p[i][1]);

                    int fd_input = open(input, O_RDONLY);
                    dup2(fd_input,0);
                    close(fd_input);

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

                    
                    transf[i] = strcat(path,transf[i]);
                    dup2(p[i-1][0],0);
                    close(p[i-1][0]);

                    int fd_output = open(output, O_CREAT | O_WRONLY | O_TRUNC, 0600);
                    dup2(fd_output,1);
                    close(fd_output);

                    execlp(transf[i],transf[i],NULL);

                    _exit(0);
                default:
                    close(p[i-1][0]);
            }
        }
        else {

            if (pipe(p[i]) == -1) {
                perror("Pipe não foi criado");
                return -1;
            }
            switch(fork()) {
                case -1:
                    perror("Fork não foi efetuado");
                    return -1;
                case 0:
                    // codigo do filho i


                    transf[i] = strcat(path,transf[i]);
                    close(p[i][0]);
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

        
    while (readln(0,buffer,sizeof(char) * 1024) ){
        
    
        comando = separaString(buffer);

        /*if(!strcmp(comando[0], "status")) {
                fd_fifo_server_client = open("server_client_pipe", O_WRONLY);
                printLista(pendentes,fd_fifo_server_client);
                printListaFiltros(listaFiltros,fd_fifo_server_client);
                close(fd_fifo_server_client);
        } 
        */
        //else 
        if(!strcmp(comando[0],"proc-file") ){
        
        char** transformacoes = NULL; //<------ falta preencher mas preciso de perceber a estrutura faaaaaaaaaaaaaaaaaaaaaaaaaaaaack
        int pid;

        char* tarefa = malloc(1024 * sizeof(char));
            strcpy(tarefa,comando[0]);
            int i = 1;
            while(i < comandoSize) { // - 1 era o que dava mal
                if(i > 2) {
                    transformacoes = (char**) realloc(transformacoes, (i + 1) * sizeof(char*)); //(i + 1) * sizeof(char*)
                    transformacoes[i - 3] = strdup(comando[i]);  
                }

                strcat(tarefa," ");
                strcat(tarefa,comando[i]);
                i++;
            }

            //pendentes = adicionaTarefa(pid,atoi(comando[comandoSize - 1]),numeroTarefa,tarefa,pendentes);

            //proc-file teste.txt bli.txt gcompress gdecompress
            // ver filhos -exec set follow-fork-mode child
            if ((pid = fork()) == 0){
                executar(comando [1], comando[2], transformacoes, comandoSize - 3);
                _exit(0);
            }
        }
          
        
        else help(1);

    }
    return 0;
}


