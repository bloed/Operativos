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
int opcion;
char opcionBuffer[2];

char bufferAcceso[100];
char bufferDormidos[100];
char bufferBloqueados[200];
char miniBufferId[10];

int shmid;
key_t key = 666; //Helo
char *shm; //apunta al inicio de memoria compartida

sem_t *semaphore;
char SEM_GEN[] = "gen";


int main();
void menu();
int getBandera();
void getEstado(int offset);
void imprimirArchivo();



void menu(){
    printf("-----------------------------------------------\n");
    printf("0. Ver archivo:\n");
    printf("1. Ver estado de los writers:\n");
    printf("2. Ver estado de los readers:\n");
    printf("3. Ver estado de los readers egoístas:\n");
    printf("4. Salir\n");
    scanf("%s", opcionBuffer);
    opcion = atoi(opcionBuffer);
    printf("-----------------------------------------------\n");
}

int getBandera(){
    int *p = (int *)shm;
    p = p + 2;
    return *p;
}

void getEstado(int offset){//devuelve estado ya sea de readers, writers, etc.
    int *p = (int *)shm;
    p = p + offset;
    int total = *p;
    p++;
    int id = 0;
    int estado = 0;
    //id - estado
    sprintf(bufferAcceso, "Acceso: ");
    sprintf(bufferDormidos, "Dormidos: ");
    sprintf(bufferBloqueados, "Bloqueados: ");
    sprintf(miniBufferId, " ");
    for(int i = 0 ; i < total; i++){
        id = *p;
        p++;
        estado = *p;
        p++;
        sprintf(miniBufferId, "- %d ",id);
        if(estado == 0){//acceso
            strcat(bufferAcceso, miniBufferId);
        }
        if(estado == 1){//dormido
            strcat(bufferDormidos, miniBufferId);
        }
        if(estado == 2){//bloqueado
            strcat(bufferBloqueados, miniBufferId);
        }
    }
    printf("%s\n", bufferAcceso);
    printf("%s\n", bufferDormidos);
    printf("%s\n", bufferBloqueados);
}




void imprimirArchivo(){
    char *s;
    for (s = shm + 3612; *s != '*'; s++){
        putchar(*s);
    }
}


int main(){

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
    

    while(getBandera() == 1){
        menu();
        if (opcion == 0){
            imprimirArchivo();
        }
        if (opcion == 1){//estado de los writers
            getEstado(3);
        }
        if (opcion == 2){//estado de los readers
            //getEstado(303);
        }
        if (opcion == 3){//estado de los readerEgoistas
            getEstado(603);
        }
        if (opcion == 4){
            break;
        }
    }
    printf("Termina ejecución del espía\n");
}

