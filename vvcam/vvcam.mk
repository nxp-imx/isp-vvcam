#the device/fsl/common/build/kernel.mk should be included before this file

KERNEL_DIR := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ
TARGET_ARCH := $(TARGET_KERNEL_ARCH)
VVCAM_CROSS_COMPILE := $(strip $(KERNEL_CROSS_COMPILE_WRAPPER))

VVCAM_SRC_PATH := $(VVCAM_PATH)/vvcam/common

#
# vvcam.ko
#

VVCAM_OUT := $(TARGET_OUT_INTERMEDIATES)/VVCAM_OBJ
VVCAM := \
	$(VVCAM_OUT)/vvcam.ko

KERNEL_CFLAGS ?= KCFLAGS=-mno-android
ARCH_TYPE ?= $(TARGET_ARCH)

KERNELENVSH := $(VVCAM_OUT)/kernelenv.sh
$(KERNELENVSH):
	mkdir -p $(VVCAM_OUT)
	echo 'export KERNEL_DIR=$(KERNEL_DIR)' > $(KERNELENVSH)
	echo 'export CROSS_COMPILE=$(VVCAM_CROSS_COMPILE)' >> $(KERNELENVSH)
	echo 'export ARCH_TYPE=$(ARCH_TYPE)' >> $(KERNELENVSH)

vvcam: $(KERNELENVSH) $(VVCAM_SRC_PATH)
	$(hide) if [ ${clean_build} = 1 ]; then \
		PATH=$$PATH $(MAKE) -f Kbuild -C $(VVCAM_SRC_PATH) clean; \
	fi
	@ . $(KERNELENVSH); $(kernel_build_shell_env) \
	$(MAKE) -f Kbuild -C $(VVCAM_SRC_PATH) \
		$(CLANG_TO_COMPILE) \
		$(KERNEL_CFLAGS) \
		ARCH_TYPE=$(ARCH_TYPE) \
		DEBUG=$(DEBUG); \
	cp $(VVCAM_SRC_PATH)/vvcam.ko $(VVCAM);