#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <semaphore.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#define true 1
#define false 0

//Variables Globales
int cantidadThreads;
int tiempoDormido;
int tiempoLeyendo;
int cantWriters;
char scantidadEscritores[2];
char stiempoDormido[3];
char stiempoLeyendo[3];
char mensajeBitacora[122];//usado para el mensaje para escribir en bitácora

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


int main();
void menu();
int archivoLleno();//devuelve si el archivo esta lleno actualmente
int siguienteLinea(int lineaActual);// devuelve la siguiente linea vacía
int getBandera();
int getContador();
void paraEspia();//pone en memoria compartida el estado de los writers
void setContador();
void crearThreads();
void setLecturaSeguida();//la pone en cero
void *accionThread(void *pointer);
void imprimirArchivo();
void escribirLinea(int linea, int idProceso);
void escribirArchivo();
int iniciarSemaforos();


void menu(){
    printf("Ingresa la cantidad de escritores:\n");
    scanf("%s", scantidadEscritores);
    printf("Ingresa el tiempo duraran escribiendo:\n");
    scanf("%s", stiempoLeyendo);
    printf("Ingresa el tiempo que duraran dormidos\n");
    scanf("%s", stiempoDormido);
    cantidadThreads = atoi(scantidadEscritores);
    tiempoLeyendo = atoi(stiempoLeyendo);
    tiempoDormido = atoi(stiempoDormido);
    printf("%s\n", stiempoLeyendo);
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

void setContador(){
    //aumenta en 1 el contador
    int *p = (int *)shm;
    *p = *p + 1;
}

void setLecturaSeguida(){
    int *p = (int *)shm;
    p++;//nos movemos hacia arriba
    *p = 0;
}

int archivoLleno(){
    char *s;
    for (s = shm + 3612; *s != '*'; s++){
        if (*s == '+'){// si se encuentra un más, es que al menos hay una línea vacía
            return false;
        }
    }
    return true;
}

void paraEspia(){
    int *p = (int *)shm;
    p = p + 3;
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
    int idThread = (intptr_t) pointer;
    int pID = variablesThreads[idThread][0];
    int lineaActual = variablesThreads[idThread][1];
    paraEspia(); //NO SE si esto hay que sincronizarlo
    while(getBandera() == 1){
        variablesThreads[idThread][2] = 2; //ponemos estado bloqueado

        pthread_mutex_lock(&memoriaEspia);
            paraEspia();
        pthread_mutex_unlock(&memoriaEspia); 
        
        pthread_mutex_lock(&counter);
        cantWriters++;
        if(cantWriters == 1){
            sem_wait(semaphoreReadTry);
            if(getBandera() != 1){
                sem_post(semaphoreReadTry);
                pthread_mutex_unlock(&counter);
                break;
            }
        }
        pthread_mutex_unlock(&counter);


        sem_wait(semaphoreResource);
        if(getBandera() != 1){
            sem_post(semaphoreResource);
            break;
        }

        variablesThreads[idThread][2] = 0; //ponemos estado escribiendo
        paraEspia(); //NO SE si esto hay que sincronizarlo
        if (!archivoLleno()){
            lineaActual = siguienteLinea(lineaActual);
            printf("--------------------------------------------\n");
            sleep(tiempoLeyendo);
            escribirLinea(lineaActual, pID);
            setLecturaSeguida();
        }
        imprimirArchivo();
        sem_post(semaphoreResource);

        pthread_mutex_lock(&counter);
        cantWriters--;
        if(cantWriters == 0){
            sem_post(semaphoreReadTry);
        }
        pthread_mutex_unlock(&counter);
        
        variablesThreads[idThread][2] = 1; //ponemos estado dormido

        pthread_mutex_lock(&memoriaEspia);
            paraEspia();
        pthread_mutex_unlock(&memoriaEspia); 
        
        sleep(tiempoDormido);
    }
    cantidadThreadsRestantes--;
    return NULL; //finaliza thread
}

void crearThreads(){
    cantWriters = 0;
    for(int i = 0; i < cantidadThreads ; i++){
        variablesThreads[contadorThreads][0] = getContador();
        variablesThreads[contadorThreads][1] = 0;//todos inician en la línea 0
        pthread_create(&threads[contadorThreads], NULL, &accionThread, (void *) (intptr_t) contadorThreads);
        setContador();//aumentamos en uno
        contadorThreads++;
        cantidadThreadsRestantes++;
    }
}

int siguienteLinea(int lineaActual){
    //devuelve la siguiente linea donde se puede escribir respecto a la lineaActual
    char *s = shm + 3612;
    int lineaReal = lineaActual;
    int offset = 66*lineaActual;//por alguna razón no van en bloques de 66, sino de 67 
    s += offset;
    //printf("Linea real(antes de): %d. Offset :%d\n", lineaReal,offset);
    while (true){
        if(*s == '+'){
            //printf("Linea real(después de): %d\n", lineaReal);
            return lineaReal;
        }
        if(*s == '-'){
            lineaReal++;
        }
        if(*s == '*'){
            s = shm;
            lineaReal = 0;
        }
        s = s + 1;
    }
}

void escribirLinea(int linea, int idProceso){
    //escribe una linea, en la posicion lineaActual
    char *s = shm + 3612;
    int offset = 66*linea;//por alguna razón no van en bloques de 66, sino de 67 
    s += offset;
    char mensaje[66];
    char fecha[19];

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(fecha,"%d_%d_%d %d:%d:%d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    sprintf(mensaje,"- Proceso: %d escribe en linea %d, ",idProceso,linea);
    strcat(mensaje,fecha);
    //ahora escribimos en bitacora
    sprintf(mensajeBitacora, "El proceso %d [writer] escribio lo siguiente: {%s}.", idProceso,mensaje);
    escribirArchivo();
    strcat(mensaje, "     \n");
    strcpy(s, mensaje);

}

void imprimirArchivo(){
    char *s;
    for (s = shm + 3612; *s != '*'; s++){
        putchar(*s);
    }
}

void escribirArchivo(){
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
    printf("Finaliza ejecución de los writers\n");
}

