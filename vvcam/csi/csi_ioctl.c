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
#define copy_from_user(a, b, c) csi_copy_data(a, b, c)
#define copy_to_user(a, b, c) csi_copy_data(a, b, c)
#define __user
#define __iomem

void csi_copy_data(void *dst, void *src, int size)
{
	if (dst != src)
		memcpy(dst, src, size);
}

#else  // __KERNEL__
#include <linux/module.h>			/* Module support */
#include <linux/uaccess.h>

#endif

#include "csi_ioctl.h"
#include "vivcsi_hub.h"


long csi_priv_ioctl(struct vvcam_csi_dev *dev, unsigned int cmd, void *args)
{
	int ret = -1;

	if (!dev) {
		return ret;
	}

	switch (cmd) {
	case VVCSI_IOC_S_RESET:
		ret = vivcsi_hub_reset(dev);
		break;

	case VVCSI_IOC_S_POWER:
		copy_from_user(&dev->power_status, args, sizeof(dev->power_status));
		ret = vivcsi_hub_set_power(dev);
		break;

	case VVCSI_IOC_G_POWER:
		ret = vivcsi_hub_get_power(dev);
		copy_to_user(args, &dev->power_status, sizeof(dev->power_status));
		break;

	case VVCSI_IOC_S_CLOCK:
		copy_from_user(&dev->clock, args, sizeof(dev->clock));
		ret = vivcsi_hub_set_clock(dev);
		break;

	case VVCSI_IOC_G_CLOCK:
		ret = vivcsi_hub_get_clock(dev);
		copy_to_user(args, &dev->clock, sizeof(dev->clock));
		ret = 0;
		break;

	case VVCSI_IOC_S_STREAM:
		copy_from_user(&dev->streaming_enable, args, sizeof(dev->streaming_enable));
		ret = vivcsi_hub_set_stream_control(dev);
		break;

	case VVCSI_IOC_G_STREAM:
		ret = vivcsi_hub_get_stream_control(dev);
		copy_to_user(args, &dev->streaming_enable, sizeof(dev->streaming_enable));
		break;

	case VVCSI_IOC_S_FMT:
		copy_from_user(&dev->csi_format, args, sizeof(dev->csi_format));
		ret = vivcsi_hub_set_fmt(dev);
		break;

	case VVCSI_IOC_G_FMT:
		ret = vivcsi_hub_get_fmt(dev);
		copy_to_user(args, &dev->csi_format, sizeof(dev->csi_format));
		break;

	case VVCSI_IOC_S_VC_SELECT:
		copy_from_user(&dev->csi_vc_select, args, sizeof(dev->csi_vc_select));
		ret = vivcsi_hub_set_vc_select(dev);
		break;

	case VVCSI_IOC_G_VC_SELECT:
		ret = vivcsi_hub_get_vc_select(dev);
		copy_to_user(args, &dev->csi_vc_select, sizeof(dev->csi_vc_select));
		break;
	case VVCSI_IOC_S_LANE_CFG:
		copy_from_user(&dev->csi_lane_cfg, args, sizeof(dev->csi_lane_cfg));
		ret = vivcsi_hub_set_csi_lane_cfg(dev);
		break;
	default:
		pr_err("unsupported command %d", cmd);
		break;
	}

	return ret;
}

extern struct vvcam_csi_hardware_function_s nwl_mipi_function;

int vvnative_csi_init(struct vvcam_csi_dev *dev)
{
	int ret = 0;
	if (dev ==  NULL)
	{
		pr_err("[%s] dev is NULL\n", __func__);
		return -1;
	}

	vvcsi_register_hardware(dev,&nwl_mipi_function);
	if (dev->csi_hard_func.init)
	{
		ret = dev->csi_hard_func.init(dev);
		if (ret < 0)
		{
			pr_err("[%s] init failed\n", __func__);
			return -1;
		}
	}
	return 0;
}

int vvnative_csi_deinit(struct vvcam_csi_dev *dev)
{
	int ret = 0;
	if (dev ==  NULL)
	{
		pr_err("[%s] dev is NULL\n", __func__);
		return -1;
	}

	if (dev->csi_hard_func.exit)
	{
		ret = dev->csi_hard_func.exit(dev);
		if (ret < 0)
		{
			pr_err("[%s] exit failed\n", __func__);
			return -1;
		}
	}

	return 0;
}


