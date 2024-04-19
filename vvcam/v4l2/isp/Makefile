
PWD := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

include $(PWD)/../../version/ISP8000NANO_V1802.mk

TARGET = vvcam-isp

obj-m +=$(TARGET).o
$(TARGET)-objs += isp_driver_of.o
$(TARGET)-objs += ../video/vvbuf.o
$(TARGET)-objs += ../../isp/isp_miv1.o
$(TARGET)-objs += ../../isp/isp_miv2.o
$(TARGET)-objs += ../../isp/isp_wdr3.o
$(TARGET)-objs += ../../isp/isp_3dnr.o
$(TARGET)-objs += ../../isp/isp_hdr.o
$(TARGET)-objs += ../../isp/isp_dpf.o
$(TARGET)-objs += ../../isp/isp_compand.o
$(TARGET)-objs += ../../isp/isp_gcmono.o
$(TARGET)-objs += ../../isp/isp_ioctl.o
$(TARGET)-objs += ../../isp/isp_rgbgamma.o
$(TARGET)-objs += ../../isp/isp_isr.o

ccflags-y += -I$(PWD)
ccflags-y += -I$(PWD)/../
ccflags-y += -I$(PWD)/../../common/
ccflags-y += -I$(PWD)/../../isp/
ccflags-y += -O2 -Werror -Wmissing-declarations -Wmissing-prototypes

ccflags-y += -DISP_REG_SIZE=0x00010000

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
