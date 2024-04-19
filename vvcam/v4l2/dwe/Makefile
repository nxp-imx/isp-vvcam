PWD := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

TARGET = vvcam-dwe

obj-m +=$(TARGET).o
$(TARGET)-objs += dwe_driver_of.o
$(TARGET)-objs += ../video/vvbuf.o
$(TARGET)-objs += dwe_devcore.o
$(TARGET)-objs += ../../dwe/dwe_ioctl.o
$(TARGET)-objs += ../../dwe/dwe_isr.o

ccflags-y += -I$(PWD)
ccflags-y += -I$(PWD)/../
ccflags-y += -I$(PWD)/../../common/
ccflags-y += -I$(PWD)/../../dwe/
ccflags-y += -O2 -Werror -Wmissing-declarations -Wmissing-prototypes

ifeq ($(ENABLE_LATENCY_STATISTIC), yes)
ccflags-y += -DENABLE_LATENCY_STATISTIC
endif

ARCH_TYPE ?= arm64
ANDROID ?= no

ifeq ($(ANDROID), yes)

M ?= $(shell pwd)

V := 1

all:
	@$(MAKE) V=$(V) -C $(KERNEL_SRC) ARCH=$(ARCH_TYPE) M=$(M) modules
modules_install:
	@$(MAKE) V=$(V) -C $(KERNEL_SRC) M=$(M) modules_install
clean:
	@rm -rf modules.order Module.symvers
	@find ../ -name "*.o" | xargs rm -f
	@find ../ -name "*.ko" | xargs rm -f
else

all:
	make -C $(KERNEL_SRC) ARCH=$(ARCH_TYPE) M=$(PWD) modules
modules_install:
	make -C $(KERNEL_SRC) M=$(PWD) modules_install
clean:
	rm -rf $($(TARGET)-objs)
	make -C $(KERNEL_SRC) M=$(PWD) clean

endif
