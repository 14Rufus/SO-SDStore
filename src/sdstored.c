//servidor

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
  int nrTarefa;             //número correspondente a uma determinada tarefa (único para cada uma)
  int fd_outFIFO;           //pid do cliente que corresponde ao nome do FIFO criado para a comunicação servidor-cliente
  int fd;                   //file descriptor do FIFO
  int timeStamp;            //valor correspondente à diferença entre o valor tStamp e a prioridade do pedido (utilizado para controlo na estrutura MinHeap das prioridades)
  char* input;              //nome do ficheiro de input
  char* output;             //nome do ficheiro de output
  int nrTransf;             //número de transformações do pedido do cliente
  int transformations[20];  //transformações a serem executadas pelo servidor, em que cada transformação é representada por um inteiro

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
  int curr;         //current
  char* executavel;
} Transform;


//Posicoes respetivas da MinHeap das prioridades
#define LCHILD(x) 2 * x + 1
#define RCHILD(x) 2 * x + 2
#define PARENT(x) (x - 1) / 2

//substruct da minHeap
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

//encontra o node de menor valor
struct Node *minValueNode(struct Node *node) {
  struct Node *current = node;

  while (current->left != NULL)
    current = current->left;

  return current;
}

//procura um node com uma determinada key
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

    

// funçao que passa de int para string
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


// adiciona uma transformação ao array de apontadores de struct transform
Transform** adicionaT(int max, char* executavel, int transf, Transform** transfList){
    
    transfList[transf] = malloc(sizeof(struct transform));
    transfList[transf]->max = max;
    transfList[transf]->curr = 0;
    transfList[transf]->executavel = malloc(11 * sizeof(char));
    strcpy(transfList[transf]->executavel, executavel);

    return transfList;
}

// Cria uma nova tarefa a partir da Message recebida
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

//Cria uma Message do tipo END
Message mensagemConcluded(int nrTarefa){
    Message m;

    m.type = END;
    m.pid = nrTarefa;

    return m;
}

// insere um nodo numa minHeap
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

// apaga um nodo numa minHeap
minHeap* deleteNode(minHeap *hp) {
    if(hp->size) {
        hp->elem[0] = hp->elem[--(hp->size)];
        hp->elem = realloc(hp->elem, hp->size * sizeof(minNode));
        heapify(hp, 0);
    } else {
        free(hp->elem);
    }

    return hp;
}

// Vai buscar a tarefa que esta na raiz da minHeap
void* node_getEstruturaMinH (minHeap* hp){
  return (hp->elem->task);
}

#define MAX_LINE_SIZE 1024

Transform** listaTransf = NULL;


int tStamp = 0;
int nrTarefa = 1;
int fdClienteServidor, fdServer_Server;


// escreve a info para o descritor fdwr
void help(int fdwr) {
    char* help = "./sdstored status\n./sdstored proc-file input-filename output-filename transf-id-1 transf-id-2 ...\n";
    write(fdwr, help, sizeof(char)*99);
}


// lê uma linha vinda do fd
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

// adiciona uma tarefa à minHeap l, escrevendo para o cliente a cena do "pending"
void adicionaPendente(Tarefa* new, int outFifo, minHeap* l) {
    int fd = open(itoa(outFifo), O_WRONLY);
    write(fd, "pending\n", 9);
    dup2(fd, new->fd);
    close(fd);
    insertNode(l, new, tStamp);

}

// adiciona uma tarefa à AVL l
void adicionaExecucao(Tarefa* task, Node l) {
    insertNodeAVL(l, task->nrTarefa, task);
}

/*  lê o file dado (com configurações de transformações) e 
    adiciona a trnasformaçao ao array de Transform's com as configurações dadas no file */
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
        listaTransf = adicionaT(atoi(max), exec, i, listaTransf);
        i++;
    }
    close(fd_config);
}

// Executa uma tarefa e escreve o output para o cliente (o processing e o concluded com os bytes lidos)
int executar(char* transFolder, Tarefa* new) {
    write(new->fd, "processing...\n", 15);
    char* path = malloc(1024 * sizeof(char));
    strcpy(path, transFolder);
    int n = new->nrTransf;                      // número de comandos == número de filhos
    int p[n-1][2];                              // matriz com os fd's dos pipes
    int status[n];                              // array que guarda o return dos filhos
    int size_input, size_output;


    // Caso apenas seja feita uma transformaçao
    if(new->nrTransf == 1) {
        int pid;

        if ((pid = fork()) == 0){
          char* transformacao = strdup(listaTransf[new->transformations[0]]->executavel);
        
          transformacao = strcat(path,transformacao);

          int fd_output = open(new->output, O_CREAT | O_WRONLY | O_TRUNC, 0666);
          int fd_input = open(new->input, O_RDONLY);
          dup2(fd_output,1);
          close(fd_output);
          dup2(fd_input,0);
          close(fd_input);
          execlp(transformacao,transformacao,NULL);
          _exit(0);
        }
          
        wait(&status[0]);
      
    } else {

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
                      char* transformacao = strdup(listaTransf[new->transformations[i]]->executavel);
          
                      transformacao = strcat(path,transformacao);
                      
                      close(p[i][0]);

                      dup2(p[i][1],1);
                      close(p[i][1]);

                      int fd_input = open(new->input, O_RDONLY);
                      dup2(fd_input,0);
                      close(fd_input);

                      execlp(transformacao,transformacao,NULL);

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
                      char* transformacao = strdup(listaTransf[new->transformations[i]]->executavel);
          
                      transformacao = strcat(path,transformacao);
                      dup2(p[i-1][0],0);
                      close(p[i-1][0]);

                      int fd_output = open(new->output, O_CREAT | O_WRONLY | O_TRUNC, 0600);
                      dup2(fd_output,1);
                      close(fd_output);

                      execlp(transformacao,transformacao,NULL);

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
                      char* transformacao = strdup(listaTransf[new->transformations[i]]->executavel);
          
                      transformacao = strcat(path,transformacao);
                      close(p[i][0]);
                      dup2(p[i-1][0],0);
                      close(p[i-1][0]);

                      dup2(p[i][1],1);
                      close(p[i][1]);

                      execlp(transformacao,transformacao,NULL);

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
      }

    }

    int fd_input = open(new->input, O_RDONLY);
    int fd_output = open(new->output, O_RDONLY);
    
    //lê os bytes do input e output
    size_input = lseek(fd_input, 0, SEEK_END);
    size_output = lseek(fd_output, 0, SEEK_END);

    char buffer[1024];
    
    strcpy(buffer,"concluded (bytes-input: ");
    strcat(buffer, itoa(size_input));
    strcat(buffer, ", bytes-output: ");
    strcat(buffer, itoa(size_output));
    strcat(buffer, ")\n");
    strcat(buffer, "\0");
    int i;
    for (i = 0; buffer[i] != '\0'; i++);
    
    write(new->fd, buffer, i * sizeof(char));
    close(new->fd);

    
    Message m = mensagemConcluded(new->nrTarefa);

    //envia mensagem de tipo END para informar que a tarefa foi concluida
    write(fdServer_Server, &m, sizeof(struct message));
    close(fdServer_Server);

    return 1;
}

//sig_handler caso receba um SIGQUIT (ctrl + \)
void terminaServer(int signum){
    switch (signum){
    case SIGQUIT:
        close(fdServer_Server);
        unlink("fifo_Clients_Server");
        break;
    }
}

// envia para o cliente o estado da minHeap dos pedidos que ficam pending
void printListaPedidos(minHeap* l, int fd) {
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
        Tarefa* task = malloc(sizeof(Tarefa));
        task = l->elem[i].task;
        
        strcat(buffer,itoa(task->nrTarefa));
        strcat(buffer,": proc-file");
        strcat(buffer,task->input);
        strcat(buffer," ");
        strcat(buffer,task->output);
        
        for (int j = 0; j < task->nrTransf; j++){
            strcat(buffer," ");
            strcat(buffer,listaTransf[task->transformations[j]]->executavel);
        }
        strcat(buffer, "\n");
    }
    write(fd, buffer, strlen(buffer));
  }
}

// envia para o cliente o estado do array de Transform's (nome/curr/mas)
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

//Verifica se tem todas as transformações de uma Tarefa disponiveis (curr < max)
int transfDisponivel(Transform** l, Tarefa* tarefa) {
    int res = 0;
    for (int i = 0; !res && (i < tarefa->nrTransf); i++){

        if((l[tarefa->transformations[i]]->curr == l[tarefa->transformations[i]]->max)) res++;
    }
    return res;
}

// Incrementa o fator curr da struct Transform;
Transform** transfInc(Transform** l, int transf) {
    if(l[transf]->curr < l[transf]->max) {
      l[transf]->curr+= 1;
    }  
    return l;
}

// Decrementa o fator curr da struct Transform;
Transform** transfDec(Transform** l, int transf) {
    if(l[transf]->curr > 0) {
      l[transf]->curr-= 1;
    }
    return l;
}

int main(int argc, char const *argv[]) {
    // AVL de pedidos em execvuçao
    struct Node *execucao = malloc(sizeof(struct Node));

    //minHeap de pedidos pending
    minHeap* pendentes = initMinHeap();

    // Cria fifo de receçao de pedidos
    if (mkfifo("fifo_Clients_Server", 0666))
        perror("Mkfifo");
        
    // signal para o sigHandler do SIGQUIT  
    signal(SIGQUIT, terminaServer);

    Message buffer;

    // path para os executaveis das transformaçoes (segundo argumento)
    char* transFolder = strdup(argv[2]);
    
    // Cria o array de structs Transform para 7 transformaçoes
    listaTransf = malloc(sizeof(struct transform) * 7);

    // lê a configuração de transformaçoes do ficheiro fornecido (primeiro argumento)
    lerConfig (argv[1]);

    // Abre o fifo para ler os pedidos dos clientes
    if((fdClienteServidor = open("fifo_Clients_Server", O_RDONLY)) == -1){
            perror("Error opening fifo_Clients_Server1\n");
        }

    // Abre o fifo para "escrever" de forma ter sempre um fd de escrita e nao ocorrer "EOF"
    // Alternativa ao while(1)
    if ((fdServer_Server = open("fifo_Clients_Server", O_WRONLY)) == -1) {
        perror("Error opening fifo_Clients_Server\n");
        return -1;
        }

    // enquanto nao for encerrado o servidor (fechado o fdServer_Server)
    while (read(fdClienteServidor, &buffer,sizeof(Message))){
        int pid;
        
        //Caso recebe uma Message do tipo STATUS
        if(buffer.type == STATUS) {
            if ((pid = fork()) == 0){
                int fdServidorCliente = open(itoa(buffer.pid), O_WRONLY);
                printListaPedidos(pendentes,fdServidorCliente);
                printListaTransf(listaTransf,fdServidorCliente);
                close(fdServidorCliente);
                _exit(0);
            }
                        
        }
        
        //Caso recebe uma Message do tipo INFO
        else if(buffer.type == INFO) {
                if ((pid = fork()) == 0) {
                    int fdServidorCliente = open(itoa(buffer.pid), O_WRONLY);
                    help(fdServidorCliente);
                    close(fdServidorCliente);
                    _exit(0);
                }
            }

        //Caso recebe uma Message do tipo PROCFILE
        else if(buffer.type == PROCFILE){

            //cria uma struct Tarefa a partir da Message recebida
            Tarefa* new = novaTarefa(buffer, tStamp, nrTarefa);
            // incrementa o timeStamp
            tStamp++;
            //incrementa o nrTarefa
            nrTarefa++;
            
            // Verifica se é possivel efetuar o pedido
            if (transfDisponivel(listaTransf, new))
                
                // Caso nao seja possivel, mete o pedido em modo pendente (adiciona a minHeap "pendentes")
                adicionaPendente(new, new->fd_outFIFO, pendentes);
            
            else {

                // Caso seja possivel efetuar, incrementa o número de cada transformaçao que é utilizada (curr)
                for(int t = 0; t < new->nrTransf; t++)
                    listaTransf = transfInc(listaTransf,new->transformations[t]);

                // Adiciona a Tarefa à AVL de tarefas em execução
                adicionaExecucao(new, execucao);
                
                // Cria filho para executar a tarefa 
                if ((pid = fork()) == 0){
                    int fd = open(itoa(new->fd_outFIFO), O_WRONLY);
                    dup2(fd, new->fd);
                    close(fd);
                    executar(transFolder, new);
                    exit(0);
                }
            }            
        }
        
        //Caso recebe uma Message do tipo END (termino da execução uma Tarefa)
        else if(buffer.type == END){
            int nrT = buffer.pid;
            
            Tarefa* task = malloc(sizeof(struct tarefa));

            // Vais buscar a Tarefa que terminou à AVL das Tarefas em execucao
            task = node_getEstruturaAVL(execucao, nrT);

            // Elimina a Tarefa que terminou à AVL das Tarefas em execucao
            execucao = deleteNodeAVL(execucao, nrT, task);
            
            // Decrementa o número de cada transformaçao que é utilizada (curr)
            for(int t = 0; t < task->nrTransf; t++)
                transfDec(listaTransf,task->transformations[t]);
                
            //Caso haja pedidos pendentes
            if(pendentes->size > 0){
              
              // Vais buscar a Tarefa à raiz da minHeap das Tarefas em minHeap (pedido com menor timeStamp)
              task = node_getEstruturaMinH (pendentes);

              // Verifica se é possivel efetuar o pedido
              if (!transfDisponivel(listaTransf, task)){
                                
                // Caso seja possivel efetuar, elimina a raiz da minHeap das Tarefas em minHeap (pedido com menor timeStamp)
                pendentes = deleteNode(pendentes);

                // Incrementa o número de cada transformaçao que é utilizada (curr)
                for(int t = 0; t < task->nrTransf; t++)
                  listaTransf = transfInc(listaTransf,task->transformations[t]);

                // Adiciona a Tarefa à AVL de tarefas em execução
                adicionaExecucao(task, execucao);

                // Cria filho para executar a tarefa 
                if (((pid = fork()) == 0) ){
                  executar(transFolder, task);
                  _exit(0);
                }
                close(task->fd);
                
              }
                
            }
            free(task);
        
        }
    }    
    return 0;
}