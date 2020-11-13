/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 VeriSilicon Holdings Co., Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************
 *
 * The GPL License (GPL)
 *
 * Copyright (c) 2020 VeriSilicon Holdings Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;
 *
 *****************************************************************************
 *
 * Note: This software is released under dual MIT and GPL licenses. A
 * recipient may use this file under the terms of either the MIT license or
 * GPL License. If you wish to use only one license not the other, you can
 * indicate your decision by deleting one of the above license notices in your
 * version of this file.
 *
 *****************************************************************************/
#ifndef _CSI_IOC_H_
#define _CSI_IOC_H_

#ifndef __KERNEL__
#include <stdint.h>
#endif
#include <linux/ioctl.h>

enum {
	VVCSI_IOC_S_RESET = _IO('r', 0),
	VVCSI_IOC_S_POWER,
	VVCSI_IOC_G_POWER,
	VVCSI_IOC_S_CLOCK,
	VVCSI_IOC_G_CLOCK,
	VVCSI_IOC_S_STREAM,
	VVCSI_IOC_G_STREAM,
	VVCSI_IOC_S_FMT,
	VVCSI_IOC_G_FMT,
	VVCSI_IOC_S_VC_SELECT,
	VVCSI_IOC_G_VC_SELECT,
	VVCSI_IOC_S_LANE_CFG,
	VVCSI_IOC_MAX,
};

struct csi_vc_select_context {
	uint32_t csi_vc_select_mode;
	uint32_t vc_channel;
};

struct csi_format_context {
	uint32_t format;
	uint32_t width;
	uint32_t height;
};

struct vvcam_csi_hardware_function_s
{
	int (*init)(void* dev);
	int (*exit)(void* dev);
	int (*reset)(void* dev);
	int (*set_power)(void* dev);
	int (*get_power)(void* dev);
	int (*set_clock)(void* dev);
	int (*get_clock)(void* dev);
	int (*set_stream_control)(void* dev);
	int (*get_stream_control)(void* dev);
	int (*set_fmt)(void* dev);
	int (*get_fmt)(void* dev);
	int (*set_vc_select)(void* dev);
	int (*get_vc_select)(void* dev);
	int (*set_lane_cfg)(void* dev);
};

struct vvcam_csi_lane_cfg
{
	uint32_t mipi_lane_num;
};

struct vvcam_csi_access_s
{
	int (*write)(void * ctx, uint32_t address, uint32_t data);
	int (*read)(void * ctx, uint32_t address, uint32_t *data);
};


#ifdef __KERNEL__

struct vvcam_csi_dev {
	void __iomem *base;
	char name[16];

	int present;
	int device_idx;

	uint32_t power_status;
	uint32_t clock;

	uint32_t streaming_enable;
	struct csi_vc_select_context csi_vc_select;
	struct csi_format_context csi_format;
	struct vvcam_csi_hardware_function_s csi_hard_func;
	struct vvcam_csi_access_s  csi_access;
	struct vvcam_csi_lane_cfg csi_lane_cfg;
	void * csi_private;
};

// internal functions

long csi_priv_ioctl(struct vvcam_csi_dev *dev, unsigned int cmd, void *args);
int vvnative_csi_init(struct vvcam_csi_dev *dev);
int vvnative_csi_deinit(struct vvcam_csi_dev *dev);



#else
//User space connections


#endif

#endif  // _CSI_IOC_H_
