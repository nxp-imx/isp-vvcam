PWD := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

TARGET = os08a20

obj-m +=$(TARGET).o
$(TARGET)-objs += os08a20_mipi_v3.o

ccflags-y += -I$(PWD)/../../../common/
ccflags-y += -O2 -Werror -Wmissing-declarations -Wmissing-prototypes

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
