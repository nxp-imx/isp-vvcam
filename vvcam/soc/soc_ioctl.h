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
#ifndef _SOC_IOC_H_
#define _SOC_IOC_H_

#ifndef __KERNEL__
#include <stdint.h>
#endif

#include <linux/ioctl.h>

enum {
	VVSOC_IOC_S_RESET_ISP = _IO('r', 0),
	VVSOC_IOC_S_POWER_ISP,
	VVSOC_IOC_G_POWER_ISP,
	VVSOC_IOC_S_CLOCK_ISP,
	VVSOC_IOC_G_CLOCK_ISP,

	VVSOC_IOC_S_RESET_DWE,
	VVSOC_IOC_S_POWER_DWE,
	VVSOC_IOC_G_POWER_DWE,
	VVSOC_IOC_S_CLOCK_DWE,
	VVSOC_IOC_G_CLOCK_DWE,

	VVSOC_IOC_S_RESET_VSE,
	VVSOC_IOC_S_POWER_VSE,
	VVSOC_IOC_G_POWER_VSE,
	VVSOC_IOC_S_CLOCK_VSE,
	VVSOC_IOC_G_CLOCK_VSE,

	VVSOC_IOC_S_RESET_CSI,
	VVSOC_IOC_S_POWER_CSI,
	VVSOC_IOC_G_POWER_CSI,
	VVSOC_IOC_S_CLOCK_CSI,
	VVSOC_IOC_G_CLOCK_CSI,

	VVSOC_IOC_S_RESET_SENSOR,
	VVSOC_IOC_S_POWER_SENSOR,
	VVSOC_IOC_G_POWER_SENSOR,
	VVSOC_IOC_S_CLOCK_SENSOR,
	VVSOC_IOC_G_CLOCK_SENSOR,

	VVSOC_IOC_MAX,
};

struct soc_control_context {
	uint32_t device_idx;
	uint32_t control_value;
};

struct vvcam_soc_func_s
{
	int (*set_power)(void*,unsigned int,unsigned int);
	int (*get_power)(void*,unsigned int,unsigned int *);
	int (*set_reset)(void*,unsigned int,unsigned int);
	int (*set_clk)(void*,unsigned int,unsigned int);
	int (*get_clk)(void*,unsigned int,unsigned int *);
};

struct vvcam_soc_function_s
{
	struct vvcam_soc_func_s isp_func;
	struct vvcam_soc_func_s dwe_func;
	struct vvcam_soc_func_s vse_func;
	struct vvcam_soc_func_s csi_func;
	struct vvcam_soc_func_s sensor_func;
};

struct vvcam_soc_access_s
{
	int (*write)(void * ctx, uint32_t address, uint32_t data);
	int (*read)(void * ctx, uint32_t address, uint32_t *data);
};


#ifdef __KERNEL__

struct vvcam_soc_dev {
	void __iomem *base;
	struct soc_control_context isp0;
	struct soc_control_context isp1;
	struct soc_control_context dwe;
	struct soc_control_context vse;
	struct vvcam_soc_function_s soc_func;
	struct vvcam_soc_access_s soc_access;
	void * csi_private;
};
// internal functions

long soc_priv_ioctl(struct vvcam_soc_dev *dev, unsigned int cmd, void *args);
int vvnative_soc_init(struct vvcam_soc_dev *dev);
int vvnative_soc_deinit(struct vvcam_soc_dev *dev);



#else
//User space connections


#endif

#endif  // _SOC_IOC_H_
