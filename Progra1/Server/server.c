/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h> //Data types for system calls
#include <sys/socket.h> //Lib for sockets
#include <netinet/in.h> //internet domain addresses, not sutre if needed

void error(const char *msg) //called when system call fails.
{
    perror(msg);
    exit(0);
}

int main()
{
    int socketMainConnection; //Value returned by socket system call
    int newSocketCon; // Value returned by accept system call
    int portNumber = 1234; //Port number for connections
    int addressClientLen; //Size of the client's address

    struct sockaddr_in serverAdress;
    struct sockaddr_in clientAddress; //structure with internet address protocol sockaddr_in
    

    //Socket creation, INET protocol, Type of message, 0 to let CPU choose protocol
    socketMainConnection = socket(AF_INET, SOCK_STREAM, 0);
    
    if (socketMainConnection < 0) // catch in case socket creation fails.
        error("ERROR opening socket");
    
    //bzero: turns the array (struct in this case) serverAdress into zeros
    bzero((char *) &serverAdress, sizeof(serverAdress));

    //asignacion de valores al struct
    serverAdress.sin_family = AF_INET;
    serverAdress.sin_addr.s_addr = INADDR_ANY; //contains de IP, INADDR_ANY gets the address
    serverAdress.sin_port = htons(portNumber); //portnumber, needs to be converted into network byte order.

    //bind of the socket to the address, if the port is used it fails enters the if
    if (bind(socketMainConnection, (struct sockaddr *) &serverAdress,
              sizeof(serverAdress)) < 0) 
              error("ERROR on binding");
    
    listen(socketMainConnection,5); //starts listening, 5 max connections waiting, cant increase over 5
    
    addressClientLen = sizeof(clientAddress);
    
    int stop = 0;

    while(stop == 0){

        newSocketCon = accept(socketMainConnection, 
                 (struct sockaddr *) &clientAddress, 
                 &addressClientLen); //waits for a connection

    // Segun la docu el proceso se detiene/traba hasta que llegue una conexion
    // al server, hay que probar como se comportaria esta parte dentro de thread
    //en caso de que trabe todo el proceso o solo el hilo

    if (newSocketCon < 0) //error on accpeted, it executes after a connection happens 
          error("ERROR on accept");
    
    socketAction(newSocketCon);
    }

    //Yo se que nunca llega aca... se me ocurre tener un socket final en el cliente
    //que tenga un formato especial para cambiar la variable paro.
    //sino... hay que esperar 1 min entre cada corrida para que Linux libere el puerto
    close(socketMainConnection);

    return 0;
    
}

void socketAction(int newSocketCon){
    char buffer[256]; //Received Message size
    int n; //return value of read or write.

    bzero(buffer,256);// turns zero the array
    
    //all socket operations uses the new socketMainConnection

    n = read(newSocketCon,buffer,255);//reads message

    if (n < 0) 
        error("ERROR reading from socket");

    printf("Here is the message: %s\n",buffer);
    
    n = write(newSocketCon,"Process stored",14); //18 = size of the mesage

    if (n < 0) 
        error("ERROR writing to socket");
    
    close(newSocketCon);
}
