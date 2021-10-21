#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#define N 1024

int abrirParametro(char* parametroSysfs);
void leerParametro(int fd, char* parametro, int Nbytes);
void escribirParametro(int fd, char* parametro);

int main(int argc, char const *argv[]){
	int opt=0;
	int fdBoleta;
	char boleta[N];

	while(1){
		fdBoleta 	= abrirParametro("/sys/module/boleta2/parameters/boleta");

		system("clear");
		
		printf("Selecciona una opcion\n1. Cambiar valor de boleta\n2. Leer valor de boleta\n3. Salir\n\nOpcion:\t\t");
		scanf("%d",&opt);		
		switch(opt){
			case 1:
				system("clear");

				printf("Ingresa la nueva boleta:\n");
				__fpurge(stdin);
				fgets(boleta,N,stdin);
				printf("Boleta: %s",boleta);
				
				escribirParametro(fdBoleta,boleta);
				close(fdBoleta);
				
				break;
			case 2:
				system("clear");

				printf("\nEl valor de la boleta es:\n");
				leerParametro(fdBoleta, boleta, N);
				close(fdBoleta);

				while(getchar()!='\n');
				getchar();

				break;
			case 3:
				exit(0);
				break;
		}
	}
	
	return 0;
}

int abrirParametro(char* parametroSysfs){
	int fd;

	fd = open(parametroSysfs, O_RDWR);
	if(fd == -1){
		perror("Error al abrir el parametro\n");
		exit(EXIT_FAILURE);
	}
	//printf("El descriptor del parametro es: %d \n", fd);

	return fd;
}

void leerParametro(int fd, char* parametro, int Nbytes){
	int nbytes;

	nbytes = read(fd, parametro, Nbytes);
	if(nbytes == -1){
		perror("Error al leer el parametro\n");
		exit(EXIT_FAILURE);
	}
	parametro[nbytes-1] = 0; //Necesario para no imprimir basura
	//printf("Bytes recibidos del parametro: %d \n", nbytes);
	//printf("El valor del parametro es: %s \n", parametro);
	printf(" %s \n", parametro);
}

void escribirParametro(int fd, char* parametro){
	int nbytes;

	nbytes = write(fd, parametro, strlen(parametro)+1); //+1 para que también se envie el null de la cadena
	if(nbytes == -1){
		perror("Error al escribir en el parametro\n");
		exit(EXIT_FAILURE);
	}
	//printf("Bytes enviados al parametro: %d \n", nbytes);
}