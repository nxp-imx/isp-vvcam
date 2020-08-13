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
#include <linux/module.h>			/* Module support */
#include <linux/version.h>			/* Kernel version */
#include <linux/cdev.h>				/* Charactor device support */
#include <linux/vermagic.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/device.h>

#include <linux/ioctl.h>
#include <linux/mm.h>


#include "vvnative.h"
#include "vvnative_cfg.h"
#include "vvnative_combo.h"

#ifdef SENSOR_USE_GPIO_I2C
#include "soc_i2c.h"
#endif

#ifndef MODULE_LICENSE
# define MODULE_LICENSE(x)
#endif /* MODULE_LICENSE */

#ifndef MODULE_VERSION
# define MODULE_VERSION(x)
#endif /* MODULE_VERSION */

#define DRIVER_AUTHOR   "VeriSilicon IPD"
#define DRIVER_DESC     "Verisilicon ISP driver"
#define DRIVER_LICENSE  "GPL"
#define DRIVER_VERSION  "1.0"


#ifdef MODULE
MODULE_AUTHOR( DRIVER_AUTHOR );
MODULE_DESCRIPTION( DRIVER_DESC );
MODULE_LICENSE( DRIVER_LICENSE );
MODULE_INFO( vermagic, VERMAGIC_STRING);
#endif /* MODULE */


/* vvcam_isp_dev_major: device major number */
unsigned int vvcam_isp_dev_major = 0;
module_param(vvcam_isp_dev_major, uint, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(vvcam_isp_dev_major, "vvcam ISP device major number");

/* vvcam_isp_dev_minor: device base(start) minor number */
unsigned int vvcam_isp_dev_minor = 0;
module_param(vvcam_isp_dev_minor, uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(vvcam_isp_dev_minor, "vvcam ISP device base minor number");

/* vvcam_isp_dev_count: counts */
unsigned int vvcam_isp_dev_count = VVCAM_ISP_DEVICES;
module_param(vvcam_isp_dev_count, uint, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(vvcam_isp_dev_count, "vvcam ISP device counts");


struct vvcam_isp_dev *vvcam_isp_drv;
struct vvcam_common *vvcam_drv_common;

static int vvcam_isp_dev_open(struct inode *inode, struct file *file);
static int vvcam_isp_dev_release(struct inode *inode, struct file *file);
static long vvcam_isp_dev_ioctl(struct file * pFile, unsigned int cmd, unsigned long arg);
static int vvcam_isp_dev_mmap(struct file *pFile, struct vm_area_struct *vma);
static int vvcam_isp_devio_remap(int dev_no);
static int vvcam_isp_devio_unmap(int dev_no);

long (*vvnative_ioctl[])(struct vvcam_isp_dev *vvcam_isp_drv,unsigned int cmd, void *args)=
{
	vvcam_combo_isp_ioctl,    /*devid0*/
	vvcam_combo_isp_ioctl,    /*devid1*/
	vvcam_combo_csi_ioctl,    /*devid2*/
	vvcam_combo_csi_ioctl,    /*devid3*/
	vvcam_combo_sensor_ioctl, /*devid4*/
	vvcam_combo_sensor_ioctl, /*devid5*/
	vvcam_combo_dwe_ioctl,    /*devid6*/
	vvcam_combo_vse_ioctl,    /*devid7*/
	vvcam_combo_soc_ioctl,    /*devid8*/
};

int (*vvnative_init[])(struct vvcam_isp_dev *vvcam_isp_drv)=
{
	vvcam_combo_isp_init,    /*devid0*/
	vvcam_combo_isp_init,    /*devid1*/
	vvcam_combo_csi_init,    /*devid2*/
	vvcam_combo_csi_init,    /*devid3*/
	vvcam_combo_sensor_init, /*devid4*/
	vvcam_combo_sensor_init, /*devid5*/
	vvcam_combo_dwe_init,    /*devid6*/
	vvcam_combo_vse_init,    /*devid7*/
	vvcam_combo_soc_init,    /*devid8*/
};

int (*vvnative_deinit[])(struct vvcam_isp_dev *vvcam_isp_drv)=
{
	vvcam_combo_isp_deinit,    /*devid0*/
	vvcam_combo_isp_deinit,    /*devid1*/
	vvcam_combo_csi_deinit,    /*devid2*/
	vvcam_combo_csi_deinit,    /*devid3*/
	vvcam_combo_sensor_deinit, /*devid4*/
	vvcam_combo_sensor_deinit, /*devid5*/
	vvcam_combo_dwe_deinit,    /*devid6*/
	vvcam_combo_vse_deinit,    /*devid7*/
	vvcam_combo_soc_deinit,    /*devid8*/
};

/*TODO:expand dual camera devs*/
struct file_operations vvcam_isp_fops = {
	.owner = THIS_MODULE,
	.open = vvcam_isp_dev_open,
	.release = vvcam_isp_dev_release,
	.unlocked_ioctl = vvcam_isp_dev_ioctl,
	.mmap = vvcam_isp_dev_mmap,
};

/*On-the-fly debug opitions*/
char * on_the_fly_str="VIV ISP local debug buffer";
static void vivdev_dump_buf(unsigned char *out_buf, unsigned char* dump_data, int len, unsigned char * phy_address)
{
	int m;
	int n;
	char *buf;
	buf = out_buf;
	int llength = 0x10;
	unsigned int udata;
	sprintf (buf, "Phy Addr: 0x%lx, Va Addr:0x%lx, len:%d\n", (unsigned long)phy_address, dump_data, len);
	buf += strlen(buf);

	for (m=0; m<len; m++)
	{
		if(m%llength == 0)
		{
				sprintf(buf, "%04x| ",  m);
				buf += strlen(buf);
		}

		if((m%2) == 0){
			sprintf (buf, " ");
			buf += strlen(buf);
		}
		udata = (unsigned int)(*(dump_data+m));
		sprintf (buf, "%02x", udata&0xff);
		buf += strlen(buf);

		if(m%llength == (llength-1))
		{
			sprintf (buf, " | ");
			buf += strlen(buf);

			for (n=llength;n>0;n--)
			{
				if ((*(dump_data+m+1-n)>31) && (*(dump_data+m+1-n)<127))
				{
					sprintf (buf, "%c", *(dump_data+m+1-n));
					buf += strlen(buf);
				}
				else
				{
					sprintf (buf, ".");
					buf += strlen(buf);
				}
			}
			sprintf (buf, "\n");
			buf += strlen(buf);
		}
	}
	sprintf (buf, "\n");
	buf += strlen(buf);
}

static ssize_t get_vivdev(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	const char * name;
	char dev_name_perfix[8];
	int dev_index;
	int ret;
	if(NULL == vvcam_drv_common)
	{
		pr_info("-->%s Error, null pointer of  vvcam_drv_common\n", __func__);
		return 0;
	}

	mutex_lock(&vvcam_drv_common->vvmutex);

	name = dev_name(dev);
	pr_info("-->%s dev_name name:%s\n", __func__, name);
	ret = sscanf(name, "%6s%d", dev_name_perfix, &dev_index);
	pr_info("-->%s sscanf ret:%d, name:%s, id:%d\n", __func__, ret, dev_name_perfix, dev_index);

	ret = sprintf(buf, "%s\n", vvcam_drv_common->viv_buf);
	mutex_unlock(&vvcam_drv_common->vvmutex);

	return ret;
}

static ssize_t set_vivdev(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t len)
{
	const char * name;
	char dev_name_perfix[8];
	int dev_index;

	unsigned int data;
	unsigned int addr;
	unsigned int read_cnt;
	unsigned char mode;
	int ret;

	if(NULL == vvcam_drv_common)
	{
		pr_info("-->%s Error, null pointer of  vvcam_drv_common\n", __func__);
		return 0;
	}

	mutex_lock(&vvcam_drv_common->vvmutex);

	name = dev_name(dev);
	pr_info("-->%s dev_name name:%s\n", __func__, name);
	ret = sscanf(name, "%6s%d", dev_name_perfix, &dev_index);
	pr_info("-->%s sscanf ret:%d, name:%s, id:%d\n", __func__, ret, dev_name_perfix, dev_index);

	ret = sscanf(buf, "%c, 0x%x, 0x%x, 0x%d", &mode, &addr, &data, &read_cnt);
	if(ret < 3)
	{
		sprintf(vvcam_drv_common->viv_buf, "Error input string %s The parameters number:%d, expect format: 'r/w, 0xAddress, 0xData, 0xReadCnt'",
			buf, ret);
		pr_info("Error input string %s The parameters number:%d, expect format: 'r/w, 0xAddress, 0xData, 0xReadNumber'",buf, ret);
		mutex_unlock(&vvcam_drv_common->vvmutex);
		return len;
	}

	sprintf(vvcam_drv_common->viv_buf, "init: %s, scanf ret:%d, mode:%c, addr:0x%x, data:0x%x, readcnt:0x%x", buf, ret, mode, addr, data, read_cnt);
	pr_info("-->%s init: %s, scanf ret:%d, mode:%c, addr:0x%x, data:0x%x, readcnt:0x%x", __func__, buf, ret, mode, addr, data, read_cnt);

	vivdev_dump_buf((unsigned char*)vvcam_drv_common->viv_buf, on_the_fly_str, strlen(on_the_fly_str), 0xaaaa);

	mutex_unlock(&vvcam_drv_common->vvmutex);

	return len;
}

static DEVICE_ATTR(vivdevio, S_IWUSR|S_IRUSR|S_IRGRP|S_IWGRP|S_IROTH, get_vivdev, set_vivdev);

static int vvcam_isp_dev_open(struct inode *inode, struct file *file)
{

	int minor = iminor(inode);
	pr_info("-->%s: ISPdev%d opened\n", __func__, minor);

	mutex_lock(&(vvcam_isp_drv + minor)->vvmutex);

	if((vvcam_isp_drv + minor)->open_cnt >= VVISP_OPEN_LIMITATION)
	{
		pr_info("-->%s: ISPdev%d cannot open device\n", __func__, minor);
		mutex_unlock(&(vvcam_isp_drv + minor)->vvmutex);
		return -EBUSY;
	}

	if((vvcam_isp_drv + minor)->initialized == 0)
	{
		pr_info("-->%s: ISPdev%d device not initialized\n", __func__, minor);
		mutex_unlock(&(vvcam_isp_drv + minor)->vvmutex);
		return -EBUSY;
	}

	(vvcam_isp_drv + minor)->open_cnt++;
	pr_info("-->%s: ISPdev%d update open_cnt to %d\n", __func__, minor, (vvcam_isp_drv + minor)->open_cnt);
	try_module_get(THIS_MODULE);
	mutex_unlock(&(vvcam_isp_drv + minor)->vvmutex);

	return 0;
}


/* Called when a process closes the device file */
static int vvcam_isp_dev_release(struct inode *inode, struct file *file)
{
	int minor = iminor(inode);
	pr_info("-->%s: ISPdev%d releaseing...\n", __func__, minor);

	mutex_lock(&(vvcam_isp_drv + minor)->vvmutex);

	if((vvcam_isp_drv + minor)->open_cnt <= 0)
	{
		pr_info("-->%s: ISPdev%d internal error\n", __func__, minor);
		mutex_unlock(&(vvcam_isp_drv + minor)->vvmutex);
		return -EBUSY;
	}

	(vvcam_isp_drv + minor)->open_cnt--;
	module_put(THIS_MODULE);

	mutex_unlock(&(vvcam_isp_drv + minor)->vvmutex);

	return 0;
}

static long vvcam_isp_dev_ioctl(struct file * pFile, unsigned int cmd, unsigned long arg)
{
	long ret;
	struct inode *inode =pFile->f_mapping->host;
	int minor = iminor(inode);
	pr_info("-->%s: ISPdev%d Ioctl runs, cmd:%d, args:%ld...\n", __func__, minor, cmd, arg);

	ret = (*(vvnative_ioctl+minor))(vvcam_isp_drv + minor, cmd, arg);

	return ret;
};

static int vvcam_isp_dev_mmap(struct file *pFile, struct vm_area_struct *vma)
{
	struct inode *inode =pFile->f_mapping->host;
	int minor = iminor(inode);
	ulong img_buf_base = vvnative_get_img_buf_base();
	ulong img_buf_size = vvnative_get_img_buf_size();
	unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
	unsigned long pfn_start = (img_buf_base >> PAGE_SHIFT) + vma->vm_pgoff;
	unsigned long size = vma->vm_end - vma->vm_start;
	int ret = 0;

	if(size > img_buf_size)
	{
		size = img_buf_size;
	}
	pr_info("-->%s: ISPdev%d mmap runs...\n", __func__, minor);
	pr_info("-->%s: mmap parameters: phy start:0x%lx, ph size:0x%lx, vm start: 0x%lx, size: 0x%lx, offset, page start: 0x%lx\n",
	__func__, img_buf_base, img_buf_size, vma->vm_start, size, offset, pfn_start);

	/* Map reserved image buffer memory. */
	if (remap_pfn_range(vma, vma->vm_start,
		(img_buf_base >> PAGE_SHIFT),
		size, vma->vm_page_prot))
	{
		pr_err("-->%s: remap_pfn_range error!\n", __func__);
		pr_info("-->%s: mmap parameters: phy start:0x%lx, ph size:0x%lx, vm start: 0x%lx, size: 0x%lx, offset\n",
		__func__, img_buf_base, img_buf_size, vma->vm_start, size, offset);
		return -EIO;
	}

	return ret;
};

static int vvcam_isp_devio_remap(int dev_no)
{
	void __iomem *addr;
	ulong reg_base = vvnative_get_reg_base_by_idx(dev_no);
	ulong reg_size = vvnative_get_reg_size_by_idx(dev_no);

	if(NULL == vvcam_isp_drv)
	{
		pr_err("-->%s: NULL pointer!\n", __func__);
		return -1;
	}

	(vvcam_isp_drv + dev_no)->phy_address = reg_base;
	(vvcam_isp_drv + dev_no)->size = reg_size;
	pr_info("ioremap of dev: %d, phy address:0x%lx, size:0x%lx\n", dev_no, reg_base, reg_size);
	if((0 == reg_base) ||(0 == reg_size))
	{
		(vvcam_isp_drv + dev_no)->base_address = NULL;
		pr_info("ioremap of dev: %d, phy address is 0\n", dev_no);
	}else{
	addr = ioremap(reg_base, reg_size);
	pr_info("ioremap of dev: %d, address:0x%lx\n", dev_no, (unsigned long)addr);
	(vvcam_isp_drv + dev_no)->base_address = addr;
	}

	return 0;
}

static int vvcam_isp_devio_unmap(int dev_no)
{
	if((NULL == vvcam_isp_drv))
	{
		pr_err("-->%s: NULL pointer!\n", __func__);
		return -1;
	}
	if(NULL != (vvcam_isp_drv + dev_no)->base_address)
	{
		iounmap((vvcam_isp_drv + dev_no)->base_address);
	}
	return 0;
}

/*!
*******************************************************************************
** First function called by the OS. This function registers the device and
** vendor id that this driver handles.
*/
int __init vvcam_isp_dev_initialize( void )
{
	int result = 0;
	struct device *dev;
	dev_t devt;
	int retval;
	int dev_idx = 0;
	struct class *vvisp_class;

	pr_info("-->%s enter\n", __func__);
	pr_info("-->%s : %d, registerMemBase = 0x%lx, dev number:%d\n", __func__, __LINE__, VVISP0_BASE, VVCAM_ISP_DEVICES);

	/**********************************************************************
	* register/alloc the device major number and range, if
	*   major = 0, alloc from kernel to get an available device major number
	*   major != 0, use the specific value as major number, this value may be invalid
	**********************************************************************/
	vvcam_isp_drv = kzalloc(sizeof(struct vvcam_isp_dev) * VVCAM_ISP_DEVICES, GFP_KERNEL);
	if (!vvcam_isp_drv)
	{
		return -ENOMEM;
	}

	vvcam_drv_common = kzalloc(sizeof(struct vvcam_common), GFP_KERNEL);
	if (!vvcam_drv_common)
	{
		return -ENOMEM;
	}

	if (0 == vvcam_isp_dev_major)
	{
		result = alloc_chrdev_region(&devt, 0, VVCAM_ISP_DEVICES, DRIVER_NAME);
		if (result)
		{
			goto fail_check;
		}
		vvcam_isp_dev_major = MAJOR(devt);
		vvcam_isp_dev_minor = MINOR(devt);
	}
	else
	{
		devt = MKDEV(vvcam_isp_dev_major, vvcam_isp_dev_minor);
		result = register_chrdev_region(devt, VVCAM_ISP_DEVICES, DRIVER_NAME);
		if (result)
		{
			result = -EBUSY;
			goto fail_check;
		}
	}

	pr_info("--> vvcam ISP init, major id: %d, minor id: %d\n", MAJOR(devt), MINOR(devt));

	vvisp_class = class_create(THIS_MODULE, DRIVER_NAME);
	if (IS_ERR(vvisp_class))
	{
		pr_info("-->%s : %d,  class_create error!\n", __func__, __LINE__);
		goto fail_check;
	}

	dev_idx = 0;
	for(dev_idx = 0; dev_idx < VVCAM_ISP_DEVICES; dev_idx++)
	{
		if(strcmp(vvnative_get_dev_name_by_idx(dev_idx), VVNATIVE_NA) == 0)
		{
			pr_info("--> subisp%d is absent, id number:%s, skip..\n", dev_idx, vvnative_get_dev_name_by_idx(dev_idx) );
			(vvcam_isp_drv + dev_idx)->initialized = 0;
			continue;
		}

		(vvcam_isp_drv + dev_idx)->devt =  MKDEV(vvcam_isp_dev_major, vvcam_isp_dev_minor + dev_idx);

		pr_info("--> subisp%d reg, major id: %d, minor id: %d, dev_name:%s\n", dev_idx,
			MAJOR((vvcam_isp_drv + dev_idx)->devt), MINOR((vvcam_isp_drv + dev_idx)->devt),
			vvnative_get_dev_name_by_idx(dev_idx));

		cdev_init(&(vvcam_isp_drv + dev_idx)->cdev, &vvcam_isp_fops);
		(vvcam_isp_drv + dev_idx)->cdev.owner = THIS_MODULE;

		retval = cdev_add(&(vvcam_isp_drv + dev_idx)->cdev, (vvcam_isp_drv + dev_idx)->devt, 1);
		if (retval)
		{
			pr_info("-->%s : %d,  cdev_add error!\n", __func__, __LINE__);
			goto fail_check;
		}

		(vvcam_isp_drv + dev_idx)->class = vvisp_class;

		dev = device_create((vvcam_isp_drv + dev_idx)->class, NULL, (vvcam_isp_drv + dev_idx)->devt,
			(vvcam_isp_drv + dev_idx), "%s%d", vvnative_get_dev_name_by_idx(dev_idx), vvnative_get_dev_idx(dev_idx));
		if (IS_ERR(dev))
		{
			pr_info("-->%s : %d,  device_create error!\n", __func__, __LINE__);
			class_destroy(vvcam_isp_drv->class);
			goto fail_check;
		}
		pr_info("-->dev create for subisp %s %d done\n", DRIVER_NAME, dev_idx);
		(vvcam_isp_drv + dev_idx)->dev = dev;


		if(sysfs_create_file(&(dev->kobj), &dev_attr_vivdevio.attr)) {
			pr_info("-->%s : %d,  sysfs_create_file error!\n", __func__, __LINE__);
			goto fail_check;
		}

		/* init */
		mutex_init(&(vvcam_isp_drv + dev_idx)->vvmutex);
		(vvcam_isp_drv + dev_idx)->open_cnt = 0;
		(vvcam_isp_drv + dev_idx)->initialized = 1;
		(vvcam_isp_drv + dev_idx)->dev_idx = vvnative_get_dev_idx(dev_idx);

		result = vvcam_isp_devio_remap(dev_idx);
		if(result)
		{
			pr_err("-->%s: vvcam_isp_devio_remap error!\n", __func__);
			goto fail_check;
		}

		/* submodule initialization */
		result = (*(vvnative_init+dev_idx))(vvcam_isp_drv + dev_idx);
		if(result)
		{
			pr_err("-->%s: vvnative_init error with subdev:%d!\n", __func__, dev_idx);
			goto fail_check;
		}
	}

	/* on-the-fly debug init */
	mutex_init(&vvcam_drv_common->vvmutex);
	sprintf(vvcam_drv_common->viv_buf, "vivisp_buf");
	return result;

fail_check:
	/* Unregister char driver */
	pr_info("-->vvcam_isp_dev_initialize error\n");
	unregister_chrdev_region(devt, VVCAM_ISP_DEVICES);
	return result;
}

/*!
*******************************************************************************
** Unload the driver module
*/
void __exit vvcam_isp_dev_cleanup( void )
{
	int result = 0;
	int dev_idx = 0;

	pr_info("-->%s enter\n", __func__);
	if(NULL == vvcam_isp_drv)
	{
		pr_info("-->%s Internal error, NULL pointer\n", __func__);
		return;
	}
	for(dev_idx = 0; dev_idx < VVCAM_ISP_DEVICES; dev_idx++)
	{
		if(0 == (vvcam_isp_drv + dev_idx)->initialized)
		{
			continue;
		}

		/* submodule un-initialization */
		result = (*(vvnative_deinit+dev_idx))(vvcam_isp_drv + dev_idx);
		if(result)
		{
			pr_err("-->%s: vvnative_deinit error with subdev:%d!\n", __func__, dev_idx);
		}

		result = vvcam_isp_devio_unmap(dev_idx);
		if(result)
		{
			pr_err("-->%s: vvcam_isp_devio_unmap error!\n", __func__);
		}

		device_destroy((vvcam_isp_drv + dev_idx)->class, (vvcam_isp_drv + dev_idx)->devt);

		cdev_del(&(vvcam_isp_drv + dev_idx)->cdev);
		/* remove the char device structure (has been added) */

		unregister_chrdev_region((vvcam_isp_drv + dev_idx)->devt, VVCAM_ISP_DEVICES);
		(vvcam_isp_drv + dev_idx)->dev = NULL;
		(vvcam_isp_drv + dev_idx)->open_cnt = 0;
	}

	class_destroy(vvcam_isp_drv->class);
	vvcam_isp_drv->class = NULL;

	kfree(vvcam_isp_drv);
	vvcam_isp_drv = NULL;
	kfree(vvcam_drv_common);
	vvcam_drv_common = NULL;

	return;
}

module_init(vvcam_isp_dev_initialize);
module_exit(vvcam_isp_dev_cleanup);

MODULE_LICENSE ("GPL");