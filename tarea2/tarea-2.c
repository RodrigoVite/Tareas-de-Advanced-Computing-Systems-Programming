/*
 *	tarea-2.c - Programar un modulo de kernel que permita realizar diversas
 *	operaciones con un arreglo de caracteres.
 */

#include <linux/module.h> 		/* Necesaria para todos los modulos */
#include <linux/kernel.h> 		/* Necesaria para KERN_INFO */
#include <linux/init.h>			/* Necesaria para las macros de documentación */
#include <linux/moduleparam.h>	/* Necesaria para las macros de parametros */
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");																/* Tipo de licencia */
MODULE_AUTHOR("RODRIGO VITE CRUZ");													/* Autor del módulo */
MODULE_DESCRIPTION("MODULO DE KERNEL CON OPERACIONES A UN ARREGLO DE CARACTERES");	/* Descripción de la funcionalidad del módulo */
MODULE_VERSION("1.0");
MODULE_INFO(driver, "OPERACIONES A UN ARREGLO DE CARACTERES");						/* Información personalizada del usuario */
MODULE_INFO(interfaz, "UART");

static int opcion = 0;
module_param(opcion, int, 0660);
MODULE_PARM_DESC(opcion, "Operacion a realizar");

static char *letra = "e";
module_param(letra, charp, 0660);
MODULE_PARM_DESC(letra, "Letra a buscar en la cadena");

static char *cadena = "esta es la optativa de advanced computing systems programming";
module_param(cadena, charp, 0660);
MODULE_PARM_DESC(cadena, "Cadena");

//Solo permisos de lectura 0440
static int longitud = 0;
module_param(longitud, int, 0660);
MODULE_PARM_DESC(longitud, "Longitud de la cadena");

static int rep = 0;
module_param(rep, int, 0660);
MODULE_PARM_DESC(rep, "Repeticion de la letra en la cadena");

struct task_struct *khilo;

int Busqueda(void){
	register int i = 0;
	int temp = 0;

	while(cadena[i] != '\0'){
		if(cadena[i] == letra[0]){
			temp++;
		}
		i++;
	}

	return temp;
}

void lonCadena(void){
	register int i = 0;

	while(cadena[i] != '\0'){
		i++;
	}
	longitud = i;
}

static int hilo_kernel(void *arg){
	int id = *(int *)arg;

	printk(KERN_INFO "Iniciando la ejecución de la funcion hilo_kernel.\n");
	while(!kthread_should_stop()){ 	// Termina cuando el hilo kthread_stop es llamada
		schedule();				// bloquea el hilo por un intervalo preestablecido de tiempo.
		//ssleep(2);				// bloquea el hilo por un numero de segundos = 2

		if(opcion == 1){			//calcular el promedio de los datos
			rep = Busqueda();
			printk(KERN_INFO "'%s' se repite %d veces en '%s'.\n", letra, rep, cadena);
			opcion = 0;
		}else if(opcion == 2){		//ordenar los elementos del arreglo
			lonCadena();
			
			printk(KERN_INFO "La longitud de '%s' es: %d\n", cadena, longitud);			
			opcion = 0;
		}
	}

	return id;
}

static int __init funcion_inicio(void){

	static int id_thread = 2;

	printk(KERN_INFO "Iniciando la tarea 2 de hilos de kernel.\n");
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

	printk(KERN_INFO "Terminando la ejecucion de la tarea 2 de hilos de kernel.\n");

	if(ret_hilok == -EINTR)
		printk(KERN_ERR "Error en la terminacion el hilo de kernel...\n");
	else
		printk(KERN_INFO "Terminacion del hilo con valor: %d\n", ret_hilok);
}

module_init(funcion_inicio);
module_exit(funcion_exit);