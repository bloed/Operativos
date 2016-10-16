#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHMSZ 277

//Variables Globales
char numeroLineas[3];
int bytesLinea = 12;//cada línea será de 12 bytes.


int main();
int pedirMemoria();

int main(){
	printf("Ingresa la cantidad de líneas que tendrá la memoria compartida:\n");
	scanf("%s", numeroLineas);
	pedirMemoria(atoi(numeroLineas) * bytesLinea);
}

int pedirMemoria(int bytes){

	key_t key = 666; /* key to be passed to shmget() */ 
	int shmid; /* return value from shmget() */ 
	size_t size = bytes; /* size to be passed to shmget() */ 
	char *shm; //apunta al inicio de memoria compartida
	char *s;

	if ((shmid = shmget(key, size, IPC_CREAT | 0666)) < 0) {
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

    /*
     * Now put some things into the memory for the
     * other process to read.
     */
    s = shm;

    for (char c = 'a'; c <= 'z'; c++){
        *s++ = c;
    }
    *s = NULL;//me parece que no es necesario

    /*
     * Finally, we wait until the other process 
     * changes the first character of our memory
     * to '*', indicating that it has read what 
     * we put there.
     */
    while (*shm != '*'){
        sleep(1);
    }

    int rtrn;
    //Para destruirila
    if ((rtrn = shmctl(shmid, IPC_RMID, NULL)) == -1) {
	    perror("shmctl: shmctl failed");
	    exit(1);
    }

    exit(0);
}