/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>
#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h> //Data types for system calls
#include <sys/socket.h> //Lib for sockets
#include <netinet/in.h> //internet domain addresses, not sutre if needed

/*variables informativas */

int algoritmo;
int quantum;
int secsOcioso;
int indexActual;
int contadorThreads;
int stop;
pthread_t threads[5];

void menu();
void crearThreads();
void listenerTable();
void increaseTimes();
void printCola();
void printatwt();


/* Zona de Listas*/
typedef struct node {
    int pid;
    int burst;
    int tiempoRestante;
    int prioridad;
    int tat;
    int wt;
    int estado; //0 terminado, 1 ready, 2 run
    struct node *next;
} node_t;

node_t *fifo();
node_t *sjf();
node_t *hpf();
node_t *rr();



typedef struct list 
{
    node_t *head;

}list_t;

list_t *lista = NULL;



/* node_t * head = NULL;
head = malloc(sizeof(node_t));*/
void initList(){
    lista = malloc(sizeof(list_t));
    lista->head = NULL;
}
//inserta al final de la lista
void push(int pPId, int pBurst, int pPrioridad, int pEstado, int pTiempoRestante) {
    node_t *node = malloc(sizeof(node_t));
    node->next = NULL;
    node->pid = pPId;
    node->burst = pBurst;
    node->prioridad = pPrioridad;
    node->tat = 0;
    node->wt = 0;
    node->estado = pEstado;
    node->tiempoRestante = pTiempoRestante;
    if(lista->head == NULL){ //la lista está vacía;
        lista->head = node;
    }
    else{
        node_t *current = lista->head;
        while (current->next != NULL) {
            current = current->next;
        }
        /* now we can add a new variable */
        current->next = node;
    }
}

void printList() {
    //la lista no puede estar vacía, o se cae
    node_t * current = lista->head;
    char sestado[15];
    while (current != NULL) {
        if(current->estado == 0){
            strcpy(sestado, "Finalizado");
        }
        if(current->estado == 1){
            strcpy(sestado, "Ready");
        }
        if(current->estado == 2){
            strcpy(sestado, "Run");
        }
        printf("Pid: %d | Burst: %d | prioridad: %d | TAT: %d | WT : %d | Estado: %s.\n", current->pid, current->burst,
            current->prioridad, current->tat, current->wt, sestado);
        current = current->next;
    }
}
/* FIN zona de listas */

void error(const char *msg) //called when system call fails.
{
    perror(msg);
    exit(0);
}

int main()
{
    stop=0;
    indexActual=0;
    contadorThreads=0; //prueba
    menu();
    initList();
    crearThreads();
    listenerTable();

}

void menu(){
    char tipoAlgoritmo[1];
    char cantQuantum[1];
    printf("Selecciona el tipo de algoritmo:\n");
    printf("1. FIFO \n");
    printf("2. SJF\n");
    printf("3. HPF\n");
    printf("4. Round Robin\n");
    scanf("%s", tipoAlgoritmo);
    algoritmo = tipoAlgoritmo[0]-'0';
    if(algoritmo == 4){
        printf("Especifique el Quantum: \n");
        scanf("%s", cantQuantum);
        quantum = cantQuantum[0] - '0';
    }
}

void socketAction(int newSocketCon){
    char buffer[256]; //Received Message size
    int n; //return value of read or write.

    bzero(buffer,256);// turns zero the array
    
    //all socket operations uses the new socketMainConnection

    n = read(newSocketCon,buffer,255);//reads message

    if (n < 0) 
        error("ERROR reading from socket");

    int index = 5;
    int pID,burst,prioridad;

    //Codigo cochino para cambiar strings del mensaje al valor int.
    if(buffer[index+1] == ' '){
        pID = buffer[index] -'0';
        index = index + 9;
    }
    else{
        pID = (buffer[index] -'0') * 10 + buffer[index + 1] - '0';
        index = index + 10;
    }

    if(buffer[index+1] == ' '){
        burst = buffer[index] -'0';
        index = index + 13;
    }
    else{
        burst = (buffer[index] -'0') * 10 + buffer[index + 1] - '0';
        index = index + 14;
    }

    if(buffer[index+1] == ' '){
        prioridad = buffer[index] -'0';
    }
    else{
        prioridad = (buffer[index] -'0') * 10 + buffer[index + 1] - '0';
    }

    // termina codigo cochino

    push(pID,burst,prioridad,1,burst);
    
    
    close(newSocketCon);
}

void *socketConnection(void *pointer){
    int socketMainConnection; //Value returned by socket system call
    int newSocketCon; // Value returned by accept system call
    int portNumber = 1234; //Port number for connections
    int addressClientLen; //Size of the client's address

    struct sockaddr_in serverAdress;
    struct sockaddr_in clientAddress; //structure with internet address protocol sockaddr_in
    

    //Socket creation, INET protocol, Type of message, 0 to let CPU choose protocol
    socketMainConnection = socket(AF_INET, SOCK_STREAM, 0);
    
    if (socketMainConnection < 0) // catch in case socket creation fails.
        error("ERROR opening socket");
    
    //bzero: turns the array (struct in this case) serverAdress into zeros
    bzero((char *) &serverAdress, sizeof(serverAdress));

    //asignacion de valores al struct
    serverAdress.sin_family = AF_INET;
    serverAdress.sin_addr.s_addr = INADDR_ANY; //contains de IP, INADDR_ANY gets the address
    serverAdress.sin_port = htons(portNumber); //portnumber, needs to be converted into network byte order.

    //bind of the socket to the address, if the port is used it fails enters the if
    if (bind(socketMainConnection, (struct sockaddr *) &serverAdress,
              sizeof(serverAdress)) < 0) 
              error("ERROR on binding");
    
    listen(socketMainConnection,5); //starts listening, 5 max connections waiting, cant increase over 5
    
    addressClientLen = sizeof(clientAddress);

    while(stop == 0){

        newSocketCon = accept(socketMainConnection, 
                 (struct sockaddr *) &clientAddress, 
                 &addressClientLen); //waits for a connection

    // Segun la docu el proceso se detiene/traba hasta que llegue una conexion
    // al server, hay que probar como se comportaria esta parte dentro de thread
    //en caso de que trabe todo el proceso o solo el hilo

    if (newSocketCon < 0) //error on accpeted, it executes after a connection happens 
          error("ERROR on accept");
    
    socketAction(newSocketCon);
    }

    //Yo se que nunca llega aca... se me ocurre tener un socket final en el cliente
    //que tenga un formato especial para cambiar la variable paro.
    //sino... hay que esperar 1 min entre cada corrida para que Linux libere el puerto
    close(socketMainConnection);

    return 0;
}

void *CPUscheduler(){
    int burstTime=0;
    node_t *node;
    while(stop == 0){
        switch(algoritmo){
            case 1:
            case 2:
            case 3:
                switch(algoritmo){
                    case 1:
                        node = fifo();
                        break;
                    case 2:
                        node = sjf();
                        break;
                    case 3:
                        node = hpf();
                        break;
                }
                if(node !=NULL ){
                    printf("Proceso: %d con burst %d y prioridad %d entra en ejecucion\n", node->pid, node->burst, node->prioridad);
                    sleep(node->burst);
                    printf("El proceso: %d acaba de salir de ejecucion\n", node->pid);
                    increaseTimes(node->burst,node->pid);
                    node->estado = 0;
                }
                break;
           case 4:
                node = rr();
                if(node !=NULL){
                    if(node->tiempoRestante - quantum > 0){
                        printf("Proceso: %d con burst %d, tiempo restante: %d y prioridad %d entra en ejecucion\n"
                            , node->pid, node->burst, node->tiempoRestante, node->prioridad);
                        sleep(quantum);
                        increaseTimes(quantum, node->pid);
                        node->tiempoRestante = node->tiempoRestante - quantum;
                        node -> estado = 1;
                    }else{
                        printf("Proceso: %d con burst %d, tiempo restante: %d y prioridad %d entra en ejecucion\n"
                            , node->pid, node->burst, node->tiempoRestante, node->prioridad);
                        sleep(node->tiempoRestante);
                        increaseTimes(node->tiempoRestante, node->pid);
                        node->tiempoRestante = 0;
                        printf("El proceso: %d acaba de salir de ejecucion\n", node->pid);
                        node -> estado = 0;
                    }
                }
                break;
        }
        if(node == NULL){
            sleep(1);
            secsOcioso++;
        } 
    }
}

void increaseTimes(int burst, int pID){
    node_t * current = lista->head;
    while (current != NULL) {
        if(current->estado != 0){
            if(current->pid == pID){
                current->tat = current->tat + burst;
            }
            else{
                current->tat = current->tat + burst;
                current->wt = current->wt + burst;        
            }
        }
        current = current->next;
    }
}

void listenerTable(){
    char parada[1];
    printf("Seleccione 1 para ver la cola y 2 para detener la ejecucion en cualquier momento");
    while(stop == 0){
        scanf("%s", parada);
        if(strcmp(parada, "1") == 0){
            printCola();
        }
        if(strcmp(parada, "2") == 0){
            stop=1;

            printatwt();
        }
        if(strcmp(parada,"3") == 0){
            printList();
        }
    }
}


void crearThreads(){
    pthread_create(&threads[0], NULL, &CPUscheduler,(void *) (intptr_t) contadorThreads);
    pthread_create(&threads[1], NULL, &socketConnection,(void *) (intptr_t) contadorThreads);
}

/*Algoritmos de seleccion */

void printatwt(){
    //imprime TAT y WT de los procesos ejecutados
    //tambien imprime el promedio de TAT y WT
    node_t * current = lista->head;
    double promedioTat = 0.0;
    double promedioWt = 0.0;
    int cantidadDeProcesos = 0;
    while(current != NULL){
        if(current->estado == 0){// finalizados
            printf("Pid: %d  | TAT: %d | WT : %d | Estado: Ejecutado.\n", current->pid,
                current->tat, current->wt);
            cantidadDeProcesos += 1.0;
            promedioWt += (double) current->wt;
            promedioTat += (double) current->tat;
        }
        current = current -> next;
    }
    promedioTat = promedioTat / (double)cantidadDeProcesos;
    promedioWt = promedioWt / (double)cantidadDeProcesos;
    printf("Cantidad de procesos: %d\n", cantidadDeProcesos);
    printf("Cantidad de segundos ocioso: %d\n", secsOcioso);
    printf("Promedio TAT: %f\n", promedioTat);
    printf("Promedio WT: %f\n", promedioWt);
}

void printCola(){
    //print los procesos que están en ready.
    node_t * current = lista->head;
    while(current != NULL){
        if(current->estado == 1){//en ready
            printf("Pid: %d | Burst: %d | prioridad: %d | TAT: %d | WT : %d | Tiempo Restante: %d | Estado: Ready.\n",
             current->pid, current->burst,
                    current->prioridad, current->tat, current->wt, current->tiempoRestante);
        }
        current = current->next;
    }
}

node_t *fifo(){
    //devuelve un nodo con el pid más bajo
    node_t * current = lista->head;
    node_t * menorNodo = malloc(sizeof(node_t));
    int menorPid = 99999;//valor muy grande
    while(current != NULL){
        if(current->estado == 1){//solo para procesos en ready
            if(current->pid < menorPid){
                menorPid = current->pid;
                menorNodo = current;
            }
        }
        current = current->next;
    }
    if( menorPid == 99999){
        return NULL;
    }
    else{
        menorNodo->estado = 2;//lo ponemos en ejecución
        return menorNodo;
    }
}

node_t *sjf(){
    //devuelve un nodo con el burst más bajo
    node_t * current = lista->head;
    node_t * menorNodo = malloc(sizeof(node_t));
    int menorBurst = 99999;//valor muy grande
    while(current != NULL){
        if(current->estado == 1){//solo para procesos en ready
            if(current->burst < menorBurst){
                menorBurst = current->burst;
                menorNodo = current;
            }
        }
        current = current->next;
    }
    if(menorBurst == 99999){
        return NULL;
    }
    else{
        menorNodo->estado = 2;//lo ponemos en ejecución
        return menorNodo;
    }
}

node_t *hpf(){
    //devuelve un nodo con la prioridad más alta (que es la más baja)
    node_t * current = lista->head;
    node_t * menorNodo = malloc(sizeof(node_t));
    int menorPrioridad = 99999;//valor muy grande
    while(current != NULL){
        if(current->estado == 1){//solo para procesos en ready
            if(current->prioridad < menorPrioridad){
                menorPrioridad = current->prioridad;
                menorNodo = current;
            }
        }
        current = current->next;
    }
    if(menorPrioridad == 99999){
        return NULL;
    }
    else{
        menorNodo->estado = 2;//lo ponemos en ejecución
        return menorNodo;
    }
}

node_t *rr(){
    int final = 0;
    //devuelve el siguiente nodo en ready
    if(lista->head != NULL){
        node_t * current = lista->head;
        node_t * menorNodo = malloc(sizeof(node_t));
        int copia = indexActual;
        while(copia != 0){
            copia--;
            current = current->next;
        }
        copia = indexActual;
        int primerId = current->pid;
        bool encontrado =  false;
        while(encontrado != true){
            if(primerId != current->pid){
                if(current->estado == 1){// esta en ready
                    current->estado = 2; //lo ponemos en ejecución
                    menorNodo = current;
                    indexActual = copia;
                    return menorNodo;
                }
            }
            if(current->pid == (primerId - 1)){
                indexActual = indexActual; //no cambia nada
                return NULL; //no encontro nada
            }

            if(current->next == NULL && final == 0){//llega a final de la lista
                copia = 0;
                current = lista->head; //comenzamos desde 0
                final = 1;
            }
            else if(current->pid == primerId && current -> estado == 1 && final == 1){
                    current->estado = 2; //lo ponemos en ejecución
                    menorNodo = current;
                    indexActual = copia;
                    return menorNodo;
            }
            else if(current->pid == primerId && current -> estado == 0 && final == 1){
                    return NULL;
            }
            else{
                copia++;
                current = current -> next;
            }
        }
    }
    else 
        return NULL;
}
