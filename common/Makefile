INCLUDE_DIR := $(INC)
EXTRA_CFLAGS := $(EXTRA)
vvcam-objs := $(vvcamobj)
obj-m += vvcam.o

all:
	make -C $(INCLUDE_DIR) $(ccflags) M=`pwd` modules

clean:
	make -C $(INCLUDE_DIR) M=`pwd` modules clean
	rm -rf modules.order
