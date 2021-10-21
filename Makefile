obj-m += tarea-1.o
obj-m += tarea-2.o
obj-m += tarea-4.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

compilar:
	gcc aplicacion4.c -o app