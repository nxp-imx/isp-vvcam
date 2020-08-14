#!/bin/sh

if [ "$KERNEL_SOURCE_DIR" == "" ]; then
export KERNEL_SOURCE_DIR=/build/users/nxa18621/proj/imx8/linux-imx
fi

#manually clean vvcam (if needed)

cd vvcam
find -name *.o | xargs rm -fv
find -name *.ko | xargs rm -fv
find -name *.o.cmd | xargs rm -fv 
cd -

cd vvcam/v4l2
#make -f 1802_chip.mk clean
make KERNEL_SRC=$KERNEL_SOURCE_DIR clean
make KERNEL_SRC=$KERNEL_SOURCE_DIR ENABLE_IRQ=yes
cd -

rm -rf modules
mkdir -p modules

cp vvcam/v4l2/vvcam-dwe.ko modules
cp vvcam/v4l2/sensor/ov2775/ov2775.ko modules
cp vvcam/v4l2/sensor/os08a20/os08a20.ko modules
cp vvcam/v4l2/csi/samsung/vvcam-csis.ko modules
cp vvcam/v4l2/vvcam-isp.ko modules
cp vvcam/v4l2/video/vvcam-video.ko modules
cp vvcam/v4l2/sensor/camera-proxy-driver/basler-camera-driver-vvcam.ko modules
