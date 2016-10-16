#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

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

int shmid;
key_t key = 666; //Helo
char *shm; //apunta al inicio de memoria compartida

pthread_t threads[100];
int contadorThreads = 0;
int variablesThreads[100][2];//se guarda pid  | y linea actual
// considerar meter estado (dormido, leyendo) y usar pid REAL.
int main();
void menu();
int archivoLleno();//devuelve si el archivo esta lleno actualmente
int siguienteLinea(int lineaActual);// devuelve la siguiente linea vacía
void crearThreads();
void *accionThread(void *pointer);
void imprimirArchivo();
void escribirLinea(int linea, int idProceso);


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

int archivoLleno(){
    char *s;
    for (s = shm; *s != '*'; s++){
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
    while(true){
        if (!archivoLleno()){
            lineaActual = siguienteLinea(lineaActual);
            printf("--------------------------------------------\n");
            sleep(tiempoLeyendo);
            escribirLinea(lineaActual, pID);
        }
        sleep(tiempoDormido);
        imprimirArchivo();
    }

    printf("Finaliza Thread %d\n", idThread);
    return NULL; //finaliza thread
}

void crearThreads(){
    for(int i = 0; i < cantidadThreads ; i++){
        variablesThreads[contadorThreads][0] = contadorThreads;
        variablesThreads[contadorThreads][1] = 2;//todos inician en la línea 0
        pthread_create(&threads[contadorThreads], NULL, &accionThread, (void *) (intptr_t) contadorThreads);
        contadorThreads++;
    }
}

int siguienteLinea(int lineaActual){
    //devuelve la siguiente linea donde se puede escribir respecto a la lineaActual
    char *s = shm;
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
    char *s = shm;
    int offset = 67*linea;//por alguna razón no van en bloques de 66, sino de 67 
    s += offset;
    char mensaje[66];
    char fecha[19];

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(fecha,"%d_%d_%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    sprintf(mensaje,"- Proceso: %d escribe en linea %d, ",idProceso,linea);
    strcat(mensaje,fecha);
    
    strcpy(s, mensaje);
}

void imprimirArchivo(){
    char *s;
    for (s = shm; *s != '*'; s++){
        putchar(*s);
    }
}

int main(){
    menu();


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

    while(true){
        // ¿Debe ser finalizado externamente?
    }
}

