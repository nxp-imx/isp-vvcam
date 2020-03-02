#!/bin/bash
source /opt/fsl-imx-xwayland/5.4-zeus/environment-setup-aarch64-poky-linux
echo $(pwd)
make  -f 1802_chip.mk KERNEL_SOURCE_DIR=$(pwd)/../../kernel/build_v8 
