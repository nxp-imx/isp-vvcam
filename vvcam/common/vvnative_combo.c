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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
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
#include <linux/cdev.h>				/* Charactor device support */
#include <linux/i2c.h>

#include "isp_ioctl.h"
#include "sensor_ioctl.h"
#include "csi_ioctl.h"
#include "soc_ioctl.h"
#include "dwe_ioctl.h"
#include "vse_ioctl.h"



#include "vvnative.h"
#include "vvnative_combo.h"

/* IOCTL combos */
long vvcam_combo_isp_ioctl(struct vvcam_isp_dev *vvcam_isp_drv,unsigned int cmd, void *args)
{
	long ret;
	struct isp_ic_dev * dev;

	if(NULL == vvcam_isp_drv)
	{
		pr_err("-->%s: NULL pointer input!\n", __func__);
		return -1;
	}

	mutex_lock(&vvcam_isp_drv->vvmutex);
	pr_info("-->%s: Ioctl runs, cmd:%d, args:%p...\n", __func__, cmd, args);

	dev = (struct isp_ic_dev *)vvcam_isp_drv->private_ctx;
	ret = isp_priv_ioctl(dev, cmd, args);
	mutex_unlock(&vvcam_isp_drv->vvmutex);

	return ret;
}


long vvcam_combo_csi_ioctl(struct vvcam_isp_dev *vvcam_isp_drv,unsigned int cmd, void *args)
{
	long ret;
	struct vvcam_csi_dev *dev;

	if(NULL == vvcam_isp_drv)
	{
		pr_err("-->%s: NULL pointer input!\n", __func__);
		return -1;
	}

	mutex_lock(&vvcam_isp_drv->vvmutex);

	dev = (struct vvcam_csi_dev *)vvcam_isp_drv->private_ctx;
	ret = csi_priv_ioctl(dev, cmd, args);

	mutex_unlock(&vvcam_isp_drv->vvmutex);

	return ret;
}

#if 0
static struct i2c_board_info sensor_i2c_info =
{
	I2C_BOARD_INFO("sensor_ov2775", 0x00),
};

static void *vvcamGetSensorI2cClient(struct vvcam_isp_dev *vvcam_isp_drv)
{
	struct i2c_adapter *i2c_adap;
	static struct i2c_client *i2c_client = NULL;
	if (i2c_client == NULL)
	{
		i2c_adap = i2c_get_adapter(0);
		i2c_client = i2c_new_device(i2c_adap, &sensor_i2c_info);
		i2c_put_adapter(i2c_adap);
	}

	return i2c_client;
}
#endif

long vvcam_combo_sensor_ioctl(struct vvcam_isp_dev *vvcam_isp_drv,unsigned int cmd, void *args)
{
	long ret;
	struct vvcam_sensor_dev * dev;

	if(NULL == vvcam_isp_drv)
	{
		pr_err("-->%s: NULL pointer input!\n", __func__);
		return -1;
	}

	mutex_lock(&vvcam_isp_drv->vvmutex);
	pr_info("-->%s: Ioctl runs, cmd:%d, args:%p...\n", __func__, cmd, args);

	dev = (struct vvcam_sensor_dev *)vvcam_isp_drv->private_ctx;
	ret = sensor_priv_ioctl(dev, cmd, args);
	mutex_unlock(&vvcam_isp_drv->vvmutex);

	return ret;
}

long vvcam_combo_dwe_ioctl(struct vvcam_isp_dev *vvcam_isp_drv,unsigned int cmd, void *args)
{
	long ret;
	struct dwe_ic_dev *dev;

	mutex_lock(&vvcam_isp_drv->vvmutex);
	pr_info("-->%s: Ioctl runs, cmd:%d, args:%p...\n", __func__, cmd, args);

	dev = (struct dwe_ic_dev *)vvcam_isp_drv->private_ctx;
	ret = dwe_priv_ioctl(dev, cmd, args);

	mutex_unlock(&vvcam_isp_drv->vvmutex);

	return ret;
}

long vvcam_combo_vse_ioctl(struct vvcam_isp_dev *vvcam_isp_drv,unsigned int cmd, void *args)
{
	long ret;
	struct vse_ic_dev *dev;

	mutex_lock(&vvcam_isp_drv->vvmutex);
	pr_info("-->%s: Ioctl runs, cmd:%d, args:%p...\n", __func__, cmd, args);

	dev = (struct vse_ic_dev *)vvcam_isp_drv->private_ctx;
	ret = vse_priv_ioctl(dev, cmd, args);

	mutex_unlock(&vvcam_isp_drv->vvmutex);

	return ret;
}

long vvcam_combo_soc_ioctl(struct vvcam_isp_dev *vvcam_isp_drv,unsigned int cmd, void *args)
{
	long ret;
	struct vvcam_soc_dev *dev;

	if(NULL == vvcam_isp_drv)
	{
		pr_err("-->%s: NULL pointer input!\n", __func__);
		return -1;
	}

	mutex_lock(&vvcam_isp_drv->vvmutex);

	dev = (struct vvcam_soc_dev *)vvcam_isp_drv->private_ctx;
	ret = soc_priv_ioctl(dev, cmd, args);

	mutex_unlock(&vvcam_isp_drv->vvmutex);

	return ret;
}

/* Submodule init combos */
int vvcam_combo_isp_init(struct vvcam_isp_dev *vvcam_isp_drv)
{
	int ret = 0;
	struct isp_ic_dev * dev;
	if(NULL == vvcam_isp_drv)
	{
		pr_err("-->%s: input NULL pointer!\n", __func__);
		return -1;
	}

	vvcam_isp_drv->private_ctx = kzalloc(sizeof(struct isp_ic_dev), GFP_KERNEL);
	if(NULL == vvcam_isp_drv->private_ctx)
	{
		pr_err("-->%s: internal alloc memory error!\n", __func__);
		return -1;
	}

	dev = (struct isp_ic_dev *)vvcam_isp_drv->private_ctx;
	dev->base = vvcam_isp_drv->base_address;

	dev->reset = NULL;

	return ret;
}

int vvcam_combo_csi_init(struct vvcam_isp_dev *vvcam_isp_drv)
{
	int ret = 0;
	struct vvcam_csi_dev * dev;
	if(NULL == vvcam_isp_drv)
	{
		pr_err("-->%s: input NULL pointer!\n", __func__);
		return -1;
	}

	vvcam_isp_drv->private_ctx = kzalloc(sizeof(struct vvcam_csi_dev), GFP_KERNEL);
	if(NULL == vvcam_isp_drv->private_ctx)
	{
		pr_err("-->%s: internal alloc memory error!\n", __func__);
		return -1;
	}

	dev = (struct vvcam_csi_dev *)vvcam_isp_drv->private_ctx;

	dev->base = vvcam_isp_drv->base_address;
	dev->device_idx = vvcam_isp_drv->dev_idx;
	ret = vvnative_csi_init(dev);
	if (ret != 0)
	{
		pr_err("-->%s: vvnative_csi_init error!\n", __func__);
		return -1;
	}

	return ret;
}

int vvcam_combo_sensor_init(struct vvcam_isp_dev *vvcam_isp_drv)
{
	int ret = 0;
	struct vvcam_sensor_dev * dev;
	if(NULL == vvcam_isp_drv)
	{
		pr_err("-->%s: input NULL pointer!\n", __func__);
		return -1;
	}

	vvcam_isp_drv->private_ctx = kzalloc(sizeof(struct vvcam_sensor_dev), GFP_KERNEL);
	if(NULL == vvcam_isp_drv->private_ctx)
	{
		pr_err("-->%s: internal alloc memory error!\n", __func__);
		return -1;
	}

	dev = (struct vvcam_sensor_dev *)vvcam_isp_drv->private_ctx;
	dev->phy_addr   = vvcam_isp_drv->phy_address;
	dev->reg_size   = vvcam_isp_drv->size;
	dev->base       = vvcam_isp_drv->base_address;
	dev->device_idx = vvcam_isp_drv->dev_idx;

	ret = vvnative_sensor_init(dev);
	if (ret != 0)
	{
		pr_err("-->%s: vvnative_sensor_init error!\n", __func__);
		return -1;
	}

	return ret;
}

int vvcam_combo_dwe_init(struct vvcam_isp_dev *vvcam_isp_drv)
{
	int ret = 0;
	struct dwe_ic_dev * dev;

	if(NULL == vvcam_isp_drv)
	{
		pr_err("-->%s: input NULL pointer!\n", __func__);
		return -1;
	}

	vvcam_isp_drv->private_ctx = kzalloc(sizeof(struct dwe_ic_dev), GFP_KERNEL);
	if(NULL == vvcam_isp_drv->private_ctx)
	{
		pr_err("-->%s: internal alloc memory error!\n", __func__);
		return -1;
	}

	dev = (struct dwe_ic_dev *)vvcam_isp_drv->private_ctx;
	dev->base = vvcam_isp_drv->base_address;

	ret = vvnative_dwe_init(dev);
	if (ret != 0)
	{
		pr_err("-->%s: vvnative_dwe_init error!\n", __func__);
		return -1;
	}

	return ret;
}

int vvcam_combo_vse_init(struct vvcam_isp_dev *vvcam_isp_drv)
{
	int ret = 0;
	struct vse_ic_dev * dev;
	if(NULL == vvcam_isp_drv)
	{
		pr_err("-->%s: input NULL pointer!\n", __func__);
		return -1;
	}

	vvcam_isp_drv->private_ctx = kzalloc(sizeof(struct vse_ic_dev), GFP_KERNEL);
	if(NULL == vvcam_isp_drv->private_ctx)
	{
		pr_err("-->%s: internal alloc memory error!\n", __func__);
		return -1;
	}

	dev = (struct vse_ic_dev *)vvcam_isp_drv->private_ctx;
	dev->base = vvcam_isp_drv->base_address;

	ret = vvnative_vse_init(dev);
	if (ret != 0)
	{
		pr_err("-->%s: vvnative_vse_init error!\n", __func__);
		return -1;
	}

	return ret;
}

int vvcam_combo_soc_init(struct vvcam_isp_dev *vvcam_isp_drv)
{
	int ret = 0;
	struct vvcam_soc_dev * dev;

	if(NULL == vvcam_isp_drv)
	{
		pr_err("-->%s: input NULL pointer!\n", __func__);
		return -1;
	}

	vvcam_isp_drv->private_ctx = kzalloc(sizeof(struct vvcam_soc_dev), GFP_KERNEL);
	if(NULL == vvcam_isp_drv->private_ctx)
	{
		pr_err("-->%s: internal alloc memory error!\n", __func__);
		return -1;
	}

	dev = (struct vvcam_soc_dev *)vvcam_isp_drv->private_ctx;
	dev->base = vvcam_isp_drv->base_address;
	vvnative_soc_init(dev);

	return ret;
}



/* Submodule deinit combos */
int vvcam_combo_isp_deinit(struct vvcam_isp_dev *vvcam_isp_drv)
{
	int ret = 0;
	if(NULL == vvcam_isp_drv)
	{
		pr_err("-->%s: input NULL pointer!\n", __func__);
		return -1;
	}

	if(NULL != vvcam_isp_drv->private_ctx)
	{
		kzfree(vvcam_isp_drv->private_ctx);
		vvcam_isp_drv->private_ctx = NULL;
	}

	return ret;
}

int vvcam_combo_csi_deinit(struct vvcam_isp_dev *vvcam_isp_drv)
{
	int ret = 0;
	struct vvcam_csi_dev * dev;

	if(NULL == vvcam_isp_drv)
	{
		pr_err("-->%s: input NULL pointer!\n", __func__);
		return -1;
	}

	if(NULL != vvcam_isp_drv->private_ctx)
	{
		dev = (struct vvcam_csi_dev *)vvcam_isp_drv->private_ctx;
		vvnative_csi_deinit(dev);
		kzfree(vvcam_isp_drv->private_ctx);
		vvcam_isp_drv->private_ctx = NULL;
	}

	return ret;
}

int vvcam_combo_sensor_deinit(struct vvcam_isp_dev *vvcam_isp_drv)
{
	int ret = 0;
	struct vvcam_sensor_dev * dev;
	if(NULL == vvcam_isp_drv)
	{
		pr_err("-->%s: input NULL pointer!\n", __func__);
		return -1;
	}

	if(NULL != vvcam_isp_drv->private_ctx)
	{
		dev = (struct vvcam_sensor_dev *)vvcam_isp_drv->private_ctx;
		vvnative_sensor_deinit(dev);
		kzfree(vvcam_isp_drv->private_ctx);
		vvcam_isp_drv->private_ctx = NULL;
	}

	return ret;
}

int vvcam_combo_dwe_deinit(struct vvcam_isp_dev *vvcam_isp_drv)
{
	int ret = 0;
	struct dwe_ic_dev * dev;
	if(NULL == vvcam_isp_drv)
	{
		pr_err("-->%s: input NULL pointer!\n", __func__);
		return -1;
	}

	if(NULL != vvcam_isp_drv->private_ctx)
	{
		dev = (struct dwe_ic_dev *)vvcam_isp_drv->private_ctx;
		vvnative_dwe_deinit(dev);
		kzfree(vvcam_isp_drv->private_ctx);
		vvcam_isp_drv->private_ctx = NULL;
	}

	return ret;
}

int vvcam_combo_vse_deinit(struct vvcam_isp_dev *vvcam_isp_drv)
{
	int ret = 0;
	struct vse_ic_dev * dev;
	if(NULL == vvcam_isp_drv)
	{
		pr_err("-->%s: input NULL pointer!\n", __func__);
		return -1;
	}

	if(NULL != vvcam_isp_drv->private_ctx)
	{
		dev = (struct vse_ic_dev *)vvcam_isp_drv->private_ctx;
		vvnative_vse_deinit(dev);
		kzfree(vvcam_isp_drv->private_ctx);
		vvcam_isp_drv->private_ctx = NULL;
	}

	return ret;
}

int vvcam_combo_soc_deinit(struct vvcam_isp_dev *vvcam_isp_drv)
{
	int ret = 0;
	struct vvcam_soc_dev * dev;

	if(NULL == vvcam_isp_drv)
	{
		pr_err("-->%s: input NULL pointer!\n", __func__);
		return -1;
	}

	if(NULL != vvcam_isp_drv->private_ctx)
	{
		dev = (struct vvcam_soc_dev *)vvcam_isp_drv->private_ctx;
		vvnative_soc_deinit(dev);
		kzfree(vvcam_isp_drv->private_ctx);
		vvcam_isp_drv->private_ctx = NULL;
	}

	return ret;
}
