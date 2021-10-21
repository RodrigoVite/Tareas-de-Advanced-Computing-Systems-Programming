#include <stdio.h>
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
	int fdOpcion, fdPromedio, fdDatos, fdPares;
	char opcion[N], promedio[N], datos[N], pares[N];
	int menu, operacion;
	
	while(1){
		fdOpcion 	= abrirParametro("/sys/module/tarea_4/parameters/opcion");
		fdPromedio 	= abrirParametro("/sys/module/tarea_4/parameters/prom");
		fdDatos 	= abrirParametro("/sys/module/tarea_4/parameters/datos");
		fdPares		= abrirParametro("/sys/module/tarea_4/parameters/numPares");

		printf("\n\n\nMenu:\n\t0)Salir\n\t1)Operaciones con el arreglo\n\t2)Leer el arreglo\n\t3)Nuevos datos para el arreglo\n");
		scanf("%d",&menu);

		switch(menu){
			system("clear");
			case 0:
				exit(0);
				break;
			case 1:
				printf("\nOperaciones posibles con el arreglo:\n\t1)Promedio de los elementos\n\t2)Ordenar usando Burbuja\n\t3)Numeros pares\n");
				scanf("%d",&operacion);

				if(operacion==1){
					strcpy(opcion, "1");
					escribirParametro(fdOpcion, opcion);
					printf("El promedio es:\n");
					leerParametro(fdPromedio, promedio, N);
				}else if(operacion==2){
					strcpy(opcion, "2");
					escribirParametro(fdOpcion, opcion);
					printf("Los datos ordenados usando Burbuja son:\n");
					leerParametro(fdDatos, datos, N);
				}else if(operacion==3){
					strcpy(opcion, "3");
					escribirParametro(fdOpcion, opcion);
					printf("Los numeros pares son:\n");
					leerParametro(fdPares, pares, N);
				}

				close(fdOpcion);
				close(fdPromedio);
				close(fdDatos);
				close(fdPares);
				operacion=0;
				break;
			case 2:
				printf("Los datos del arreglo son:\n");
				leerParametro(fdDatos, datos, N);
				close(fdOpcion);
				close(fdPromedio);
				close(fdDatos);
				close(fdPares);
				break;
			case 3:
				printf("Dijite los datos, utilice comas para separarlos:\n");
				scanf("%s",datos);
				escribirParametro(fdDatos, datos);
				close(fdOpcion);
				close(fdPromedio);
				close(fdDatos);
				close(fdPares);

				break;
			default:
				printf("Ingrese una opcion valida.\n");

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