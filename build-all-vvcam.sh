#!/bin/sh

if [ -z "${KERNEL_SOURCE_DIR}" ]; then
export KERNEL_SOURCE_DIR=/build/users/$USER/proj/imx8/linux-imx
fi

#manually clean vvcam (if needed)

cd vvcam
find -name *.o | xargs rm -fv
find -name *.ko | xargs rm -fv
find -name *.o.cmd | xargs rm -fv 
cd -

cd vvcam/v4l2
if [ "$1" != "" ]; then
	#echo "Arg 1 is BUILD_MODE: native or v4l2 ->"
	#echo $1
	BUILD_MODE=$1
else
	BUILD_MODE=v4l2
fi

BUILD_MODE=`echo $BUILD_MODE| tr '[:upper:]' '[:lower:]'`
echo "BUILD_MODE: $BUILD_MODE"
#make -f 1802_chip.mk clean
make KERNEL_SRC=$KERNEL_SOURCE_DIR clean
if [ "$BUILD_MODE" = 'v4l2' ]
then
   echo "v4l2 mode build --------------------->"
   make KERNEL_SRC=$KERNEL_SOURCE_DIR
fi

cd -

rm -rf modules
mkdir -p modules

cp vvcam/v4l2/vvcam-dwe.ko modules
cp vvcam/v4l2/sensor/ov2775/ov2775.ko modules
cp vvcam/v4l2/sensor/os08a20/os08a20.ko modules
#cp vvcam/v4l2/csi/samsung/vvcam-csis.ko modules
cp vvcam/v4l2/vvcam-isp.ko modules
cp vvcam/v4l2/video/vvcam-video.ko modules
cp vvcam/v4l2/sensor/camera-proxy-driver/basler-camera-driver-vvcam.ko modules
