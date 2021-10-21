/** @brief: Este programa muestra solicita las direcciones virtuales
 *  de el GPIO de la raspberry, se usan la función kthread_create para
 *  crear un hilo y la función wake_up_process para iniciar la ejecución
 *  del hilo
 */

#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/kthread.h>
#include<linux/sched.h>
#include<linux/delay.h>
#include<asm/io.h>

//#define ADDR_BASE_GPIO 0x3f200000		/* Direccion base para Raspberry Pi3 B+ */
#define ADDR_BASE_GPIO 0xfe200000		/* Direccion base para Raspberry Pi4 B */

MODULE_LICENSE("GPL");											/* Tipo de licencia */
MODULE_AUTHOR("EQUIPO ALEX-DAVID-RODRIGO");						/* Autor del módulo */
MODULE_DESCRIPTION("MODULO DE KERNEL PARA MANEJO DE DISPLAY");	/* Descripción de la funcionalidad del módulo */
MODULE_VERSION("1.0");
MODULE_INFO(driver, "DESPLEGAR BOLETA");						/* Información personalizada del usuario */

static int cont = -1;
module_param(cont, int, 0660);
MODULE_PARM_DESC(cont, "Contador");

static char *boleta = "0";
module_param(boleta, charp, 0660);
MODULE_PARM_DESC(boleta, "Boleta");

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

/*
 * La funcion que es ejecutada por el hilo del kernel
 * Valor de retorno:
 * Regresa id
 */
int hilo_kernel(void *data)
{
	int id = *(int *)data;

	printk(KERN_INFO "Hilo en ejecucion con id: %d...\n", id);
    while( !kthread_should_stop() )	//Termina el hilo cuando kthread_stop es llamada
	{
		ssleep(1);
		if(cont>=0){
			imprimirDigito(boleta[cont]);
			printk( KERN_INFO "boleta[cont]:%c\n", boleta[cont]);
			cont++;
			ssleep(1);
			if(cont==10){
				cont=-1;
				limpiar();
			}
		}
    }
	return id;
}

/*
 * La funcion kthread_run crea y despierta a un hilo, lo pone en ejecucion.
 * Ejecuta una llamada a kthread_create() seguida de una llamada a wake_up_process()
 * Valor de retorno:
 * 		Regresa el hilo creado ó
 * 		ERR_PTR(-ENOMEM )
 */
static int __init kernel_init(void)
{
	static int id_thread = 10;

	printk( KERN_INFO "\n--------------------------------------------\n" );
   	khilo = kthread_create( hilo_kernel, (void *)&id_thread, "kmi_hilo" );

	if( IS_ERR(khilo) )
	{
		printk("KERN_ERR Error en la creacion del hilo... \n");
		return PTR_ERR(khilo);
	}
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

	wake_up_process( khilo );
   	printk( KERN_INFO "Hilo de Kernel creado: %s con PID %d \n", khilo->comm, khilo->pid );

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
	int ret_hilo_kernel = kthread_stop( khilo );

	if( ret_hilo_kernel == -EINTR )
		printk( KERN_INFO "Error en la terminación del hilo \n" );
	else
		printk( KERN_INFO "Hilo de Kernel con id: %d detenido\n", ret_hilo_kernel );

	iounmap( gpio_virtual );
}

module_init(kernel_init);
module_exit(kernel_exit);