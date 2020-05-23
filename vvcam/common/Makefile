INCLUDE_DIR := $(INC)
SRC := $(shell pwd)
MODULE_NAME = vvcam

obj-m := $(MODULE_NAME).o
$(MODULE_NAME)-objs := \
	video.o \
	../driver/dwe_driver.o \
	cma.o \
	../isp/isp_miv1.o \
	../isp/isp_wdr3.o \
	../isp/isp_hdr.o \
	../isp/isp_dpf.o \
	../isp/isp_compand.o \
	../isp/isp_gcmono.o \
	../isp/isp_ioctl.o \
	../csi/samsung/sam_ioctl.o \
	../csi/samsung/mxc-mipi-csi2-sam.o \
	../sensor/ov2775/ov2775_mipi_v3.o \
	../sensor/os08a20/os08a20_mipi_v3.o \
	../sensor/camera-proxy-driver/basler-camera-driver-vvcam/basler-camera-driver-vvcam.o \
	../driver/isp_driver.o \
	../dwe/dwe_ioctl.o \

EXTRA_CFLAGS += -I$(PWD)/../isp/
EXTRA_CFLAGS += -I$(PWD)/../csi/samsung/
EXTRA_CFLAGS += -I$(PWD)/../sensor/
EXTRA_CFLAGS += -I$(PWD)/../driver/
EXTRA_CFLAGS += -I$(PWD)/../common/
EXTRA_CFLAGS += -I$(PWD)/../dwe/

EXTRA_CFLAGS += -DISP8000NANO_V1802
EXTRA_CFLAGS += -DWITH_DWE
EXTRA_CFLAGS += -DCSI_SENSOR_KERNEL

EXTRA_CFLAGS += -DISP_REG_BASE0=0x32E10000
EXTRA_CFLAGS += -DISP_REG_BASE1=0x32E20000
EXTRA_CFLAGS += -DISP_REG_SIZE=0x00010000
EXTRA_CFLAGS += -DISP_HW_NUMBER=1
EXTRA_CFLAGS += -DDWE_REG_BASE=0x32E30000
EXTRA_CFLAGS += -DDWE_REG_SIZE=0x00010000
EXTRA_CFLAGS += -DRESERVED_MEM_BASE=0xB0000000
EXTRA_CFLAGS += -DRESERVED_MEM_SIZE=0x10000000
EXTRA_CFLAGS += -DCSI_REG_BASE=0x32e40000
EXTRA_CFLAGS += -DCSI_REG_SIZE=0x00010000

EXTRA_CFLAGS += -DISP_WDR_V3
EXTRA_CFLAGS += -DISP_MIV1
EXTRA_CFLAGS += -DISP_CMPD
 
all:
	make -C $(KERNEL_SRC) M=$(SRC)

modules_install:
	make -C $(KERNEL_SRC) M=$(SRC) modules_install
 
 clean:
	make -C $(KERNEL_SRC) M=$(SRC) clean
	find . -name '*.o' -delete
	rm -f *.o *.ko *.mod.c
	rm -f Module.symvers modules.order
