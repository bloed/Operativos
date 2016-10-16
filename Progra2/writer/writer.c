#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#define SHMSZ 27


//Variables Globales

int main();

int main(){
	int shmid;
    key_t key;
    char *shm; //apunta al inicio de memoria compartida
	char *s;

    key = 666; //Helo

    /*
     * Locate the segment. Si existe falla.
     */
    if ((shmid = shmget(key, SHMSZ, 0666)) < 0) {
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

    /*
     * Now read what the server put in the memory.
     */
    for (s = shm; *s != NULL; s++){
        putchar(*s);
    }
    putchar('\n');

    /*
     * Finally, change the first character of the 
     * segment to '*', indicating we have read 
     * the segment.
     */
    *shm = '*'; 

    exit(0);
}

