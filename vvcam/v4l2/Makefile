ifeq ($(KERNEL_SRC),)
export KERNEL_SRC := $(shell dirname $(shell pwd))/../nxp_kernel/build_v8
endif

all:
	@cd dwe;      make || exit $$?;
	@cd isp;      make || exit $$?;
	@cd video;    make || exit $$?;
	@cd sensor;   make || exit $$?;
	@cd focus;    make || exit $$?;

clean:
	@cd dwe;      make clean;
	@cd isp;      make clean;
	@cd video;    make clean;
	@cd sensor;   make clean;
	@cd focus;    make clean;

modules_install:
	@cd dwe;      make modules_install;
	@cd isp;      make modules_install;
	@cd video;    make modules_install;
	@cd sensor;   make modules_install;
	@cd focus;    make modules_install;
