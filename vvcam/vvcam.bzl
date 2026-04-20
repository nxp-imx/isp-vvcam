# SPDX-License-Identifier: GPL-2.0
# Copyright 2026 NXP
#
# Bazel build configuration for Verisilicon ISP VVCAM drivers
# Converted from original Makefiles in v4l2/ directory
#
# Modules built:
#   - vvcam-dwe.ko     (Dewarp Engine)
#   - vvcam-isp.ko     (ISP core)
#   - vvcam-video.ko   (V4L2 video device)
#   - os08a20.ko       (OS08A20 sensor driver)
#   - basler-camera-driver-vvcam.ko (Basler camera proxy)

load("//build/kernel/kleaf:kernel.bzl", "ddk_module", "kernel_module_group")

# =============================================================================
# Common compiler flags
# =============================================================================

_COMMON_COPTS = [
    "-O2",
    "-Werror",
    "-Wmissing-declarations",
    "-Wmissing-prototypes",
]

# ISP version flags from version/ISP8000NANO_V1802.mk
_ISP_VERSION_COPTS = [
    "-DISP_WDR_V3",
    "-DISP_MIV1",
    "-DISP_COMPAND",
    "-DISP8000NANO_BASE",
    "-DISP_MP_34BIT",
    "-DISP_FILTER",
    "-DISP8000NANO_V1802",
]

# =============================================================================
# Source files
# =============================================================================

# vvcam-dwe.ko sources - Dewarp Engine driver
# Based on v4l2/dwe/Makefile
_DWE_SRCS = [
    "dwe/dwe_ioctl.c",
    "dwe/dwe_isr.c",
    "v4l2/dwe/dwe_devcore.c",
    "v4l2/dwe/dwe_driver_of.c",
    "v4l2/video/vvbuf.c",
]

# vvcam-isp.ko sources - ISP core driver
# Based on v4l2/isp/Makefile + version/ISP8000NANO_V1802.mk
_ISP_SRCS = [
    "isp/isp_3dnr.c",
    "isp/isp_compand.c",
    "isp/isp_dpf.c",
    "isp/isp_gcmono.c",
    "isp/isp_hdr.c",
    "isp/isp_ioctl.c",
    "isp/isp_isr.c",
    "isp/isp_miv1.c",
    "isp/isp_miv2.c",
    "isp/isp_rgbgamma.c",
    "isp/isp_wdr3.c",
    "v4l2/isp/isp_driver_of.c",
    "v4l2/video/vvbuf.c",
]

# vvcam-video.ko sources - V4L2 video device driver
# Based on v4l2/video/Makefile
# Note: Depends on vvcam-dwe for symbols (KBUILD_EXTRA_SYMBOLS in original Makefile)
_VIDEO_SRCS = [
    "v4l2/video/video.c",
    "v4l2/video/vvbuf.c",
]

# =============================================================================
# Module definition macro
# =============================================================================

def define_vvcam(name_suffix, kernel_build, kernel_headers):
    """Define vvcam modules for a specific kernel build.

    Args:
        name_suffix: Suffix for target names (e.g., "imx8mp", "imx")
        kernel_build: Label for kernel_build target (e.g., "//kernel_imx:imx8mp")
        kernel_headers: Label for kernel headers (e.g., "//kernel_imx:all_headers_aarch64")
    """
    dwe_name = "vvcam-dwe_" + name_suffix
    isp_name = "vvcam-isp_" + name_suffix
    video_name = "vvcam-video_" + name_suffix
    os08a20_name = "os08a20_" + name_suffix
    basler_name = "basler-camera-driver-vvcam_" + name_suffix
    group_name = "vvcam_modules_" + name_suffix

    # vvcam-dwe.ko - Dewarp Engine driver
    # Based on v4l2/dwe/Makefile
    ddk_module(
        name = dwe_name,
        out = "vvcam-dwe.ko",
        srcs = _DWE_SRCS,
        hdrs = [":vvcam_headers"],
        copts = _COMMON_COPTS,
        kernel_build = kernel_build,
        deps = [kernel_headers],
    )

    # vvcam-isp.ko - ISP core driver
    # Based on v4l2/isp/Makefile + version/ISP8000NANO_V1802.mk
    ddk_module(
        name = isp_name,
        out = "vvcam-isp.ko",
        srcs = _ISP_SRCS,
        hdrs = [":vvcam_headers"],
        copts = _COMMON_COPTS + _ISP_VERSION_COPTS + [
            "-DISP_REG_SIZE=0x00010000",
        ],
        kernel_build = kernel_build,
        deps = [kernel_headers],
    )

    # vvcam-video.ko - V4L2 video device driver
    # Based on v4l2/video/Makefile
    # Note: Depends on vvcam-dwe for symbols (KBUILD_EXTRA_SYMBOLS in original Makefile)
    ddk_module(
        name = video_name,
        out = "vvcam-video.ko",
        srcs = _VIDEO_SRCS,
        hdrs = [":vvcam_headers"],
        copts = [
            "-O2",
            "-Werror",
            "-DRESERVED_MEM_BASE=0xB0000000",
            "-DRESERVED_MEM_SIZE=0x10000000",
        ],
        kernel_build = kernel_build,
        deps = [
            ":" + dwe_name,
            kernel_headers,
        ],
    )

    # os08a20.ko - OmniVision OS08A20 sensor driver
    # Based on v4l2/sensor/os08a20/Makefile
    ddk_module(
        name = os08a20_name,
        out = "os08a20.ko",
        srcs = ["v4l2/sensor/os08a20/os08a20_mipi_v3.c"],
        hdrs = [":vvcam_headers"],
        copts = _COMMON_COPTS + [
            "-DCONFIG_ANDROID",
        ],
        kernel_build = kernel_build,
        deps = [kernel_headers],
    )

    # basler-camera-driver-vvcam.ko - Basler camera proxy driver
    # Based on v4l2/sensor/camera-proxy-driver/Makefile
    ddk_module(
        name = basler_name,
        out = "basler-camera-driver-vvcam.ko",
        srcs = ["v4l2/sensor/camera-proxy-driver/basler-camera-driver-vvcam/basler-camera-driver-vvcam.c"],
        hdrs = [":vvcam_headers"],
        copts = _COMMON_COPTS + [
            "-DISP8000NANO_V1802",
            "-DCONFIG_HARDENED_USERCOPY",
        ],
        kernel_build = kernel_build,
        deps = [kernel_headers],
    )

    # All VVCAM ISP modules
    kernel_module_group(
        name = group_name,
        srcs = [
            ":" + basler_name,
            ":" + os08a20_name,
            ":" + dwe_name,
            ":" + isp_name,
            ":" + video_name,
        ],
    )
