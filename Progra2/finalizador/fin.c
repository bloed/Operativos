#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

//Variables Globales


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

    int rtrn;
    //Para destruirila
    if ((rtrn = shmctl(shmid, IPC_RMID, NULL)) == -1) {
        perror("shmctl: shmctl failed");
        exit(1);
    }

    exit(0);
}