CONFIG_MODULE_SIG=y
obj-m += Kprobe.o
Kprobe-objs := kprobe.o
CURRENT_PATH := $(shell pwd)
LINUX_KERNEL := /usr/src/kernels/$(shell uname -r)
LINUX_KERNEL_PATH := $(LINUX_KERNEL)
all:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) modules
install: all
	sudo insmod ./Kprobe.ko
clean:
	sudo rmmod ./Kprobe.ko
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) clean
