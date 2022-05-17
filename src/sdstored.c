//servidor
//./bin/sdstore proc-file teste.txt bli.txt gcompress gdecompress

//./bin/sdstored sdstored.conf bin/SDStore-transf/
//./bin/sdstore proc-file teste.txt pipe.txt gcompress gdecompress encrypt decrypt

#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <signal.h>

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

typedef struct tarefa
{ 
  int nrTarefa; 
  int fd_outFIFO;
  int fd;
  int timeStamp;
	char* input;
  char* output;
  int nrTransf;
  int transformations[20];

} Tarefa;

typedef struct Node {
  int key;
  void* task;
  struct Node *left;
  struct Node *right;
  int height;
} *Node;

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

typedef struct transform 
{
  int max;
  int curr; //current
  char* executavel;
} Transform;

#define LCHILD(x) 2 * x + 1
#define RCHILD(x) 2 * x + 2
#define PARENT(x) (x - 1) / 2

typedef struct minNode {
    
    int timeStamp;
    void* task;

} minNode ;

typedef struct minHeap {
    int size ;
    minNode *elem ;
} minHeap ;

minHeap* initMinHeap() {
    minHeap* hp = malloc(sizeof(struct minHeap));
    hp->size = 0 ;
    return hp ;
}

int max(int a, int b);

// Calculate height
int height(struct Node *N) {
  if (N == NULL)
    return 0;
  return N->height;
}

int max(int a, int b) {
  return (a > b) ? a : b;
}


// Create a node
struct Node *newNode(int key, void* task) {
  struct Node *node = (struct Node *)
  malloc(sizeof(struct Node));
  node->key = key;
  node->task = task;
  node->left = NULL;
  node->right = NULL;
  node->height = 1;
  return (node);
}

// Right rotate
struct Node *rightRotate(struct Node *y) {
  struct Node *x = y->left;
  struct Node *T2 = x->right;

  x->right = y;
  y->left = T2;

  y->height = max(height(y->left), height(y->right)) + 1;
  x->height = max(height(x->left), height(x->right)) + 1;

  return x;
}

// Left rotate
struct Node *leftRotate(struct Node *x) {
  struct Node *y = x->right;
  struct Node *T2 = y->left;

  y->left = x;
  x->right = T2;

  x->height = max(height(x->left), height(x->right)) + 1;
  y->height = max(height(y->left), height(y->right)) + 1;

  return y;
}

// Get the balance factor
int getBalance(struct Node *N) {
  if (N == NULL)
    return 0;
  return height(N->left) - height(N->right);
}

// Insert node
struct Node *insertNodeAVL(struct Node *node, int key, void* task) {
  // Find the correct position to insertNode the node and insertNode it
  if (node == NULL){
    return (newNode(key, task));
  }
  
  if (key < node->key)
    node->left = insertNodeAVL(node->left, key, task);
  else if (key > node->key)
    node->right = insertNodeAVL(node->right, key, task);
  else
    return node;

  // Update the balance factor of each node and
  // Balance the tree
  node->height = 1 + max(height(node->left),
               height(node->right));

  int balance = getBalance(node);
  if (balance > 1 && key < node->left->key)
    return rightRotate(node);

  if (balance < -1 && key > node->right->key)
    return leftRotate(node);

  if (balance > 1 && key > node->left->key) {
    node->left = leftRotate(node->left);
    return rightRotate(node);
  }

  if (balance < -1 && key < node->right->key) {
    node->right = rightRotate(node->right);
    return leftRotate(node);
  }

  return node;
}

struct Node *minValueNode(struct Node *node) {
  struct Node *current = node;

  while (current->left != NULL)
    current = current->left;

  return current;
}

void* node_getEstruturaAVL(struct Node* node, int value)
{
    if (node == NULL)
        return NULL;
    else
    {
        
        if (value == node->key)
            return node->task;
        else if (value < node->key)
            return node_getEstruturaAVL(node->left, value);
        else
            return node_getEstruturaAVL(node->right, value);
    }
}



// Delete a nodes
struct Node *deleteNodeAVL(struct Node *root, int key, void* task) {
  // Find the node and delete it
  if (root == NULL)
    return root;

  if (key < root->key)
    root->left = deleteNodeAVL(root->left, key, task);

  else if (key > root->key)
    root->right = deleteNodeAVL(root->right, key, task);

  else {
    if ((root->left == NULL) || (root->right == NULL)) {
      struct Node *temp = root->left ? root->left : root->right;

      if (temp == NULL) {
        temp = root;
        root = NULL;
      } else
        *root = *temp;
      free(temp);
    } else {
      struct Node *temp = minValueNode(root->right);

      root->key = temp->key;

      root->right = deleteNodeAVL(root->right, temp->key, task);
    }
  }

  if (root == NULL)
    return root;

  // Update the balance factor of each node and
  // balance the tree
  root->height = 1 + max(height(root->left),
               height(root->right));

  int balance = getBalance(root);
  if (balance > 1 && getBalance(root->left) >= 0)
    return rightRotate(root);

  if (balance > 1 && getBalance(root->left) < 0) {
    root->left = leftRotate(root->left);
    return rightRotate(root);
  }

  if (balance < -1 && getBalance(root->right) <= 0)
    return leftRotate(root);

  if (balance < -1 && getBalance(root->right) > 0) {
    root->right = rightRotate(root->right);
    return leftRotate(root);
  }

  return root;
}

    


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

Transform** adicionaT(int max, char* executavel, int transf, Transform** transfList){
    transfList[transf] = malloc(sizeof(struct transform));
    transfList[transf]->max = max;
    transfList[transf]->curr = 0;
    transfList[transf]->executavel = malloc(11 * sizeof(char));
    strcpy(transfList[transf]->executavel, executavel);

    return transfList;
}

Tarefa* novaTarefa(Message mensagem, int time, int nrTarefa){
    Tarefa* task = malloc(sizeof(struct tarefa));

    task->nrTarefa = nrTarefa;
    task->fd_outFIFO = mensagem.pid;
    task->timeStamp = time - mensagem.priority;
    task->input= strdup(mensagem.input);
    task->output = strdup(mensagem.output);
    task->nrTransf = mensagem.nrTransf;
    for (int i = 0; i < mensagem.nrTransf; i++)
        task->transformations[i] = mensagem.transformations[i];

    return task;

}

Message mensagemConcluded(int nrTarefa){
    Message m;

    m.type = END;
    m.pid = nrTarefa;

    return m;
}

Message mensagemTermina(){
    Message m;

    m.type = -1;

    return m;
}

void insertNode(minHeap *hp, Tarefa* task, int time) {
    // allocating space
    if(hp->size) {
        hp->elem = realloc(hp->elem, (hp->size + 1) * sizeof(minNode)) ;
    } else {
        hp->elem = malloc(sizeof(minNode)) ;
    }

    // initializing the node with value
    minNode nd;
    nd.task = task;
    nd.timeStamp = task->timeStamp;


    // Positioning the node at the right position in the min heap
    int i = (hp->size)++ ;
    while(i && nd.timeStamp < hp->elem[PARENT(i)].timeStamp) {
        hp->elem[i] = hp->elem[PARENT(i)] ;
        i = PARENT(i) ;
    }
    hp->elem[i] = nd ;
}

void swap(minNode *n1, minNode *n2) {
    minNode temp = *n1 ;
    *n1 = *n2 ;
    *n2 = temp ;
}

void heapify(minHeap *hp, int i) {
    int smallest = (LCHILD(i) < hp->size && hp->elem[LCHILD(i)].timeStamp < hp->elem[i].timeStamp) ? LCHILD(i) : i ;
    if(RCHILD(i) < hp->size && hp->elem[RCHILD(i)].timeStamp < hp->elem[smallest].timeStamp) {
        smallest = RCHILD(i) ;
    }
    if(smallest != i) {
        swap(&(hp->elem[i]), &(hp->elem[smallest])) ;
        heapify(hp, smallest) ;
    }
}

minNode deleteNode(minHeap *hp) {
    minNode node = hp->elem[0];

    if(hp->size) {
        hp->elem[0] = hp->elem[--(hp->size)] ;
        hp->elem = realloc(hp->elem, hp->size * sizeof(minNode)) ;
        heapify(hp, 0) ;
    } else {
        free(hp->elem) ;
    }

    return node;

}

#define MAX_LINE_SIZE 1024

Transform** listaTransf = NULL;


int tStamp = 0;
int nrTarefa = 1;
int fdClienteServidor, fdServer_Server;



void help(int fdwr) {
    char* help = "./sdstored status\n./sdstored proc-file input-filename output-filename transf-id-1 transf-id-2 ...\n";
    write(fdwr, help, MAX_LINE_SIZE);
}

ssize_t readln (int fd, char *buffer, size_t size) {

    int resultado = 0, i = 0;

    while ((resultado = read (fd, &buffer[i], 1)) > 0 && i < size) {
        if (buffer[i] == '\n') {
            i += resultado;
            return i;
        }

        i += resultado;
    }

    return i;
}

void adicionaPendente(Tarefa* new, int outFifo, minHeap* l) {

    int fd = open(itoa(outFifo), O_WRONLY);
    write(fd, "pending\n", 9);
    dup2(fd, new->fd);
    close(fd);
    insertNode(l, new, tStamp);

}

void adicionaExecucao(Tarefa* task, Node l) {

    insertNodeAVL(l, task->nrTarefa, task);
}


void lerConfig(const char* file) {
    char *buffer = malloc(1024 * sizeof(char));
    char *exec;
    char *max;
    int fd_config = open(file, O_RDONLY);
    int n;
    char* token;
    int i = 0;
    
    while ((n = readln(fd_config,buffer,1024 * sizeof(char))) > 0) {
        token = strtok(buffer," ");
        exec = strdup(token);
        token = strtok(NULL," ");
        max = strdup(token);

        //printf("%s %s %s\n", filtro,exec,max);
        listaTransf = adicionaT(atoi(max), exec, i, listaTransf);
        i++;
    }

    close(fd_config);
}





int executar(char* transFolder, Tarefa* new) {
    write(new->fd, "processing...\n", 15);
    char* path = malloc(1024 * sizeof(char));
    strcpy(path, transFolder);
    //printf("%d %d\n", fd_output, fd_input);

    int n = new->nrTransf;                       // número de comandos == número de filhos
    int p[n-1][2];                              // -> matriz com os fd's dos pipes
    int status[n];                              // -> array que guarda o return dos filhos
    //int size_input, size_output;

    if(new->nrTransf == 1) {
        int pid;
        
        char* tarnsformacao = strdup(listaTransf[new->transformations[0]]->executavel);
        
        tarnsformacao = strcat(path,tarnsformacao);

        int fd_output = open(new->output, O_CREAT | O_WRONLY | O_TRUNC, 0666);
        int fd_input = open(new->input, O_RDONLY);
        dup2(fd_input,0);
        close(fd_input);
        dup2(fd_output,1);
        close(fd_output);
    
        
        if ((pid = fork()) == 0) execlp(tarnsformacao,tarnsformacao,NULL);

        Message m = mensagemConcluded(new->nrTarefa);

        write(new->fd, "concluded\n", 11);

        write(fdServer_Server, &m, sizeof(Message));
        close(fdServer_Server);
        

      
    } else {

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
                  case 0: ;
                      // codigo do filho 0
                      char* tarnsformacao = strdup(listaTransf[new->transformations[i]]->executavel);
          
                      tarnsformacao = strcat(path,tarnsformacao);
                      
                      close(p[i][0]);

                      dup2(p[i][1],1);
                      close(p[i][1]);

                      int fd_input = open(new->input, O_RDONLY);
                      dup2(fd_input,0);
                      close(fd_input);

                      execlp(tarnsformacao,tarnsformacao,NULL);

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
                  case 0: ;
                      // codigo do filho n-1

                      
                      char* tarnsformacao = strdup(listaTransf[new->transformations[i]]->executavel);
          
                      tarnsformacao = strcat(path,tarnsformacao);
                      dup2(p[i-1][0],0);
                      close(p[i-1][0]);

                      int fd_output = open(new->output, O_CREAT | O_WRONLY | O_TRUNC, 0600);
                      dup2(fd_output,1);
                      close(fd_output);

                      execlp(tarnsformacao,tarnsformacao,NULL);

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
                  case 0: ;
                      // codigo do filho i


                      char* tarnsformacao = strdup(listaTransf[new->transformations[i]]->executavel);
          
                      tarnsformacao = strcat(path,tarnsformacao);
                      close(p[i][0]);
                      dup2(p[i-1][0],0);
                      close(p[i-1][0]);

                      dup2(p[i][1],1);
                      close(p[i][1]);


                      execlp(tarnsformacao,tarnsformacao,NULL);

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
    }
    

    write(new->fd, "concluded\n", 11);

    Message m = mensagemConcluded(new->nrTarefa);

    write(fdServer_Server, &m, sizeof(struct message));
    close(fdServer_Server);
    
    

    return 1;
}




void terminaServer(int signum){
    
    switch (signum){
    case SIGQUIT:
        close(fdServer_Server);

        
        unlink("fifo_Clients_Server");
        
        
        break;
    }
}


/*void printListaPedidos(minHeap* l, int fd) {
  if(l->size == 0)
    write(fd, "Não há tarefas em execução\n", 31);

  else{
    
    int primeiro = 0;
    char buffer[1024];
    for (int i; i < l->size; i++) {
      if (primeiro == 0) {
        strcpy(buffer,"task #");
        primeiro++;

      } else {strcat(buffer,"task #");}
        
        strcat(buffer,itoa(l->elem->task.nrTarefa));
        strcat(buffer,": proc-file");
        strcat(buffer,l->elem->task.input);
        strcat(buffer," ");
        strcat(buffer,l->elem->task.output);
        
        for (int j = 0; j < l->elem->task.nrTransf; j++){
            strcat(buffer," ");
            strcat(buffer,listaTransf[l->elem->task.transformations[j]]->executavel);
        }
        
        strcat(buffer, "\n");
    }
    write(fd, buffer, strlen(buffer));
  }
}*/

void printListaTransf(Transform** l, int fd) {


    char* buffer = malloc(MAX_LINE_SIZE);
    for (int i=0; i< 7;i++) {

      if (i == 0) {
        strcpy(buffer,"transf ");
      }
      
      else {strcat(buffer,"transf ");}

        strcat(buffer,l[i]->executavel);
        strcat(buffer,": ");
        strcat(buffer,itoa(l[i]->curr));
        strcat(buffer,"/");
        strcat(buffer,itoa(l[i]->max));
        strcat(buffer, " (running/max)\n");
        
    }
    write(fd, buffer, MAX_LINE_SIZE);
}


\
int transfDisponivel(Transform** l, Tarefa* tarefa) {
    int res = 0;
    for (int i = 0; !res && (i < tarefa->nrTransf); i++){

        if((l[tarefa->transformations[i]]->curr == l[tarefa->transformations[i]]->max))
        res++;
    }
    
    return res;
}

Transform** transfInc(Transform** l, int transf) {
    if(l[transf]->curr < l[transf]->max) {
      l[transf]->curr+= 1;
    }
    return l;
}

Transform** transfDec(Transform** l, int transf) {
    if(l[transf]->curr > 0) {
      l[transf]->curr-= 1;
    }
    return l;
}



//./bin/sdstored sdstored.conf bin/SDStore-transf/
//proc-file teste.txt pipe.txt gcompress gdecompress encrypt decrypt


int main(int argc, char const *argv[]) {

    
    struct Node *execucao = malloc(sizeof(struct Node));
    minHeap* pendentes = initMinHeap();

    if (mkfifo("fifo_Clients_Server", 0666))
        perror("Mkfifo");
        
    signal(SIGQUIT, terminaServer);

    Message buffer;

    char* transFolder = strdup(argv[2]);
    
    listaTransf = malloc(sizeof(struct transform) * 7);
    lerConfig (argv[1]);

    if((fdClienteServidor = open("fifo_Clients_Server", O_RDONLY)) == -1){
            perror("Error opening fifo_Clients_Server1\n");
        }

    if ((fdServer_Server = open("fifo_Clients_Server", O_WRONLY)) == -1) {
        perror("Error opening fifo_Clients_Server\n");
        return -1;
        }

    while (read(fdClienteServidor, &buffer,sizeof(Message))){
        //memset(&buffer,0,MAX_LINE_SIZE); //Limpa o espaço de memória das strings usadas
        int pid;
        
        if(buffer.type == STATUS) {
            if ((pid = fork()) == 0){
                int fdServidorCliente = open(itoa(buffer.pid), O_WRONLY);
                //printListaPedidos(pendentes,fdServidorCliente);
                printListaTransf(listaTransf,fdServidorCliente);
                close(fdServidorCliente);
                _exit(0);
            }
                        
        }
                
        else if(buffer.type == INFO) {
                if ((pid = fork()) == 0) {
                    int fdServidorCliente = open(itoa(buffer.pid), O_WRONLY);
                    help(fdServidorCliente);
                    close(fdServidorCliente);
                    _exit(0);
                }
            }



        else if(buffer.type == PROCFILE){

            Tarefa* new = novaTarefa(buffer, tStamp, nrTarefa);
            tStamp++;
            nrTarefa++;
            
            if (transfDisponivel(listaTransf, new))
                adicionaPendente(new, new->fd_outFIFO, pendentes);
                
            
            else {
                
                for(int t = 0; t < new->nrTransf; t++)
                    listaTransf = transfInc(listaTransf,new->transformations[t]);

                adicionaExecucao(new, execucao);
                




                if ((pid = fork()) == 0){
                    int fd = open(itoa(new->fd_outFIFO), O_WRONLY);
                    dup2(fd, new->fd);
                    close(fd);
                    //proc-file teste.txt bli.txt gcompress gdecompress
                    // ver filhos -exec set follow-fork-mode child
                    executar(transFolder, new);
                    close(new->fd);
                    
                    exit(0);
                }

                
            }

            
        }
        
        else if(buffer.type == END){
            int nrT = buffer.pid;
            
            Tarefa* task = malloc(sizeof(struct tarefa));

            task = node_getEstruturaAVL(execucao, nrT);

            execucao = deleteNodeAVL(execucao, nrT, task);

            printf("%d -> %s-> %s-> %s\n", task->nrTransf, task->input, task->output, listaTransf[task->transformations[0]]->executavel);
                fflush(0);

            

            for(int t = 0; t < task->nrTransf; t++)
                transfDec(listaTransf,task->transformations[t]);
            
            
            
            
            if(pendentes->size > 0){
              
                minNode node = deleteNode(pendentes);
                task = node.task;
                

                if (((pid = fork()) == 0) ){
                    //proc-file teste.txt bli.txt gcompress gdecompress
                    // ver filhos -exec set follow-fork-mode child
                    executar(transFolder, node.task);
                    close(task->fd);
                    _exit(0);
                }
                
            }
            free(task);
        
        }
    }
    
    return 0;
}


