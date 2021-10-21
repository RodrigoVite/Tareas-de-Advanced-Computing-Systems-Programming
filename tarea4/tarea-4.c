/*
 *	tarea-1.c - Programar un modulo de kernel que permita realizar diversas
 *	operaciones con un arreglo.
 */

#include <linux/module.h> 		/* Necesaria para todos los modulos */
#include <linux/kernel.h> 		/* Necesaria para KERN_INFO */
#include <linux/init.h>			/* Necesaria para las macros de documentación */
#include <linux/moduleparam.h>	/* Necesaria para las macros de parametros */
#define MAX 8

MODULE_LICENSE("GPL");													/* Tipo de licencia */
MODULE_AUTHOR("RODRIGO VITE CRUZ");										/* Autor del módulo */
MODULE_DESCRIPTION("MODULO DE KERNEL CON OPERACIONES A UN ARREGLO");	/* Descripción de la funcionalidad del módulo */
MODULE_VERSION("1.0");
MODULE_INFO(driver, "OPERACIONES A UN ARREGLO");						/* Información personalizada del usuario */
MODULE_INFO(interfaz, "UART");

static int opcion = 0;
/*module_param(opcion, int, 0660);
MODULE_PARM_DESC(opcion, "Operacion a realizar");*/

//Solo permisos de lectura: 0440
static int prom = 0;
module_param(prom, int, 0660);
MODULE_PARM_DESC(prom, "Promedio del arreglo");

static int datos[MAX] = {5,78,-23,97,12,-5,7,44};
static int numEle = MAX;
module_param_array(datos, int, &numEle, 0660);
MODULE_PARM_DESC(datos, "Muestras");

//Solo permisos de lectura: 0440
static int numPares[MAX] = {1};
static int numEleP = MAX;
module_param_array(numPares, int, &numEleP, 0660);
MODULE_PARM_DESC(numPares, "Pares");

struct task_struct *khilo;

void Burbuja(void){
	register int i, j; 
	int temp = 0;

	for(i = 0; i < numEle-1; i++){
		for(j = i+1; j < numEle; j++){
			if(datos[i] > datos[j]){
				temp = datos[i];
				datos[i] = datos[j];
				datos[j] = temp;
			}
		}
	}
}

int Promedio(void){
	register int i;
	int temp = 0;

	for(i=0; i<numEle; i++){
		temp += datos[i];
	}

	return temp/numEle;
}

int set_param(const char *val, const struct kernel_param *kp){
	
	register int i, j;
	int paramArg = *(int *)kp->arg;
	int ret;

	printk(KERN_INFO "Funcion Call Back set_param ejecutada...\n");
	printk(KERN_INFO "Parametro val: %s \n", val);
	printk(KERN_INFO "Parametro arg: %d, opcion: %d \n", paramArg, opcion);
	
	//Esta funcion convierte una cadena a un entero
	ret = param_set_int(val, kp);
	if(ret == 0){

		paramArg = *(int *)kp->arg;
		printk(KERN_INFO "Valores despues de la conversion...\n");
		printk(KERN_INFO "Parametro arg: %d, opcion: %d \n", paramArg, opcion);
		//colocamos código propio

		if(opcion == 1){			//calcular el promedio de los datos
			prom = Promedio();
			printk(KERN_INFO "Promedio del arreglo: %d\n", prom);

			//opcion = 0;
		}else if(opcion == 2){		//ordenar los elementos del arreglo
			Burbuja();
			
			printk(KERN_INFO "Ordenamiento de Burbuja:\n");
			for(i=0; i<numEle; i++){
				printk(KERN_INFO "Datos[%d] = %d \n", i, datos[i]);
			}
			
			//opcion = 0;
		}else if(opcion == 3){		//encontrar los numeros pares del arreglo
			printk(KERN_INFO "Numeros Pares del arreglo Datos:\n");
			j = 0;

			for(i=0; i<numEleP; i++){
				if((datos[i]%2) == 0){
					numPares[j] = datos[i];
					j++;
				}
			}
			if(j != 0){
				numEleP = j;
				for(i=0; i<j; i++){
					printk(KERN_INFO "numPares[%d] = %d \n", i, numPares[i]);
				}
			}else{
				for(i=0; i<numEleP; i++){
					numPares[i] = 0;
				}
			}

			//opcion = 0;
		}
	}

	return ret;
}

int get_param(char *buffer, const struct kernel_param *kp){
	
	int paramArg = *(int *)kp->arg;
	int ret;
	
	printk(KERN_INFO "Funcion Call Back get_param ejecutada...\n");
	printk(KERN_INFO "Parametro buffer: %s \n", buffer);
	printk(KERN_INFO "Parametro arg: %d, opcion: %d \n", paramArg, opcion);
	
	//Esta funcion convierte un entero a una cadena
	//Retorna la longitud de la cadena incluyendo el nulo
	ret = param_get_int(buffer, kp);
	if(ret > 0){

		printk(KERN_INFO "Valores despues de la conversion...\n");
		printk(KERN_INFO "Parametro buffer: %s \n", buffer);
		printk(KERN_INFO "Valor de retorno: %d \n", ret);

		return ret;
	}

	return -EPERM;
}

static const struct kernel_param_ops mis_param_ops = {
	
	.set = set_param,
	.get = get_param,
};

module_param_cb(opcion, &mis_param_ops, &opcion, 0660);
MODULE_PARM_DESC(opcion, "Operacion a realizar");

static int __init funcion_inicio(void){

	printk(KERN_INFO "Iniciando la tarea 4, funciones call back.\n");

	/*
	 * Un valor de retorno diferente de 0 significa que 
	 * init_module fallo; el modulo no se puede cargar
	 */
	return 0;
}

static void __exit funcion_exit(void){

	printk(KERN_INFO "Terminando la ejecucion de la tarea 4, funciones call back.\n");
}

module_init(funcion_inicio);
module_exit(funcion_exit);