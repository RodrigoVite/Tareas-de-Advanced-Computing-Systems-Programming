/** @brief: Este programa muestra solicita las direcciones virtuales
 *  de el GPIO de la raspberry, se usan la función kthread_create para
 *  crear un hilo y la función wake_up_process para iniciar la ejecución
 *  del hilo
 */

#include <linux/module.h> 		/* Necesaria para todos los modulos */
#include <linux/kernel.h> 		/* Necesaria para KERN_INFO */
#include <linux/init.h>			/* Necesaria para las macros de documentación */
#include <linux/moduleparam.h>	/* Necesaria para las macros de parametros */
#include<linux/delay.h>
#include<asm/io.h>
#define N 11

//#define ADDR_BASE_GPIO 0x3f200000		/* Direccion base para Raspberry Pi3 B+ */
#define ADDR_BASE_GPIO 0xfe200000		/* Direccion base para Raspberry Pi4 B */

MODULE_LICENSE("GPL");											/* Tipo de licencia */
MODULE_AUTHOR("EQUIPO ALEX-DAVID-RODRIGO");						/* Autor del módulo */
MODULE_DESCRIPTION("MODULO DE KERNEL PARA MANEJO DE DISPLAY");	/* Descripción de la funcionalidad del módulo */
MODULE_VERSION("1.0");
MODULE_INFO(driver, "DESPLEGAR BOLETA");						/* Información personalizada del usuario */

static char boleta[N];
/*module_param(boleta, charp, 0660);
MODULE_PARM_DESC(boleta, "Boleta");*/

struct task_struct *khilo;

volatile uint32_t *gpio_virtual;

char * codificacion[11]= {"0111111","0000110","1011011","1001111","1100110","1101101","1111101","0000111","1111111","1101111","0000000"};

void valueGPIO( volatile uint32_t *gpio_virt, int gpio, int valor )
{
	int bit = (gpio >> 5)&1,
		GPSET_CLR;
	int bitRango = gpio % 32;

	if( valor == 0 )
		GPSET_CLR = bit + 10;
	else if( valor == 1 )
		GPSET_CLR = bit + 7;
    
	*(gpio_virt + GPSET_CLR) =  (1 << bitRango);
}

void configGPIO( volatile uint32_t * gpio_virt, int gpio, int modo )
{
	int GPFSEL = gpio / 10;
	int bitRango = (gpio % 10);

	bitRango = (bitRango << 1) + bitRango; // bitRango = bitRango * 3

	if( modo == 0 )
        	*(gpio_virt + GPFSEL) = (*(gpio_virt + GPFSEL) & ~(7 << bitRango)) | (1 << bitRango);
	else if( modo == 1 )
        	*(gpio_virt + GPFSEL) = (*(gpio_virt + GPFSEL) & ~(7 << bitRango));
}

void imprimirDigito(char numero){
	char * led=codificacion[(int)numero-48];
	
	valueGPIO(gpio_virtual,10,(int)led[0]-48);
	valueGPIO(gpio_virtual,24,(int)led[1]-48);
	valueGPIO(gpio_virtual,23,(int)led[2]-48);
	valueGPIO(gpio_virtual,22,(int)led[3]-48);
	valueGPIO(gpio_virtual,27,(int)led[4]-48);
	valueGPIO(gpio_virtual,18,(int)led[5]-48);
	valueGPIO(gpio_virtual,17,(int)led[6]-48);
}

void limpiar(void){
	char * led=codificacion[10];
	valueGPIO(gpio_virtual,10,(int)led[0]-48);
	valueGPIO(gpio_virtual,24,(int)led[1]-48);
	valueGPIO(gpio_virtual,23,(int)led[2]-48);
	valueGPIO(gpio_virtual,22,(int)led[3]-48);
	valueGPIO(gpio_virtual,27,(int)led[4]-48);
	valueGPIO(gpio_virtual,18,(int)led[5]-48);
	valueGPIO(gpio_virtual,17,(int)led[6]-48);
}

int set_param(const char *val, const struct kernel_param *kp){
	
	register int i;
	char *cadena = (char *)kp->arg;
	int longitud = strlen(val);

	printk(KERN_INFO "Funcion Call Back set_param ejecutada...\n");
	printk(KERN_INFO "Parametro val: %s \n", val);
	printk(KERN_INFO "Parametro arg: %s, boleta: %s \n", cadena, boleta);
	
	if(longitud > N){
		
		printk(KERN_ERR "Error la cadena: %s es muy larga\n", val);
		return -ENOSPC;
	}else if(longitud == 1){
		
		printk(KERN_ERR "Error parametro vacio\n");
		return -EINVAL;
	}

	strcpy(kp->arg, val);

	printk(KERN_INFO "Parametro boleta despues de la copia: %s\n", boleta);
	//colocamos código propio

	for(i=0; i<10; i++){
		imprimirDigito(boleta[i]);
		printk( KERN_INFO "boleta[i]:%c\n", boleta[i]);
		ssleep(1);
	}

	limpiar();

	return 0;
}

int get_param(char *buffer, const struct kernel_param *kp){
	
	char *cadena = (char *)kp->arg;
	int longitud;
	
	printk(KERN_INFO "Funcion Call Back get_param ejecutada...\n");
	printk(KERN_INFO "Parametro buffer: %s \n", buffer);
	printk(KERN_INFO "Parametro arg: %s, boleta: %s \n", cadena, boleta);
	
	strcpy(buffer, cadena);
	longitud = strlen(buffer);

	printk(KERN_INFO "Parametro buffer despues de la copia: %s\n", buffer);

	return longitud;
}

static const struct kernel_param_ops mis_param_ops = {
	
	.set = set_param,
	.get = get_param,
};

module_param_cb(boleta, &mis_param_ops, &boleta, 0660);
MODULE_PARM_DESC(boleta, "Boleta");

/*
 * La funcion kthread_run crea y despierta a un hilo, lo pone en ejecucion.
 * Ejecuta una llamada a kthread_create() seguida de una llamada a wake_up_process()
 * Valor de retorno:
 * 		Regresa el hilo creado ó
 * 		ERR_PTR(-ENOMEM )
 */
static int __init kernel_init(void)
{
	printk( KERN_INFO "\n--------------------------------------------\n" );
	printk(KERN_INFO "Iniciando la tarea 5, Manejo de display con funciones call back.\n");
   	
	//RANGO DE DIRECCIONES DE LOS REGISTROS A PARTIR DE LA DIRECCION BASE
	gpio_virtual = (uint32_t *)ioremap( ADDR_BASE_GPIO, 0X30 );
	configGPIO(gpio_virtual,10,0);
	configGPIO(gpio_virtual,17,0);
	configGPIO(gpio_virtual,18,0);
	configGPIO(gpio_virtual,22,0);
	configGPIO(gpio_virtual,23,0);
	configGPIO(gpio_virtual,24,0);
	configGPIO(gpio_virtual,27,0);
	
	//*(gpio_virtual + 1) = (*(gpio_virtual + 1) & ~(7 << 18)) | (1 << 18);

   	return 0;
}

/*
 * La funcion kthread_stop realiza lo siguiente:
 * 1. Activa kthread->kthread_should_stop = true
 * 2. Despierta al hilo
 * 3. Espera a que el hilo termine
 * 4. Valor de retorno:
 * 		Regresa el resultado de hilo_kernel ó
 * 		-EINTR si no se llama a la funcion wake_up_process()
 */
static void __exit kernel_exit(void)
{
	printk(KERN_INFO "Terminando la ejecucion de la tarea 5, Manejo de display con funciones call back.\n");

	iounmap( gpio_virtual );
}

module_init(kernel_init);
module_exit(kernel_exit);