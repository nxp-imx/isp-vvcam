ifeq ($(BUILD_TYPE), fpga)
INCLUDE_DIR := /usr/src/linux-headers-$(shell uname -r)
KERNEL_SRC := $(INCLUDE_DIR)
endif

SRC := $(shell pwd)

ANDROID ?= no
ifeq ($(ANDROID), yes)
  M ?= $(shell pwd)
  PWD := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
endif

EXTRA_CFLAGS += -I$(PWD)/../common/
EXTRA_CFLAGS += -I$(PWD)/../isp/
EXTRA_CFLAGS += -I$(PWD)/../v4l2/

EXTRA_CFLAGS += -O2 -Werror

include $(PWD)/../version/ISP8000NANO_V1802.mk

EXTRA_CFLAGS += -DISP_REG_SIZE=0x00010000
EXTRA_CFLAGS += -DDEC_REG_BASE=0x00600000
EXTRA_CFLAGS += -DDEC_REG_SIZE=0x2000
EXTRA_CFLAGS += -DRESERVED_MEM_BASE=0xB0000000
EXTRA_CFLAGS += -DRESERVED_MEM_SIZE=0x10000000

vvcam-isp-objs += ../isp/isp_miv1.o
vvcam-isp-objs += ../isp/isp_miv2.o
vvcam-isp-objs += ../isp/isp_wdr3.o
vvcam-isp-objs += ../isp/isp_3dnr.o
vvcam-isp-objs += ../isp/isp_hdr.o
vvcam-isp-objs += ../isp/isp_dpf.o
vvcam-isp-objs += ../isp/isp_compand.o
vvcam-isp-objs += ../isp/isp_gcmono.o
vvcam-isp-objs += ../isp/isp_ioctl.o
vvcam-isp-objs += ../isp/isp_rgbgamma.o
#vvcam-isp-objs += ../isp/isp_dec.o
#vvcam-isp_objs += ../isp/isp_dmsc2.o
vvcam-isp-objs += ../isp/isp_isr.o
vvcam-isp-objs += isp_driver_of.o
vvcam-isp-objs += video/vvbuf.o
obj-m += vvcam-isp.o

EXTRA_CFLAGS += -I$(PWD)/../dwe/
vvcam-dwe-objs += ../dwe/dwe_ioctl.o
vvcam-dwe-objs += ../dwe/dwe_isr.o
vvcam-dwe-objs += dwe_driver_of.o
vvcam-dwe-objs += dwe_devcore.o
vvcam-dwe-objs += video/vvbuf.o
obj-m += vvcam-dwe.o

#obj-m += csi/
obj-m += sensor/
obj-m += focus/
obj-m += video/

ifeq ($(ANDROID), yes)
V := 1

build_target := modules

ARCH_TYPE ?= arm64

all:
	@$(MAKE) V=1 -C $(KERNEL_SRC) ARCH=$(ARCH_TYPE) M=$(M)

modules_install:
	@$(MAKE) V=1 -C $(KERNEL_SRC) ARCH=$(ARCH_TYPE) M=$(M) modules_install

clean:
	@rm -rf modules.order Module.symvers
	@find ../ -name "*.o" | xargs rm -f
	@find ../ -name "*.ko" | xargs rm -f
else
all:
	make -C $(KERNEL_SRC) M=$(SRC)
modules_install:
	make -C $(KERNEL_SRC) M=$(SRC) modules_install

clean:
	make -C $(KERNEL_SRC) M=$(SRC) clean
	find ../ -name '*.o' -delete
	find ../ -name '*.ko' -delete
	find ../ -name 'modules.order' -delete
	find ../ -name 'Module.symvers' -delete
	find ../ -name '*.mod.c' -delete
	find ../ -name '*.cmd' -delete
endif
