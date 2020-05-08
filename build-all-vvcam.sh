#!/bin/sh

export KERNEL_SOURCE_DIR=/build/users/nxa18621/proj/imx8/linux-imx
export KERNEL_SRC=/build/users/nxa18621/proj/imx8/linux-imx
#export KERNEL_SOURCE_DIR=/build/users/nxa18621/proj/imx8/linux-imx-caf

#manually clean vvcam (if needed)

cd vvcam
find -name *.o | xargs rm -fv
find -name *.ko | xargs rm -fv
find -name *.o.cmd | xargs rm -fv 
cd -

cd vvcam/common
#make -f 1802_chip.mk clean
make
cd -

