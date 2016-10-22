#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <semaphore.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define true 1
#define false 0

//Variables Globales
int cantidadThreads;
int tiempoDormido;
int tiempoLeyendo;
int cantReaders;
char scantidadEscritores[2];
char stiempoDormido[3];
char stiempoLeyendo[3];
char mensajeBitacora[152];//usado para el mensaje para escribir en bitácora

int shmid;
key_t key = 666; //Helo
char *shm; //apunta al inicio de memoria compartida

//Semaforos
char SEM_readTry[] = "readTry";
char SEM_resource[] = "resource";
sem_t *semaphoreReadTry;
sem_t *semaphoreResource;
pthread_mutex_t memoriaEspia;
pthread_mutex_t counter;



pthread_t threads[100];
int contadorThreads = 0;
int variablesThreads[100][3];//se guarda pid  | y linea actual y estado 
// 0 : acceso
// 1 : dormido
// 2 : bloqueado
int cantidadThreadsRestantes = 0;

char bufferLinea[66];

int main();
void menu();
int puedeLeer(int lineaActual);
int getContador();
void setContador();
void setLecturaSeguida();
void crearThreads();
void *accionThread(void *pointer);
void imprimirArchivo();
void leerLinea(int lineaActual);
void escribirArchivo(int pID);
int iniciarSemaforos();

void menu(){
    printf("Ingresa la cantidad de lectores:\n");
    scanf("%s", scantidadEscritores);
    printf("Ingresa el tiempo que duraran leyendo:\n");
    scanf("%s", stiempoLeyendo);
    printf("Ingresa el tiempo que duraran dormidos\n");
    scanf("%s", stiempoDormido);
    cantidadThreads = atoi(scantidadEscritores);
    tiempoDormido = atoi(stiempoDormido);
    tiempoLeyendo = atoi(stiempoLeyendo);
}

int puedeLeer(int lineaActual){
    char *s = shm + 3612;
    int offset = 66*lineaActual;
    s = shm + 3612 + offset;
    if(*s == '-'){
        return true;
    }
    return false;
}

int siguienteLinea(int lineaActual){
    //devuelve la siguiente linea donde se puede escribir respecto a la lineaActual
    //falta el caso en que si llega al final, que comience otra vez
    char *s = shm + 3612;
    int lineaReal = lineaActual;
    int offset = 66*lineaActual;
    s += offset;
    //printf("Char %c \n",s[0]);
    while (true){
        if(*s == '-'){ //este método devuelve línea 0, pero abajo se va hacia uno.
            return lineaReal;
        }
        if(*s == '+'){
            s = s + 66;
            lineaReal++;
        }
        if(*s == '*'){
            s = shm + 3612;
            lineaReal = 0;
        }
    }
    return 0; //nunca deberia ejecutarse
}

int getBandera(){
    int *p = (int *)shm;
    p = p + 2;
    return *p;
}

int getContador(){
    int *p = (int *)shm;
    return *p;
}

void setLecturaSeguida(){
    int *p = (int *)shm;
    p++;//nos movemos hacia arriba
    *p = 0;
}

void setContador(){
    //aumenta en 1 el contador
    int *p = (int *)shm;
    *p = *p + 1;
}

void paraEspia(){
    int *p = (int *)shm;
    p = p + 303;
    *p = cantidadThreads;
    p++;
    for(int i = 0; i < cantidadThreads ; i++){
        *p = variablesThreads[i][0]; //id
        p++;
        *p = variablesThreads[i][2]; //estado
        p++;
    }
}

void *accionThread(void *pointer){
    int idThread = (intptr_t) pointer;//id local para el array de 100
    int pID = variablesThreads[idThread][0];// id verdadero del programa
    int lineaActual = variablesThreads[idThread][1];
    while(getBandera() == 1){
        variablesThreads[idThread][2] = 2; //ponemos estado bloqueado

        pthread_mutex_lock(&memoriaEspia);
            paraEspia();
        pthread_mutex_unlock(&memoriaEspia); 

        sem_wait(semaphoreReadTry); //indica que un reader quiere entrar
        pthread_mutex_lock(&counter); // lock del contador y obtener solo un recurso para todos los readers

        if(getBandera() != 1){
            sem_post(semaphoreReadTry);
            pthread_mutex_unlock(&counter); // libera lock contador
            break;
        }
        
        cantReaders++;
        if(cantReaders == 1){
            sem_wait(semaphoreResource); //lock del recurso compartido
            if(getBandera() != 1){
                sem_post(semaphoreResource);
                break;
            }
        }
        pthread_mutex_unlock(&counter); // libera lock contador
        sem_post(semaphoreReadTry); // libera permiso de entrar


        variablesThreads[idThread][2] = 0; //ponemos estado escribiendo
        paraEspia(); 

        if(puedeLeer(lineaActual)){
            printf("--------------------------------------------\n");
            sleep(tiempoLeyendo);
            leerLinea(lineaActual);
            lineaActual++;
            escribirArchivo(pID);
            setLecturaSeguida();
        }

        imprimirArchivo();
        pthread_mutex_lock(&counter); // lock del contador y obtener solo un recurso para todos los readers
        cantReaders--;
        if(cantReaders == 0){
            sem_post(semaphoreResource);
        }
        variablesThreads[idThread][2] = 1; //ponemos estado dormido
        pthread_mutex_unlock(&counter);

        pthread_mutex_lock(&memoriaEspia);
            paraEspia();
        pthread_mutex_unlock(&memoriaEspia);

        sleep(tiempoDormido);
    }
    printf("Finaliza Thread %d\n", idThread);
    cantidadThreadsRestantes--;
    return NULL; //finaliza thread
}

void crearThreads(){
    cantReaders = 0;
    for(int i = 0; i < cantidadThreads ; i++){
        variablesThreads[contadorThreads][0] = getContador();;
        variablesThreads[contadorThreads][1] = 0;//todos inician en la línea 0
        pthread_create(&threads[contadorThreads], NULL, &accionThread, (void *) (intptr_t) contadorThreads);
        setContador();//aumentamos en uno
        contadorThreads++;
        cantidadThreadsRestantes++;
    }
}

void imprimirArchivo(){
    char *s;
    for (s = shm + 3612; *s != '*'; s++){
        putchar(*s);
    }
}

void leerLinea(int lineaActual){
    char *s = shm + 3612;
    int offset = 66*lineaActual;
    int contador = 0;
    for(s += offset; *s!='\n';s++){
        bufferLinea[contador] = *s;
        contador++;
    }
}

void escribirArchivo(int pID){
    char fecha[19];

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(fecha,"%d_%d_%d %d:%d:%d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    sprintf(mensajeBitacora, "El proceso %d [reader] leyo lo siguiente: {%s} en fecha: %s.", pID,bufferLinea, fecha);

    FILE *f = fopen("../Bitacora.txt", "a");
    if (f == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }
    fprintf(f, "%s\n", mensajeBitacora);
    fclose(f);
}

int iniciarSemaforos(){

    if (pthread_mutex_init(&memoriaEspia, NULL) != 0)
    {
        printf("\n mutex memoriaEspia init failed\n");
        return 1;
    }

    if (pthread_mutex_init(&counter, NULL) != 0)
    {
        printf("\n mutex counter init failed\n");
        return 1;
    }

    semaphoreReadTry = sem_open(SEM_readTry,0,0644,0);
    if(semaphoreReadTry == SEM_FAILED){
        perror("unable to create semaphore");
        sem_unlink(SEM_readTry);
        exit(-1);
    }

    semaphoreResource = sem_open(SEM_resource,0,0644,0);
    if(semaphoreResource == SEM_FAILED){
        perror("unable to create semaphore");
        sem_unlink(SEM_resource);
        exit(-1);
    }

}

int main(){
    menu();
    iniciarSemaforos();

    /*
     * Locate the segment. Si existe falla.
     */
    if ((shmid = shmget(key, 10, 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    /*
     * Now we attach the segment to our data space.
     * Conseguimos un puntero a la primera parte de la memoria
     */
    if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }
    crearThreads();
    while(cantidadThreadsRestantes != 0){
        int caca;
    }

    pthread_mutex_destroy(&memoriaEspia);

    printf("Finaliza ejecución de los readers\n");
}

