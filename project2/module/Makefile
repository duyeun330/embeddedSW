obj-m   := hw2.o

KDIR    :=/work/achroimx_kernel
PWD     :=$(shell pwd)

all: driver

driver:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules ARCH=arm

install:
	cp -a hw2.ko /nfsroot
clean:
	rm -rf *.ko
	rm -rf *.mod.*
	rm -rf *.o
	rm -rf Module.symvers
	rm -rf modules.order
	rm -rf .led*
	rm -rf .tmp*
