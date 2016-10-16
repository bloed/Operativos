#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

//Variables Globales
char numeroLineas[3];
int cantidadBytes;
int bytesLinea = 66;//cada línea será de 66 bytes.


int main();
int pedirMemoria();

int main(){
	printf("Ingresa la cantidad de líneas que tendrá la memoria compartida:\n");
	scanf("%s", numeroLineas);
    cantidadBytes = atoi(numeroLineas) * bytesLinea;
	pedirMemoria(cantidadBytes + 1);//el último se usará para el final del achivo
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

    for (int c = 0; c < atoi(numeroLineas); c++){
        strcpy( s, "+ Linea vacía \n");
        s += 66;
    }
    *s = '*';//marcar el final del archivo

    exit(0);
}