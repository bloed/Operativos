#include <stdio.h>

//Variables Globales:
char tipoCliente[1];
main(){
	menu();
	printf("%s\n", tipoCliente);
} 

menu(){
	printf("Selecciona el tipo de cliente");
    printf("1. Manual\n");
    printf("2. Automatico\n");
    scanf("%s", tipoCliente);
}