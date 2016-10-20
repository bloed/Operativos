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
char scantidadEscritores[2];
char stiempoDormido[2];
char stiempoLeyendo[2];
char mensajeBitacora[122];//usado para el mensaje para escribir en bitácora

int shmid;
key_t key = 666; //Helo
char *shm; //apunta al inicio de memoria compartida

sem_t *semaphore;
char SEM_GEN[] = "gen";

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
void setContador();
void crearThreads();
void *accionThread(void *pointer);
void imprimirArchivo();
void escribirLinea(int linea, int idProceso);
void escribirArchivo();


void menu(){
    printf("Ingresa la cantidad de escritores:\n");
    scanf("%s", scantidadEscritores);
    printf("Ingresa el tiempo duraran escribiendo:\n");
    scanf("%s", stiempoLeyendo);
    printf("Ingresa el tiempo que duraran dormidos\n");
    scanf("%s", stiempoDormido);
    cantidadThreads = atoi(scantidadEscritores);
    tiempoDormido = atoi(stiempoDormido);
    tiempoLeyendo = atoi(stiempoLeyendo);
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

int archivoLleno(){
    char *s;
    for (s = shm + 3612; *s != '*'; s++){
        if (*s == '+'){// si se encuentra un más, es que al menos hay una línea vacía
            return false;
        }
    }
    return true;
}

void *accionThread(void *pointer){
    int idThread = (intptr_t) pointer;
    int pID = variablesThreads[idThread][0];
    int lineaActual = variablesThreads[idThread][1];
    while(getBandera() == 1){
        variablesThreads[idThread][2] = 2; //ponemos estado bloqueado
        sem_wait(semaphore);
        variablesThreads[idThread][2] = 0; //ponemos estado escribiendo
        if (!archivoLleno()){
            lineaActual = siguienteLinea(lineaActual);
            printf("--------------------------------------------%d\n",pID);
            sleep(tiempoLeyendo);
            escribirLinea(lineaActual, pID);
        }
        imprimirArchivo();
        sem_post(semaphore);
        variablesThreads[idThread][2] = 1; //ponemos estado dormido
        sleep(tiempoDormido);
    }
    cantidadThreadsRestantes--;
    return NULL; //finaliza thread
}

void crearThreads(){
    for(int i = 0; i < cantidadThreads ; i++){
        variablesThreads[contadorThreads][0] = getContador();
        variablesThreads[contadorThreads][1] = 0;//todos inician en la línea 0
        pthread_create(&threads[contadorThreads], NULL, &accionThread, (void *) (intptr_t) getContador());
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

int main(){
    menu();

    semaphore = sem_open(SEM_GEN,0,0644,0);
    if(semaphore == SEM_FAILED){
        perror("unable to create semaphore");
        sem_unlink(SEM_GEN);
        exit(-1);
    }
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

