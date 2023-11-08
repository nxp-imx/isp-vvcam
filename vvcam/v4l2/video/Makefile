PWD := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

TARGET = vvcam-video
obj-m +=$(TARGET).o
$(TARGET)-objs += video.o
$(TARGET)-objs += vvbuf.o

ccflags-y += -I$(PWD)/../../common/
ccflags-y += -O2 -Werror
ccflags-y += -DRESERVED_MEM_BASE=0xB0000000
ccflags-y += -DRESERVED_MEM_SIZE=0x10000000

ARCH_TYPE ?= arm64
ANDROID ?= no

ifeq ($(ANDROID), yes)

M ?= $(shell pwd)

V := 1

all:
	@$(MAKE) V=$(V) -C $(KERNEL_SRC) ARCH=$(ARCH_TYPE) M=$(M) modules KBUILD_EXTRA_SYMBOLS=$(M)/../dwe/Module.symvers
modules_install:
	@$(MAKE) V=$(V) -C $(KERNEL_SRC) M=$(M) modules_install
clean:
	@rm -rf modules.order Module.symvers
	@find ../ -name "*.o" | xargs rm -f
	@find ../ -name "*.ko" | xargs rm -f

else

all:
	make -C $(KERNEL_SRC) ARCH=$(ARCH_TYPE) M=$(PWD) modules KBUILD_EXTRA_SYMBOLS=$(PWD)/../dwe/Module.symvers
modules_install:
	make -C $(KERNEL_SRC) M=$(PWD) modules_install
clean:
	rm -rf $($(TARGET)-objs)
	make -C $(KERNEL_SRC) M=$(PWD) clean
endif
