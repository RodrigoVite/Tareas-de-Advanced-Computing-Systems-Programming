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
	int fdOpcion, fdLetra, fdCadena, fdLongitud, fdRep;
	char opcion[N], letra[N], cadena[N], longitud[N], rep[N];
	int menu = 0;
	int operacion;
	
	do{
		fdOpcion 	= abrirParametro("/sys/module/tarea_2/parameters/opcion");
		fdLetra 	= abrirParametro("/sys/module/tarea_2/parameters/letra");
		fdCadena 	= abrirParametro("/sys/module/tarea_2/parameters/cadena");
		fdLongitud	= abrirParametro("/sys/module/tarea_2/parameters/longitud");
		fdRep		= abrirParametro("/sys/module/tarea_2/parameters/rep");

		printf("\n\n\nMenu:\n\t0)Salir\n\t1)Operaciones con la cadena\n\t2)Leer la cadena\n\t3)Nueva cadena\n\t4)Limpiar pantalla\n");
		scanf("%d",&menu);
		switch(menu){
			case 1:
				printf("Operaciones posibles con la cadena:\n\t1)Contar repetición de un caracter\n\t2)Longitud\n");
				scanf("%d",&operacion);

				if(operacion==1){
					leerParametro(fdCadena, cadena, N);
					printf("Ingrese el caracter a buscar:");
					scanf("%s",letra);
					escribirParametro(fdLetra, letra);
					strcpy(opcion, "1");
					escribirParametro(fdOpcion, opcion);
					leerParametro(fdRep, rep, N);
				}else if(operacion==2){
					strcpy(opcion, "2");
					escribirParametro(fdOpcion, opcion);
					leerParametro(fdLongitud, longitud, N);
				}

				close(fdOpcion);
				close(fdLetra);
				close(fdCadena);
				close(fdLongitud);
				operacion=0;
				break;
			case 2:
				leerParametro(fdCadena, cadena, N);
				close(fdOpcion);
				close(fdLetra);
				close(fdCadena);
				close(fdLongitud);
				break;
			case 3:
				printf("Ingrese la nueva cadena:\n");
				//scanf("%s",cadena);
				__fpurge(stdin);
				fgets(cadena,N,stdin);
				escribirParametro(fdCadena, cadena);
				close(fdOpcion);
				close(fdLetra);
				close(fdCadena);
				close(fdLongitud);
				break;
			case 4:
				system("clear");
				break;
		}
	}while(menu!=0);

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
	printf("El valor del parametro es: %s \n", parametro);
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