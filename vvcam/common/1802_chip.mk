INCLUDE_DIR := $(KERNEL_SOURCE_DIR)


EXTRA_CFLAGS += -Wno-unused-result
EXTRA_CFLAGS += -Wno-maybe-uninitialized
EXTRA_CFLAGS += -I$(PWD)/../isp/
EXTRA_CFLAGS += -I$(PWD)/../csi/samsung/
EXTRA_CFLAGS += -I$(PWD)/../sensor/ov2775/
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

vvcam-objs += ../dwe/dwe_ioctl.o
vvcam-objs += ../driver/dwe_driver.o
vvcam-objs += ../isp/cma.o
vvcam-objs += ../isp/isp_miv1.o
vvcam-objs += ../isp/isp_miv2.o
vvcam-objs += ../isp/isp_wdr3.o
vvcam-objs += ../isp/isp_3dnr.o
vvcam-objs += ../isp/isp_hdr.o
vvcam-objs += ../isp/isp_dpf.o
vvcam-objs += ../isp/isp_compand.o
vvcam-objs += ../isp/isp_gcmono.o
vvcam-objs += ../isp/isp_rgbgamma.o
vvcam-objs += ../isp/isp_ioctl.o
vvcam-objs += ../csi/samsung/sam_ioctl.o
vvcam-objs += ../csi/samsung/mxc-mipi-csi2-sam.o
vvcam-objs += ../sensor/ov2775/ov2775_ioctl.o
vvcam-objs += ../sensor/ov2775/ov2775_mipi_v3.o
vvcam-objs += ../driver/isp_driver.o
vvcam-objs += video.o

all:
	$(MAKE)  INC=$(INCLUDE_DIR) EXTRA="$(EXTRA_CFLAGS)" vvcamobj="$(vvcam-objs)"

clean:
	$(MAKE)  INC=$(INCLUDE_DIR) EXTRA="$(EXTRA_CFLAGS)" vvcamobj="$(vvcam-objs)" clean
