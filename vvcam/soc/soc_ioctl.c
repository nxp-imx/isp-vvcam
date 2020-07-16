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

#ifndef __KERNEL__
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

#define pr_info printf
#define pr_err printf
#define copy_from_user(a, b, c) soc_copy_data(a, b, c)
#define copy_to_user(a, b, c) soc_copy_data(a, b, c)
#define __user
#define __iomem

void soc_copy_data(void *dst, void *src, int size)
{
	if (dst != src)
		memcpy(dst, src, size);
}

#else  // __KERNEL__
#include <linux/module.h>			/* Module support */
#include <linux/uaccess.h>

#endif

#include "soc_ioctl.h"
#include "vivsoc_hub.h"


long soc_priv_ioctl(struct vvcam_soc_dev *dev, unsigned int cmd, void *args)
{
	int ret = -1;
	struct soc_control_context soc_ctrl;
	if (!dev) {
		return ret;
	}

	switch (cmd) {
	/* ISP part */
	case VVSOC_IOC_S_RESET_ISP:
		copy_from_user(&soc_ctrl, args, sizeof(soc_ctrl));
		ret = vivsoc_hub_isp_reset(dev, &soc_ctrl);
		break;

	case VVSOC_IOC_S_POWER_ISP:
		copy_from_user(&soc_ctrl, args, sizeof(soc_ctrl));
		ret = vivsoc_hub_isp_s_power_ctrl(dev, &soc_ctrl);
		break;

	case VVSOC_IOC_G_POWER_ISP:
		copy_from_user(&soc_ctrl, args, sizeof(soc_ctrl));
		ret = vivsoc_hub_isp_g_power_ctrl(dev, &soc_ctrl);
		copy_to_user(args, &soc_ctrl, sizeof(soc_ctrl));
		break;

	case VVSOC_IOC_S_CLOCK_ISP:
		copy_from_user(&soc_ctrl, args, sizeof(soc_ctrl));
		ret = vivsoc_hub_isp_s_clock_ctrl(dev, &soc_ctrl);
		break;

	case VVSOC_IOC_G_CLOCK_ISP:
		copy_from_user(&soc_ctrl, args, sizeof(soc_ctrl));
		ret = vivsoc_hub_isp_g_clock_ctrl(dev, &soc_ctrl);
		copy_to_user(args, &soc_ctrl, sizeof(soc_ctrl));
		break;

	/* DWE part */
	case VVSOC_IOC_S_RESET_DWE:
		copy_from_user(&soc_ctrl, args, sizeof(soc_ctrl));
		ret = vivsoc_hub_dwe_reset(dev, &soc_ctrl);
		break;

	case VVSOC_IOC_S_POWER_DWE:
		copy_from_user(&soc_ctrl, args, sizeof(soc_ctrl));
		ret = vivsoc_hub_dwe_s_power_ctrl(dev, &soc_ctrl);
		break;

	case VVSOC_IOC_G_POWER_DWE:
		copy_from_user(&soc_ctrl, args, sizeof(soc_ctrl));
		ret = vivsoc_hub_dwe_g_power_ctrl(dev, &soc_ctrl);
		copy_to_user(args, &soc_ctrl, sizeof(soc_ctrl));
		break;

	case VVSOC_IOC_S_CLOCK_DWE:
		copy_from_user(&soc_ctrl, args, sizeof(soc_ctrl));
		ret = vivsoc_hub_dwe_s_clock_ctrl(dev, &soc_ctrl);
		break;

	case VVSOC_IOC_G_CLOCK_DWE:
		copy_from_user(&soc_ctrl, args, sizeof(soc_ctrl));
		ret = vivsoc_hub_isp_g_clock_ctrl(dev, &soc_ctrl);
		copy_to_user(args, &soc_ctrl, sizeof(soc_ctrl));
		break;

	/* VSE part */
	case VVSOC_IOC_S_RESET_VSE:
		copy_from_user(&soc_ctrl, args, sizeof(soc_ctrl));
		ret = vivsoc_hub_vse_reset(dev, &soc_ctrl);
		break;

	case VVSOC_IOC_S_POWER_VSE:
		copy_from_user(&soc_ctrl, args, sizeof(soc_ctrl));
		ret = vivsoc_hub_vse_s_power_ctrl(dev, &soc_ctrl);
		break;

	case VVSOC_IOC_G_POWER_VSE:
		copy_from_user(&soc_ctrl, args, sizeof(soc_ctrl));
		ret = vivsoc_hub_vse_g_power_ctrl(dev, &soc_ctrl);
		copy_to_user(args, &soc_ctrl, sizeof(soc_ctrl));
		break;

	case VVSOC_IOC_S_CLOCK_VSE:
		copy_from_user(&soc_ctrl, args, sizeof(soc_ctrl));
		ret = vivsoc_hub_vse_s_clock_ctrl(dev, &soc_ctrl);
		break;

	case VVSOC_IOC_G_CLOCK_VSE:
		copy_from_user(&soc_ctrl, args, sizeof(soc_ctrl));
		ret = vivsoc_hub_vse_g_clock_ctrl(dev, &soc_ctrl);
		copy_to_user(args, &soc_ctrl, sizeof(soc_ctrl));
		break;


	/* CSI part */
	case VVSOC_IOC_S_RESET_CSI:
		copy_from_user(&soc_ctrl, args, sizeof(soc_ctrl));
		ret = vivsoc_hub_csi_reset(dev, &soc_ctrl);
		break;

	case VVSOC_IOC_S_POWER_CSI:
		copy_from_user(&soc_ctrl, args, sizeof(soc_ctrl));
		ret = vivsoc_hub_csi_s_power_ctrl(dev, &soc_ctrl);
		break;

	case VVSOC_IOC_G_POWER_CSI:
		copy_from_user(&soc_ctrl, args, sizeof(soc_ctrl));
		ret = vivsoc_hub_csi_g_power_ctrl(dev, &soc_ctrl);
		copy_to_user(args, &soc_ctrl, sizeof(soc_ctrl));
		break;

	case VVSOC_IOC_S_CLOCK_CSI:
		copy_from_user(&soc_ctrl, args, sizeof(soc_ctrl));
		ret = vivsoc_hub_csi_s_clock_ctrl(dev, &soc_ctrl);
		break;

	case VVSOC_IOC_G_CLOCK_CSI:
		copy_from_user(&soc_ctrl, args, sizeof(soc_ctrl));
		ret = vivsoc_hub_csi_g_clock_ctrl(dev, &soc_ctrl);
		copy_to_user(args, &soc_ctrl, sizeof(soc_ctrl));
		break;

	/* sensor part */
	case VVSOC_IOC_S_RESET_SENSOR:
		copy_from_user(&soc_ctrl, args, sizeof(soc_ctrl));
		ret = vivsoc_hub_sensor_reset(dev, &soc_ctrl);
		break;

	case VVSOC_IOC_S_POWER_SENSOR:
		copy_from_user(&soc_ctrl, args, sizeof(soc_ctrl));
		ret = vivsoc_hub_sensor_s_power_ctrl(dev, &soc_ctrl);
		break;

	case VVSOC_IOC_G_POWER_SENSOR:
		copy_from_user(&soc_ctrl, args, sizeof(soc_ctrl));
		ret = vivsoc_hub_sensor_g_power_ctrl(dev, &soc_ctrl);
		copy_to_user(args, &soc_ctrl, sizeof(soc_ctrl));
		break;

	case VVSOC_IOC_S_CLOCK_SENSOR:
		copy_from_user(&soc_ctrl, args, sizeof(soc_ctrl));
		ret = vivsoc_hub_sensor_s_clock_ctrl(dev, &soc_ctrl);
		break;

	case VVSOC_IOC_G_CLOCK_SENSOR:
		copy_from_user(&soc_ctrl, args, sizeof(soc_ctrl));
		ret = vivsoc_hub_sensor_g_clock_ctrl(dev, &soc_ctrl);
		copy_to_user(args, &soc_ctrl, sizeof(soc_ctrl));
		break;


	default:
		pr_err("unsupported command %d", cmd);
		break;
	}

	return ret;
}

extern struct vvcam_soc_function_s  gen6_soc_function;

int vvnative_soc_init(struct vvcam_soc_dev *dev)
{
	if (dev ==  NULL)
	{
		pr_err("[%s] dev is NULL\n", __func__);
		return -1;
	}
	
	vivsoc_register_hardware(dev, &gen6_soc_function);
	return 0;
}

int vvnative_soc_deinit(struct vvcam_soc_dev *dev)
{
	return 0;
}


