#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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

int main();
void menu();
int archivoLleno();//devuelve si el archivo esta lleno actualmente
int siguienteLinea(int lineaActual);// devuelve la siguiente linea vacía
void crearThreads();
void *accionThread(void *pointer);
void imprimirArchivo();


void menu(){
    printf("Ingresa la cantidad de escritores:\n");
    scanf("%s", scantidadEscritores);
    printf("Ingresa el tiempo duraran leyendo:\n");
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
        lineaActual = siguienteLinea(lineaActual);
        printf("La siguiente linea es: %d\n", lineaActual);
        sleep(3);
    }



    printf("Finaliza Thread %d\n", idThread);
    return NULL; //finaliza thread
}

void crearThreads(){
    for(int i = 0; i < cantidadThreads ; i++){
        variablesThreads[contadorThreads][0] = contadorThreads;
        variablesThreads[contadorThreads][1] = 0;//todos inician en la línea 0
        pthread_create(&threads[contadorThreads], NULL, &accionThread, (void *) (intptr_t) contadorThreads);
        contadorThreads++;
    }
}

int siguienteLinea(int lineaActual){
    //devuelve la siguiente linea donde se puede escribir respecto a la lineaActual
    //falta el caso en que si llega al final, que comience otra vez
    char *s = shm;
    int lineaReal = lineaActual;
    int offset = 67*lineaActual;//por alguna razón no van en bloques de 66, sino de 67 
    int contador = 0;
    s += offset;
    for (s; *s != '*'; s++){
        if(contador % 67 == 0 || contador == 0){
            //falta ver si hay un +
        }
        contador++;
    }
    return lineaReal;
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

