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
#include <linux/module.h>
#include <linux/uaccess.h>

#include "soc_ioctl.h"
#include "vivsoc_hub.h"
#include "vsi_core_gen6.h"


int vivsoc_hub_isp_reset(struct vvcam_soc_dev *dev, struct soc_control_context * soc_ctrl)
{
	int ret = 0;
	if ((dev==NULL) || (soc_ctrl == NULL))
	{
		pr_err("[%s]:dev is null\n", __func__);
		return -1;
	}

	if (dev->soc_func.isp_func.set_reset == NULL)
	{
		pr_err("[%s] function is null\n", __func__);
		return -1;
	}

	ret = dev->soc_func.isp_func.set_reset(dev, soc_ctrl->device_idx, soc_ctrl->control_value);

	return ret;
}

int vivsoc_hub_isp_s_power_ctrl(struct vvcam_soc_dev *dev, struct soc_control_context * soc_ctrl)
{
	int ret = 0;
	if ((dev==NULL) || (soc_ctrl == NULL))
	{
		pr_err("[%s]:dev is null\n", __func__);
		return -1;
	}

	if (dev->soc_func.isp_func.set_power == NULL)
	{
		pr_err("[%s] function is null\n", __func__);
		return -1;
	}

	ret = dev->soc_func.isp_func.set_power(dev, soc_ctrl->device_idx, soc_ctrl->control_value);

	return ret;
}

int vivsoc_hub_isp_g_power_ctrl(struct vvcam_soc_dev *dev, struct soc_control_context * soc_ctrl)
{
	int ret = 0;
	if ((dev==NULL) || (soc_ctrl == NULL))
	{
		pr_err("[%s]:dev is null\n", __func__);
		return -1;
	}

	if (dev->soc_func.isp_func.get_power == NULL)
	{
		pr_err("[%s] function is null\n", __func__);
		return -1;
	}

	ret = dev->soc_func.isp_func.get_power(dev, soc_ctrl->device_idx, &soc_ctrl->control_value);

	return ret;
}

int vivsoc_hub_isp_s_clock_ctrl(struct vvcam_soc_dev *dev, struct soc_control_context * soc_ctrl)
{
	int ret = 0;
	if ((dev==NULL) || (soc_ctrl == NULL))
	{
		pr_err("[%s]:dev is null\n", __func__);
		return -1;
	}

	if (dev->soc_func.isp_func.set_clk == NULL)
	{
		pr_err("[%s] function is null\n", __func__);
		return -1;
	}

	ret = dev->soc_func.isp_func.set_clk(dev, soc_ctrl->device_idx, soc_ctrl->control_value);

	return ret;
}

int vivsoc_hub_isp_g_clock_ctrl(struct vvcam_soc_dev *dev, struct soc_control_context * soc_ctrl)
{
	int ret = 0;
	if ((dev==NULL) || (soc_ctrl == NULL))
	{
		pr_err("[%s]:dev is null\n", __func__);
		return -1;
	}

	if (dev->soc_func.isp_func.get_clk == NULL)
	{
		pr_err("[%s] function is null\n", __func__);
		return -1;
	}

	ret = dev->soc_func.isp_func.get_clk(dev, soc_ctrl->device_idx, &soc_ctrl->control_value);

	return ret;
}



int vivsoc_hub_dwe_reset(struct vvcam_soc_dev *dev, struct soc_control_context * soc_ctrl)
{
	int ret = 0;
	if ((dev==NULL) || (soc_ctrl == NULL))
	{
		pr_err("[%s]:dev is null\n", __func__);
		return -1;
	}

	if (dev->soc_func.dwe_func.set_reset == NULL)
	{
		pr_err("[%s] function is null\n", __func__);
		return -1;
	}

	ret = dev->soc_func.dwe_func.set_reset(dev, soc_ctrl->device_idx, soc_ctrl->control_value);

	return ret;
}

int vivsoc_hub_dwe_s_power_ctrl(struct vvcam_soc_dev *dev, struct soc_control_context * soc_ctrl)
{
	int ret = 0;
	if ((dev==NULL) || (soc_ctrl == NULL))
	{
		pr_err("[%s]:dev is null\n", __func__);
		return -1;
	}

	if (dev->soc_func.dwe_func.set_power == NULL)
	{
		pr_err("[%s] function is null\n", __func__);
		return -1;
	}

	ret = dev->soc_func.dwe_func.set_power(dev, soc_ctrl->device_idx, soc_ctrl->control_value);

	return ret;
}

int vivsoc_hub_dwe_g_power_ctrl(struct vvcam_soc_dev *dev, struct soc_control_context * soc_ctrl)
{
	int ret = 0;
	if ((dev==NULL) || (soc_ctrl == NULL))
	{
		pr_err("[%s]:dev is null\n", __func__);
		return -1;
	}

	if (dev->soc_func.dwe_func.get_power == NULL)
	{
		pr_err("[%s] function is null\n", __func__);
		return -1;
	}

	ret = dev->soc_func.dwe_func.get_power(dev, soc_ctrl->device_idx, &soc_ctrl->control_value);

	return ret;
}

int vivsoc_hub_dwe_s_clock_ctrl(struct vvcam_soc_dev *dev, struct soc_control_context * soc_ctrl)
{
	int ret = 0;
	if ((dev==NULL) || (soc_ctrl == NULL))
	{
		pr_err("[%s]:dev is null\n", __func__);
		return -1;
	}

	if (dev->soc_func.dwe_func.set_clk == NULL)
	{
		pr_err("[%s] function is null\n", __func__);
		return -1;
	}

	ret = dev->soc_func.dwe_func.set_clk(dev, soc_ctrl->device_idx, soc_ctrl->control_value);

	return ret;
}

int vivsoc_hub_dwe_g_clock_ctrl(struct vvcam_soc_dev *dev, struct soc_control_context * soc_ctrl)
{
	int ret = 0;
	if ((dev==NULL) || (soc_ctrl == NULL))
	{
		pr_err("[%s]:dev is null\n", __func__);
		return -1;
	}

	if (dev->soc_func.dwe_func.get_clk == NULL)
	{
		pr_err("[%s] function is null\n", __func__);
		return -1;
	}

	ret = dev->soc_func.dwe_func.get_clk(dev, soc_ctrl->device_idx, &soc_ctrl->control_value);

	return ret;
}



int vivsoc_hub_vse_reset(struct vvcam_soc_dev *dev, struct soc_control_context * soc_ctrl)
{
	int ret = 0;
	if ((dev==NULL) || (soc_ctrl == NULL))
	{
		pr_err("[%s]:dev is null\n", __func__);
		return -1;
	}

	if (dev->soc_func.vse_func.set_reset == NULL)
	{
		pr_err("[%s] function is null\n", __func__);
		return -1;
	}

	ret = dev->soc_func.vse_func.set_reset(dev, soc_ctrl->device_idx, soc_ctrl->control_value);

	return ret;
}

int vivsoc_hub_vse_s_power_ctrl(struct vvcam_soc_dev *dev, struct soc_control_context * soc_ctrl)
{
	int ret = 0;
	if ((dev==NULL) || (soc_ctrl == NULL))
	{
		pr_err("[%s]:dev is null\n", __func__);
		return -1;
	}

	if (dev->soc_func.vse_func.set_power == NULL)
	{
		pr_err("[%s] function is null\n", __func__);
		return -1;
	}

	ret = dev->soc_func.vse_func.set_power(dev, soc_ctrl->device_idx, soc_ctrl->control_value);

	return ret;
}

int vivsoc_hub_vse_g_power_ctrl(struct vvcam_soc_dev *dev, struct soc_control_context * soc_ctrl)
{
	int ret = 0;
	if ((dev==NULL) || (soc_ctrl == NULL))
	{
		pr_err("[%s]:dev is null\n", __func__);
		return -1;
	}

	if (dev->soc_func.vse_func.get_power == NULL)
	{
		pr_err("[%s] function is null\n", __func__);
		return -1;
	}

	ret = dev->soc_func.vse_func.get_power(dev, soc_ctrl->device_idx, &soc_ctrl->control_value);

	return ret;
}

int vivsoc_hub_vse_s_clock_ctrl(struct vvcam_soc_dev *dev, struct soc_control_context * soc_ctrl)
{
	int ret = 0;
	if ((dev==NULL) || (soc_ctrl == NULL))
	{
		pr_err("[%s]:dev is null\n", __func__);
		return -1;
	}

	if (dev->soc_func.vse_func.set_clk == NULL)
	{
		pr_err("[%s] function is null\n", __func__);
		return -1;
	}

	ret = dev->soc_func.vse_func.set_clk(dev, soc_ctrl->device_idx, soc_ctrl->control_value);

	return ret;
}

int vivsoc_hub_vse_g_clock_ctrl(struct vvcam_soc_dev *dev, struct soc_control_context * soc_ctrl)
{
	int ret = 0;
	if ((dev==NULL) || (soc_ctrl == NULL))
	{
		pr_err("[%s]:dev is null\n", __func__);
		return -1;
	}

	if (dev->soc_func.vse_func.get_clk == NULL)
	{
		pr_err("[%s] function is null\n", __func__);
		return -1;
	}

	ret = dev->soc_func.vse_func.get_clk(dev, soc_ctrl->device_idx, &soc_ctrl->control_value);

	return ret;
}



int vivsoc_hub_csi_reset(struct vvcam_soc_dev *dev, struct soc_control_context * soc_ctrl)
{
	int ret = 0;
	if ((dev==NULL) || (soc_ctrl == NULL))
	{
		pr_err("[%s]:dev is null\n", __func__);
		return -1;
	}

	if (dev->soc_func.csi_func.set_reset == NULL)
	{
		pr_err("[%s] function is null\n", __func__);
		return -1;
	}

	ret = dev->soc_func.csi_func.set_reset(dev, soc_ctrl->device_idx, soc_ctrl->control_value);

	return ret;
}

int vivsoc_hub_csi_s_power_ctrl(struct vvcam_soc_dev *dev, struct soc_control_context * soc_ctrl)
{
	int ret = 0;
	if ((dev==NULL) || (soc_ctrl == NULL))
	{
		pr_err("[%s]:dev is null\n", __func__);
		return -1;
	}

	if (dev->soc_func.csi_func.set_power == NULL)
	{
		pr_err("[%s] function is null\n", __func__);
		return -1;
	}

	ret = dev->soc_func.csi_func.set_power(dev, soc_ctrl->device_idx, soc_ctrl->control_value);

	return ret;
}

int vivsoc_hub_csi_g_power_ctrl(struct vvcam_soc_dev *dev, struct soc_control_context * soc_ctrl)
{
	int ret = 0;
	if ((dev==NULL) || (soc_ctrl == NULL))
	{
		pr_err("[%s]:dev is null\n", __func__);
		return -1;
	}

	if (dev->soc_func.csi_func.get_power == NULL)
	{
		pr_err("[%s] function is null\n", __func__);
		return -1;
	}

	ret = dev->soc_func.csi_func.get_power(dev, soc_ctrl->device_idx, &soc_ctrl->control_value);

	return ret;
}

int vivsoc_hub_csi_s_clock_ctrl(struct vvcam_soc_dev *dev, struct soc_control_context * soc_ctrl)
{
	int ret = 0;
	if ((dev==NULL) || (soc_ctrl == NULL))
	{
		pr_err("[%s]:dev is null\n", __func__);
		return -1;
	}

	if (dev->soc_func.csi_func.set_clk == NULL)
	{
		pr_err("[%s] function is null\n", __func__);
		return -1;
	}

	ret = dev->soc_func.csi_func.set_clk(dev, soc_ctrl->device_idx, soc_ctrl->control_value);

	return ret;
}

int vivsoc_hub_csi_g_clock_ctrl(struct vvcam_soc_dev *dev, struct soc_control_context * soc_ctrl)
{
	int ret = 0;
	if ((dev==NULL) || (soc_ctrl == NULL))
	{
		pr_err("[%s]:dev is null\n", __func__);
		return -1;
	}

	if (dev->soc_func.csi_func.get_clk == NULL)
	{
		pr_err("[%s] function is null\n", __func__);
		return -1;
	}

	ret = dev->soc_func.csi_func.get_clk(dev, soc_ctrl->device_idx, &soc_ctrl->control_value);

	return ret;
}

int vivsoc_hub_sensor_reset(struct vvcam_soc_dev *dev, struct soc_control_context * soc_ctrl)
{
	int ret = 0;
	if ((dev==NULL) || (soc_ctrl == NULL))
	{
		pr_err("[%s]:dev is null\n", __func__);
		return -1;
	}

	if (dev->soc_func.sensor_func.set_reset == NULL)
	{
		pr_err("[%s] function is null\n", __func__);
		return -1;
	}

	ret = dev->soc_func.sensor_func.set_reset(dev, soc_ctrl->device_idx, soc_ctrl->control_value);

	return ret;
}

int vivsoc_hub_sensor_s_power_ctrl(struct vvcam_soc_dev *dev, struct soc_control_context * soc_ctrl)
{
	int ret = 0;
	if ((dev==NULL) || (soc_ctrl == NULL))
	{
		pr_err("[%s]:dev is null\n", __func__);
		return -1;
	}

	if (dev->soc_func.sensor_func.set_power == NULL)
	{
		pr_err("[%s] function is null\n", __func__);
		return -1;
	}

	ret = dev->soc_func.sensor_func.set_power(dev, soc_ctrl->device_idx, soc_ctrl->control_value);

	return ret;
}

int vivsoc_hub_sensor_g_power_ctrl(struct vvcam_soc_dev *dev, struct soc_control_context * soc_ctrl)
{
	int ret = 0;
	if ((dev==NULL) || (soc_ctrl == NULL))
	{
		pr_err("[%s]:dev is null\n", __func__);
		return -1;
	}

	if (dev->soc_func.sensor_func.get_power == NULL)
	{
		pr_err("[%s] function is null\n", __func__);
		return -1;
	}

	ret = dev->soc_func.sensor_func.get_power(dev, soc_ctrl->device_idx, &soc_ctrl->control_value);

	return ret;
}

int vivsoc_hub_sensor_s_clock_ctrl(struct vvcam_soc_dev *dev, struct soc_control_context * soc_ctrl)
{
	int ret = 0;
	if ((dev==NULL) || (soc_ctrl == NULL))
	{
		pr_err("[%s]:dev is null\n", __func__);
		return -1;
	}

	if (dev->soc_func.sensor_func.set_clk == NULL)
	{
		pr_err("[%s] function is null\n", __func__);
		return -1;
	}

	ret = dev->soc_func.sensor_func.set_clk(dev, soc_ctrl->device_idx, soc_ctrl->control_value);

	return ret;
}

int vivsoc_hub_sensor_g_clock_ctrl(struct vvcam_soc_dev *dev, struct soc_control_context * soc_ctrl)
{
	int ret = 0;
	if ((dev==NULL) || (soc_ctrl == NULL))
	{
		pr_err("[%s]:dev is null\n", __func__);
		return -1;
	}

	if (dev->soc_func.sensor_func.get_clk == NULL)
	{
		pr_err("[%s] function is null\n", __func__);
		return -1;
	}

	ret = dev->soc_func.sensor_func.get_clk(dev, soc_ctrl->device_idx, &soc_ctrl->control_value);

	return ret;
}



unsigned int vivsoc_register_hardware(struct vvcam_soc_dev *dev, struct vvcam_soc_function_s *func)
{
	if (func == NULL)
	{
		return -1;
	}

	memcpy(&dev->soc_func,func,sizeof(struct vvcam_soc_function_s));

	dev->soc_access.write = gen6_write_reg;
	dev->soc_access.read = gen6_read_reg;

	return 0;
}




