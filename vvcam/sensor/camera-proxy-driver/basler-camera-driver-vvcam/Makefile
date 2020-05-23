ifneq ($(KERNELRELEASE),)

   obj-m += basler-camera-driver-vvcam.o 

else
# normal makefile
SRC := $(shell pwd)

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC)

modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC) EXTRA_CFLAGS+=-DCONFIG_BASLER_CAMERA_VVCAM modules_install

clean:
	rm -f *.o *~ core .depend .*.cmd *.ko *.mod.c
	rm -f Module.markers Module.symvers modules.order
	rm -rf .tmp_versions Modules.s

endif
