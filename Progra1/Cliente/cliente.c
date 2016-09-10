#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
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
int infoProcesos[3];//contiene PID, BURST, PRIORIDAD
pthread_t threads[100];
int contadorThreads = 0;
int variablesThreads[100][3];
int activeThreads = 0;


int main();

main(){
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
menu(){
	printf("Selecciona el tipo de cliente:\n");
    printf("1. Manual\n");
    printf("2. Automático\n");
    scanf("%s", tipoCliente);
}

procesarString(){
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

quitaN(char *string, size_t n){
	//qiita los primeros n digitos de un string
    size_t len = strlen(string);
    if (n > len)
        return;  // Or: n = len;
    memmove(string, string + n, len - n + 1);
}



void *accionThread(void *pointer){
	int idThread = (intptr_t) pointer;
	//lo que viene es una pequeña prueba, aquí ser haría la conexión al socket
	int tiempoRestante = variablesThreads[idThread][1];
	while(tiempoRestante != 0){
		printf("Thread %d, le faltan %d segundos.\n", idThread, tiempoRestante);
		sleep(1);
		tiempoRestante--;
	}

	printf("Finaliza Thread");

	socketConnection(); //prueba

	printf("Finaliza Thread %d\n", idThread);
	activeThreads--;
	return NULL; //finaliza thread
}

generarThread(){
	variablesThreads[contadorThreads][0] = infoProcesos[0];
	variablesThreads[contadorThreads][1] = infoProcesos[1];
	variablesThreads[contadorThreads][2] = infoProcesos[2];
	pthread_create(&threads[contadorThreads], NULL, &accionThread, (void *) (intptr_t) contadorThreads);
	contadorThreads++;
	activeThreads++;
}

mainAutomatico(){
	printf("Apreta la tecla e para salir.\n");

    char c = 0;
    int contador = 1;
    while (c != 'e') {
        initscr();
		timeout(100); //se le da un tiempo para esperar une entrada
		c = getch();
		endwin();

		sleep(rand() % 3 + 1);//random de 1 a 3 segundos
		infoProcesos[0] = contador;
		contador++;
		infoProcesos[1]= rand() % 20 + 1; //burst random de 1 a 20 segundos
		infoProcesos[2]= rand() % 5 + 1; //prioridad random de 1 a 20 segundos		
		
        generarThread();
    }
}
mainManual(){
   FILE *fp;
   fp = fopen(pathArchivo, "r");
   if(fp == NULL) 
   {
      perror("Error opening file");
      return(-1);
   }
   else{
	   while(fgets(bufferArchivo, 30, (FILE*)fp) != NULL){
		    sleep(rand() % 3 + 1);//random de 1 a 3 segundos
		    procesarString();
		    generarThread();
	    }
	   fclose(fp);
	}
	while(activeThreads != 0){
		int cacaConCarne;
	}
}

void socketConnection(){
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

    //establishes connection, error if not
    if (connect(socketCon,(struct sockaddr *) &serverAddress,sizeof(serverAddress)) < 0) 
        error("ERROR connecting");

    char buffer[256] = "Proceso X!"; //aqui seria el mensaje
    
    n = write(socketCon,buffer,strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");

    bzero(buffer,256);
    
    n = read(socketCon,buffer,255);
    
    if (n < 0) 
         error("ERROR reading from socket");
    
    printf("%s\n",buffer);
    
    close(socketCon);
}

void error(const char *msg) //called when system call fails.
{
    perror(msg);
    exit(0);
}