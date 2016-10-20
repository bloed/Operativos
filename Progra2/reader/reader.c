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
int variablesThreads[100][3];//se guarda pid  | y linea actual y estado 
// 0 : acceso
// 1 : dormido
// 2 : bloqueado
int cantidadThreadsRestantes = 0;

int main();
void menu();
int archivoLleno();//devuelve si el archivo esta lleno actualmente
int getBandera();
int getContador();
void setContador();
void setLecturaSeguida();
int siguienteLinea(int lineaActual);// devuelve la siguiente linea vacía
void crearThreads();
void *accionThread(void *pointer);
void imprimirArchivo();
void leerArchivo(int lineaActual);


void menu(){
    printf("Ingresa la cantidad de lectores:\n");
    scanf("%s", scantidadEscritores);
    printf("Ingresa el tiempo que duraran leyendo:\n");
    scanf("%s", stiempoLeyendo);
    printf("Ingresa el tiempo que duraran dormidos\n");
    scanf("%s", stiempoDormido);
    cantidadThreads = atoi(scantidadEscritores);
    tiempoDormido = atoi(stiempoDormido);
    tiempoLeyendo = atoi(stiempoLeyendo);
}

int archivoVacio(){
    char *s;
    for (s = shm + 3612; *s != '*'; s++){
        if (*s == '-'){// si se encuentra un más, es que al menos hay una línea vacía
            return false;
        }
    }
    return true;
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

void setLecturaSeguida(){
    int *p = (int *)shm;
    p++;//nos movemos hacia arriba
    *p = 0;
}

void setContador(){
    //aumenta en 1 el contador
    int *p = (int *)shm;
    *p = *p + 1;
}

void *accionThread(void *pointer){
    //HACE FALTA PONER ESTADO BLOQUEADO/LEYENDO/ETC.
    int idThread = (intptr_t) pointer;//id local para el array de 100
    int pID = variablesThreads[idThread][0];// id verdadero del programa
    int lineaActual = variablesThreads[idThread][1];
    while(getBandera() == 1){
        if(archivoVacio()){
            printf("Vacío \n");
            sleep(tiempoDormido); //si esta lleno se duerme? Y si debería
        }else{
            lineaActual = siguienteLinea(lineaActual);
            leerArchivo(lineaActual);
            lineaActual++;
            variablesThreads[idThread][1] = lineaActual; //para guardar la info
            setLecturaSeguida();
            sleep(tiempoLeyendo + tiempoDormido);
        }
    }

    printf("Finaliza Thread %d\n", idThread);
    cantidadThreadsRestantes--;
    return NULL; //finaliza thread
}

void crearThreads(){
    for(int i = 0; i < cantidadThreads ; i++){
        variablesThreads[contadorThreads][0] = getContador();;
        variablesThreads[contadorThreads][1] = 0;//todos inician en la línea 0
        pthread_create(&threads[contadorThreads], NULL, &accionThread, (void *) (intptr_t) contadorThreads);
        setContador();//aumentamos en uno
        contadorThreads++;
        cantidadThreadsRestantes++;
    }
}

int siguienteLinea(int lineaActual){
    //devuelve la siguiente linea donde se puede escribir respecto a la lineaActual
    //falta el caso en que si llega al final, que comience otra vez
    char *s = shm + 3612;
    int lineaReal = lineaActual;
    int offset = 66*lineaActual;
    s += offset;
    //printf("Char %c \n",s[0]);
    while (true){
        if(*s == '-'){ //este método devuelve línea 0, pero abajo se va hacia uno.
            return lineaReal;
        }
        if(*s == '+'){
            s = s + 66;
            lineaReal++;
        }
        if(*s == '*'){
            s = shm + 3612;
            lineaReal = 0;
        }
        printf("Vuelve al inicio \n");
    }
    return 0; //nunca deberia ejecutarse
}

void imprimirArchivo(){
    char *s;
    for (s = shm + 3612; *s != '*'; s++){
        putchar(*s);
    }
}

void leerArchivo(int lineaActual){
    char *s = shm + 3612;
    int offset = 66*lineaActual;
    for(s += offset; *s!='\n';s++){
        putchar(*s);
    }
    putchar('\n');
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
    while(cantidadThreadsRestantes != 0){
        int caca;
    }
    printf("Finaliza ejecución de los readers\n");
}

