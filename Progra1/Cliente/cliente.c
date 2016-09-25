#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <ncurses.h>
#include <pthread.h>

#include <unistd.h>
#include <string.h>
#include <sys/types.h> //Data types for system calls
#include <sys/socket.h> //Lib for sockets
#include <netinet/in.h> //internet domain addresses, not sutre if needed
#include <netdb.h>

//Variables Globales:
char tipoCliente[1];
char pathArchivo[20];
char bufferArchivo[30];
char bufferMensaje[50]; //Datos enviados del proceso
int infoProcesos[3];//contiene PID, BURST, PRIORIDAD
pthread_t threads[100];
int contadorThreads = 0;
int variablesThreads[100][3];
int activeThreads = 0;


int main();
void menu();
void procesarString();
int getELemento();
void quitaN(char *string, size_t n);
void *accionThread(void *pointer);
void generarThread();
void mainAutomatico();
int mainManual();
void socketConnection();
void error(const char *msg);

/* Zona de Listas*/
typedef struct node {
    int pid;
    int burst;
    
    int prioridad;
    int tat;
    int wt;
    int estado; //1 ejecución, 0 terminado, 2 Run.  
    struct node *next;
} node_t;

typedef struct list 
{
    node_t *head;

}list_t;


/* node_t * head = NULL;
head = malloc(sizeof(node_t));*/

//inserta al final de la lista
void push(list_t *list, int pPId, int pBurst, int pPrioridad, int pTat,
	int pWt, int pEstado) {
    node_t *node = malloc(sizeof(node_t));
    node->next = NULL;
    node->pid = pPId;
	node->burst = pBurst;
	node->prioridad = pPrioridad;
	node->tat = pTat;
	node->wt = pWt;
	node->estado = pEstado;
    if(list->head == NULL){ //la lista está vacía;
    	list->head = node;
    }
    else{
    	node_t *current = list->head;
	    while (current->next != NULL) {
	        current = current->next;
	    }
	    /* now we can add a new variable */
	    current->next = node;
	}
}

void printList(list_t *list) {
	//la lista no puede estar vacía, o se cae
    node_t * current = list->head;
    char sestado[15];
    while (current != NULL) {
    	if(current->estado == 0){
    		strcpy(sestado, "Finalizado");
    	}
    	else{
    		strcpy(sestado, "Ejecución");
    	}
        printf("Pid: %d | Burst: %d | prioridad: %d | TAT: %d | WT : %d | Estado: %s.\n", current->pid, current->burst,
        	current->prioridad, current->tat, current->wt, sestado);
        current = current->next;
    }
}

/* FIN zona de listas */

int main(){
	srand(time(NULL));
	menu();
	if(strcmp(tipoCliente, "1") == 0){
		printf("Se escogió cliente manual.\n");
		printf("Ingresa el archivo a con la lista de procesos:\n");
		scanf("%s", pathArchivo);
		mainManual();

	};
	if(strcmp(tipoCliente, "2") == 0){
		printf("Se escogió cliente automático.\n");
		mainAutomatico();
	};
	return 0;
} 
void menu(){
	printf("Selecciona el tipo de cliente:\n");
    printf("1. Manual\n");
    printf("2. Automático\n");
    scanf("%s", tipoCliente);
}

void procesarString(){
	//cochino
	//conseguimos id
	int indice = getELemento();
	char pid[indice+1];
	strncpy (pid, bufferArchivo , indice);
	pid[indice] = '\0';
	infoProcesos[0] = atoi(pid); //convierte string a int
	strncpy (bufferArchivo, bufferArchivo , indice);
	quitaN(bufferArchivo, indice + 1);

	//conseguimos burst
	indice = getELemento();
	char burst[indice+1];
	strncpy (burst, bufferArchivo , indice);
	burst[indice] = '\0';
	infoProcesos[1] = atoi(burst);
	strncpy (bufferArchivo, bufferArchivo , indice);
	quitaN(bufferArchivo, indice + 1);

	//conseguimos prioridad
	indice = getELemento();
	char prioridad[indice+1];
	strncpy (prioridad, bufferArchivo , indice);
	prioridad[indice] = '\0';
	infoProcesos[2] = atoi(prioridad);
}

int getELemento(){
	//devuelve un elemento del string bufferArchivo
	const char *elemento;
	int indice;
	elemento = strchr(bufferArchivo, '|');
	indice = (int)(elemento - bufferArchivo);
	return indice;
}

void quitaN(char *string, size_t n){
	//qiita los primeros n digitos de un string
    size_t len = strlen(string);
    if (n > len)
        return;  // Or: n = len;
    memmove(string, string + n, len - n + 1);
}



void *accionThread(void *pointer){
	int idThread = (intptr_t) pointer;

	int pID = variablesThreads[idThread][0];
	int burst = variablesThreads[idThread][1];
	int prioridad = variablesThreads[idThread][2];

	socketConnection(pID,burst,prioridad);

	printf("Finaliza Thread %d\n", idThread);
	activeThreads--;
	return NULL; //finaliza thread
}

void generarThread(){
	variablesThreads[contadorThreads][0] = infoProcesos[0];
	variablesThreads[contadorThreads][1] = infoProcesos[1];
	variablesThreads[contadorThreads][2] = infoProcesos[2];
	pthread_create(&threads[contadorThreads], NULL, &accionThread, (void *) (intptr_t) contadorThreads);
	contadorThreads++;
	activeThreads++;
}

void mainAutomatico(){
	printf("Apreta la tecla e para salir.\n");

    char c = 0;
    int contador = 1;
    while (c != 'e') {
        initscr();
		timeout(100); //se le da un tiempo para esperar une entrada
		c = getch();
		endwin();

		sleep(rand() % 10 + 1);//random de 1 a 10 segundos
		infoProcesos[0] = contador;
		contador++;
		infoProcesos[1]= rand() % 20 + 1; //burst random de 1 a 20 segundos
		infoProcesos[2]= rand() % 5 + 1; //prioridad random de 1 a 20 segundos		
		
        generarThread();
    }
}
int mainManual(){
   FILE *fp;
   fp = fopen(pathArchivo, "r");
   if(fp == NULL) 
   {
      perror("Error opening file");
      return(-1);
   }
   else{
	   while(fgets(bufferArchivo, 30, (FILE*)fp) != NULL){
		    sleep(rand() % 10 + 1);//random de 1 a 10 segundos
		    procesarString();
		    generarThread();
	    }
	   fclose(fp);
	}
	while(activeThreads != 0){
		int cacaConCarne;
	}
}

void socketConnection(int pID, int burst, int prioridad){
	int socketCon; 
	int portNumber = 1234;
	int n; // value of the write and read

	struct hostent *server; //pointer to struct on hader file metdb.h, defines a host computer on the internet.
    struct sockaddr_in serverAddress; //

    

    socketCon = socket(AF_INET, SOCK_STREAM, 0);
    if (socketCon < 0) 
        error("ERROR opening socket");
    
    server = (struct hostent *) gethostbyname("localhost");

    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serverAddress, sizeof(serverAddress)); //zeros on the struct

    //assign value to he struct
    serverAddress.sin_family = AF_INET;
    //copies character string values from one struct to another
    bcopy((char *)server->h_addr, 
         (char *)&serverAddress.sin_addr.s_addr,
         server->h_length);

    serverAddress.sin_port = htons(portNumber);

    char auxiliar[50];
	char message[50];

    sprintf(auxiliar,"PID: %d ",pID);
    strcat(message,auxiliar);
    bzero(auxiliar,50);

    sprintf(auxiliar,"Burst: %d ",burst);
    strcat(message,auxiliar);
    bzero(auxiliar,50);

    sprintf(auxiliar,"Prioridad: %d ",prioridad);
   	strcat(message,auxiliar);

   	printf("Pid: %d Burst: %d Prioridad: %d",pID, burst, prioridad);

    //establishes connection, error if not
    if (connect(socketCon,(struct sockaddr *) &serverAddress,sizeof(serverAddress)) < 0) 
        error("ERROR connecting");

    
    n = write(socketCon,message,strlen(message));
    if (n < 0) 
         error("ERROR writing to socket");
    
    printf("Message Sent \n");
    
    close(socketCon);
}

void error(const char *msg) //called when system call fails.
{
    perror(msg);
    exit(0);
}
