build
x86
make  BUILD_TYPE=fpga WITH_DWE=1 VERSION=ISP8000NANO_V1802 
arm64
make VERSION=ISP8000NANO_V1802 KERNEL_SRC=$(arm_kernel_src)

generate kernel modules, support hot plug.
find -name *.ko
./v4l2/vvcam-dwe.ko
./v4l2/sensor/ov2775/ov2775.ko
./v4l2/sensor/os08a20/os08a20.ko
./v4l2/csi/samsung/vvcam-csis.ko
./v4l2/video/vvcam-video.ko
./v4l2/vvcam-isp.ko
..
