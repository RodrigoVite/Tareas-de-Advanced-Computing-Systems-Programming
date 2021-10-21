/*
 *	tarea-1.c - Programar un modulo de kernel que permita realizar diversas
 *	operaciones con un arreglo.
 */

#include <linux/module.h> 		/* Necesaria para todos los modulos */
#include <linux/kernel.h> 		/* Necesaria para KERN_INFO */
#include <linux/init.h>			/* Necesaria para las macros de documentación */
#include <linux/moduleparam.h>	/* Necesaria para las macros de parametros */
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>
#define MAX 8

MODULE_LICENSE("GPL");													/* Tipo de licencia */
MODULE_AUTHOR("RODRIGO VITE CRUZ");										/* Autor del módulo */
MODULE_DESCRIPTION("MODULO DE KERNEL CON OPERACIONES A UN ARREGLO");	/* Descripción de la funcionalidad del módulo */
MODULE_VERSION("1.0");
MODULE_INFO(driver, "OPERACIONES A UN ARREGLO");						/* Información personalizada del usuario */
MODULE_INFO(interfaz, "UART");

static int opcion = 0;
module_param(opcion, int, 0660);
MODULE_PARM_DESC(opcion, "Operacion a realizar");

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

static int hilo_kernel(void *arg){
	int id = *(int *)arg;
	register int i, j;

	printk(KERN_INFO "Iniciando la ejecución de la funcion hilo_kernel.\n");
	while(!kthread_should_stop()){ // Termina cuando el hilo kthread_stop es llamada
		schedule();	// bloquea el hilo por un intervalo preestablecido de tiempo.
		//ssleep(2);		// bloquea el hilo por un numero de segundos = 2

		if(opcion == 1){			//calcular el promedio de los datos
			prom = Promedio();
			printk(KERN_INFO "Promedio del arreglo: %d\n", prom);
			opcion = 0;
		}else if(opcion == 2){		//ordenar los elementos del arreglo
			Burbuja();
			
			printk(KERN_INFO "Ordenamiento de Burbuja:\n");
			for(i=0; i<numEle; i++){
				printk(KERN_INFO "Datos[%d] = %d \n", i, datos[i]);
			}
			
			opcion = 0;
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
			opcion = 0;
		}
	}

	return id;
}

static int __init funcion_inicio(void){

	static int id_thread = 1;

	printk(KERN_INFO "Iniciando la tarea 1 de hilos de kernel.\n");
	khilo = kthread_run(hilo_kernel, (void *)&id_thread, "kmi_hilo");

	if(IS_ERR(khilo)){ 				//Macro para verificar si es negativo
		printk(KERN_ERR "Error al crear el hilo de kernel...\n");
		return PTR_ERR(khilo);		//Macro para poder imprimir correctamente (convierte de pointer a error)
	}
	printk(KERN_INFO "Hilo creado con pid: %d y nombre: %s\n", khilo->pid, khilo->comm);

	/*
	 * Un valor de retorno diferente de 0 significa que 
	 * init_module fallo; el modulo no se puede cargar
	 */
	return 0;
}

static void __exit funcion_exit(void){

	//register int i;
	int ret_hilok = kthread_stop(khilo);

	printk(KERN_INFO "Terminando la ejecucion de la tarea 1 de hilos de kernel.\n");

	if(ret_hilok == -EINTR)
		printk(KERN_ERR "Error en la terminacion el hilo de kernel...\n");
	else
		printk(KERN_INFO "Terminacion del hilo con valor: %d\n", ret_hilok);
}

module_init(funcion_inicio);
module_exit(funcion_exit);