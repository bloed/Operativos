#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

//Semaforos
char SEM_readTry[] = "readTry";
char SEM_resource[] = "resource";

int main();
int devolverMemoria();

int main(){
    devolverMemoria();
}

int devolverMemoria(){

    key_t key = 666; /* key to be passed to shmget() */ 
    int shmid; /* return value from shmget() */ 
    char *shm; //apunta al inicio de memoria compartida
    char *s;
    sem_t *semaphoreReadTry;
    sem_t *semaphoreResource;


    //Release Semaphores
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

    //Release Shared Memory

    if ((shmid = shmget(key, 10, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    /*
     * Now we attach the segment to our data space.
     */
    if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }


    //ahora procedemos a cerrar los programos, ponemos la bandera en 0
    int *p = (int *)shm;
    p = p + 2;
    *p = 0; // inicializamos bandera
    printf("Cerramos todos los threads... 5 segs más...\n");
    sleep(5);



    int rtrn;
    //Para destruirila
    if ((rtrn = shmctl(shmid, IPC_RMID, NULL)) == -1) {
        perror("shmctl: shmctl failed");
        exit(1);
    }


    sem_close(semaphoreReadTry);
    sem_unlink(SEM_readTry);

    sem_close(semaphoreResource);
    sem_unlink(SEM_resource);
    
    printf("Memoria limpiada \n");
    exit(0);
}