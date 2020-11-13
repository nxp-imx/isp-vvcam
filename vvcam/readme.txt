isp kernel driver (verisilicon)

file description:
  vvcam/common/video.c      - register v4l2 video device, filename is /dev/videox, process standard v4l2 command.
  vvcam/driver/isp_driver.c - register v4l2 subdev, filename is /dev/v4l-subdevx, supply full isp options for all modules.
  units/ispdrv/isp/ic_dev.h     - defined all isp main/sub modules structure.
  units/ispdrv/isp/isp_ioctl.h  - defined all isp ioctls and internal functions.
  units/ispdrv/isp/isp_ioctl.c  - dispatch ioctl and implement isp main functions.(need create new file if module code number larger than 100, such as wdr3, 3dnr)
  units/ispdrv/isp/cma.h        - continuous memory allocator, user may need replace it.

setup environment:
  cd appshell
  source environment-setup-<tool-chain>-linux
  source environment-setup-x86_64-linux

build:
  cd vvcam/native
  make VERSION_CFG=ISP8000NANO_V1802
clean:
	cd vvcam/native
	make clean

install:
  cd vvcam/native/bin
  ./insmod.sh

unistall:
	cd vvcam/native/bin
  ./rmmod.sh


porting guide:
  1. modify ISP_REG_BASE and ISP_REG_BASE in version.h
  2. modify RESERVED_MEM_BASE and RESERVED_MEM_SIZE in version.h
  3. for use cma, user need modify linux dts file to map the device memory.
       or replace vb2_cma_alloc to default vb2 mem_ops.

