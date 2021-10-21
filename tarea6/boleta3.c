/*
 *  boleta3.c - Asignacion de numero mayor y numero menor
 *	(se usa asignación dinámica).
 * 	Se realiza la creación de clase y archivo de dispositivo
 *	(se usa creación automática).
 * 	Se colocan las operaciones de archivo.
 *	Se coloca la definición de las operaciones de archivo
 * 	Para un device driver de caracter.
 */
#include <linux/module.h>			/* Necesaria para todos los modulos 		*/
#include <linux/kernel.h>			/* Necesaria para KERN_INFO 			*/
#include <linux/init.h>				/* Necesaria para las macros			*/
#include <linux/kdev_t.h>			/* Necesaria para las macros MAJOR, MINOR	*/
#include <linux/fs.h>				/* Necesaria para las funciones de registro	*/
#include <linux/device.h>			/* Necesaria para el dispositivo		*/
#include <linux/cdev.h>				/* Necesaria para cdev				*/
#include <linux/slab.h>				/* Necesaria para kmalloc y kfree		*/
#include<linux/delay.h>
#include<asm/io.h>
#define MEM_SIZE 1024
#define ADDR_BASE_GPIO 0xfe200000	/* Direccion base para Raspberry Pi4 B */


MODULE_LICENSE("GPL");													/* Tipo de licencia */
MODULE_AUTHOR("EQUIPO ALEX-DAVID-RODRIGO");								/* Autor del módulo */
MODULE_DESCRIPTION("DEVICE DRIVER DE CARACTER PARA MANEJO DE DISPLAY");	/* Descripción de la funcionalidad del módulo */
MODULE_VERSION("3.0");
MODULE_INFO(driver, "DESPLEGAR BOLETA");								/* Información personalizada del usuario */


static dev_t dispositivo;
static struct class * dev_class;
static struct device * dev_file;
static struct cdev dev_cdev;
static char * boleta = "";

volatile uint32_t *gpio_virtual;

char * codificacion[11]= {"0111111","0000110","1011011","1001111","1100110","1101101","1111101","0000111","1111111","1101111","0000000"};


static int driver_open	   ( struct inode *inode, struct file * file );
static int driver_release  ( struct inode *inode, struct file * file );
static ssize_t driver_read ( struct file *filp, char __user *buf, size_t len, loff_t * off );
static ssize_t driver_write( struct file *filp, const char *buf, size_t len, loff_t * off );



static struct file_operations fops =
{
	.owner 	 = THIS_MODULE,
	.read 	 = driver_read,
	.write 	 = driver_write,
	.open 	 = driver_open,
	.release = driver_release,
};

static int driver_open	   ( struct inode *inode, struct file * file )
{
	printk(KERN_INFO "Ejecutando la Operacion Open \n");

	boleta = (char *)kmalloc( MEM_SIZE, GFP_KERNEL );
	if( boleta == NULL )
	{
		printk(KERN_ERR "Error al asignar memoria \n");
		return -ENOMEM;
	}

	return 0;
}

static int driver_release  ( struct inode *inode, struct file * file )
{
	printk(KERN_INFO "Ejecutando la Operacion release \n");
	kfree( boleta );

	return 0;
}

static ssize_t driver_read ( struct file *filp, char __user *buf, size_t len, loff_t * off )
{
	int lon, ret;

	printk(KERN_INFO "Ejecutando la Operacion read \n");
	printk(KERN_INFO "len: %ld, offset: %lld \n", len, *off);

	if( *off == 0 && len > 0 )
	{
		strcpy( boleta, boleta );
//		lon = strlen( boleta ) + 1;
		lon = strlen( boleta );

		ret = copy_to_user( buf, boleta, lon );
		if( ret )
		{
			printk(KERN_ERR "Error al copiar la boleta a espacio de usuario \n");
			return -EFAULT;
		}
		(*off) += lon;

		return lon;
	}
	
	return 0;
}

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

static ssize_t driver_write( struct file *filp, const char *buf, size_t len, loff_t * off )
{
	int ret;
	register int i;

	printk(KERN_INFO "Ejecutando la Operacion write \n");
	printk(KERN_INFO "len: %ld, offset: %lld \n", len, *off);

	ret = copy_from_user( boleta, buf, len );
	if( ret )
	{
		printk(KERN_ERR "Error al copiar desde espacio de usuario \n");
		return -EFAULT;
	}
	
	printk(KERN_INFO "Boleta recibido: %s \n", boleta);
	
	for(i=0; i<strlen(boleta)-1; i++){
		imprimirDigito(boleta[i]);
		printk( KERN_INFO "boleta[i]:%c\n", boleta[i]);
		ssleep(1);
	}
	
	limpiar();
	
	//printk(KERN_INFO "Boleta recibido: %s \n", boleta);

	return len;
}

static int __init funcion_inicio(void)
{
	int ret;

	printk(KERN_INFO "Iniciando el DDC \n");
	printk(KERN_INFO "Registrando el device driver de caracter... \n");

	ret = alloc_chrdev_region( &dispositivo, 0, 1, "ESCOM_DDC" );
	if( ret < 0 )
	{
		printk(KERN_ERR "Error al registrar el device driver de caracter \n");
		return ret;
	}
	printk(KERN_INFO "Dispositivo Registrado exitosamente... \n");
	printk(KERN_INFO "Numero mayor asignado: %d \n", MAJOR(dispositivo));
	printk(KERN_INFO "Numero menor asignado: %d \n", MINOR(dispositivo));

	cdev_init( &dev_cdev, &fops );
	ret = cdev_add( &dev_cdev, dispositivo, 1 );
	if( ret < 0 )
	{
		printk(KERN_ERR "Error al registrar las operaciones del device driver de caracter \n");
		unregister_chrdev_region( dispositivo, 1 );

		return ret;
	}
	printk(KERN_INFO "Operaciones Registradas exitosamente... \n");

	dev_class = class_create(THIS_MODULE, "ESCOM_class");
	if( IS_ERR( dev_class ) )
	{
		printk(KERN_ERR "Error al crear la clase del dispositivo \n");
		cdev_del( &dev_cdev );
		unregister_chrdev_region( dispositivo, 1 );

		return PTR_ERR(dev_class);
	}
	printk(KERN_INFO "Clase creada exitosamente... \n");

	dev_file = device_create( dev_class, NULL, dispositivo, NULL, "ESCOM_device");
	if( IS_ERR( dev_file ) )
	{
		printk(KERN_ERR "Error al crear el dispositivo \n");
		class_destroy( dev_class );
		cdev_del( &dev_cdev );
		unregister_chrdev_region( dispositivo, 1 );

		return PTR_ERR(dev_file);
	}
	printk(KERN_INFO "Dispositivo creado exitosamente... \n");
	
	
	//RANGO DE DIRECCIONES DE LOS REGISTROS A PARTIR DE LA DIRECCION BASE
	gpio_virtual = (uint32_t *)ioremap( ADDR_BASE_GPIO, 0X30 );
	configGPIO(gpio_virtual,10,0);
	configGPIO(gpio_virtual,17,0);
	configGPIO(gpio_virtual,18,0);
	configGPIO(gpio_virtual,22,0);
	configGPIO(gpio_virtual,23,0);
	configGPIO(gpio_virtual,24,0);
	configGPIO(gpio_virtual,27,0);
	

	return 0;
}

static void __exit funcion_exit(void)
{
	printk(KERN_INFO "Terminando la ejecucion del DDC \n");
	
	iounmap( gpio_virtual );

	device_destroy( dev_class, dispositivo );
	class_destroy( dev_class );
	cdev_del( &dev_cdev );
	unregister_chrdev_region( dispositivo, 1 );
}

module_init(funcion_inicio);
module_exit(funcion_exit);
