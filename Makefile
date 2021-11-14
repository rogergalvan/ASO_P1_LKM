obj-m := fase1.o

all:
        $(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules
clean:
        $(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
install:
        insmod fase1.ko
delete:
        rmmod fase1.ko
