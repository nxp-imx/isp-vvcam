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




(1)编译imx8-media-dev.ko
mkdir build_v8 && cd build_v8
make ARCH=arm64 menuconfig
		Device Drivers  --->
			[*] Staging drivers  --->
				[*]   Media staging drivers  --->
					i.MX8QXP/QM Camera ISI/MIPI Features support  --->
						<M> IMX8 Media Device Driver
make -j4

(2)编译vvcam
DWE disable IRQ:
make VERSION=ISP8000NANO_V1802 KERNEL_SRC=$(arm_kernel_src) ENABLE_IRQ=no

DWE enable IRQ:
make VERSION=ISP8000NANO_V1802 KERNEL_SRC=$(arm_kernel_src) ENABLE_IRQ=yes

(3)加载KO
ov2775 insmod:
cp $Kernel_SRC/driver/staging/media/imx/imx8-media-dev.ko to your board directory
insmod vvcam-video.ko
insmod ov2775.ko
insmod vvcam-dwe.ko
insmod vvcam-isp.ko
insmod imx8-media-dev.ko


os08a20 insmod:
cp $Kernel_SRC/driver/staging/media/imx/imx8-media-dev.ko to your board directory
insmod vvcam-video.ko
insmod os08a20.ko
insmod vvcam-dwe.ko
insmod vvcam-isp.ko
insmod imx8-media-dev.ko