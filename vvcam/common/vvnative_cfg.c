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
#include "vvnative_cfg.h"

char vvcam_driver_name [][8]=
{
	#ifdef WITH_VVISP  /* devid 0 */
	VVNATIVE_ISP,
	#else
	VVNATIVE_NA,
	#endif
	#ifdef WITH_VVISP_DUAL  /* devid 1 */
	VVNATIVE_ISP,
	#else
	VVNATIVE_NA,
	#endif
	#ifdef WITH_VVCSI  /* devid 2 */
	VVNATIVE_CSI,
	#else
	VVNATIVE_NA,
	#endif
	#ifdef WITH_VVCSI_DUAL  /* devid 3 */
	VVNATIVE_CSI,
	#else
	VVNATIVE_NA,
	#endif
	#ifdef WITH_VVCAM  /* devid 4 */
	VVNATIVE_SENSOR,
	#else
	VVNATIVE_NA,
	#endif
	#ifdef WITH_VVCAM_DUAL  /* devid 5 */
	VVNATIVE_SENSOR,
	#else
	VVNATIVE_NA,
	#endif
	#ifdef WITH_VVDWE  /* devid 6 */
	VVNATIVE_DWE,
	#else
	VVNATIVE_NA,
	#endif
	#ifdef WITH_VVVSE  /* devid 7 */
	VVNATIVE_VSE,
	#else
	VVNATIVE_NA,
	#endif
	#ifdef WITH_VVCTL  /* devid 8 */
	VVNATIVE_SOC,
	#else
	VVNATIVE_NA,
	#endif
};

char * vvnative_get_dev_name_by_idx(int devidx)
{
	if((devidx > VVCAM_ISP_DEVICES)||(devidx < 0))
	{
		return VVNATIVE_NA;
	}
	return vvcam_driver_name[devidx];
}


/* Parameters that can be set with 'insmod' */
/* ISP */
static ulong vvisp0_reg_base = VVISP0_BASE;
module_param(vvisp0_reg_base, ulong, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );
MODULE_PARM_DESC(vvisp0_reg_base, "VVISP0 Reg Base address of AHB register");

static ulong vvisp1_reg_base = VVISP1_BASE;
module_param(vvisp1_reg_base, ulong, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );
MODULE_PARM_DESC(vvisp1_reg_base, "VVISP1 Reg Base address of AHB register");

static ulong vvisp_reg_size = VVISP_SIZE;
module_param(vvisp_reg_size, ulong, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(vvisp_reg_size, "VVISP0/1 Reg address range of AHB register");

/* CSI */
static ulong vvcsi0_reg_base = VVCSI0_BASE;
module_param(vvcsi0_reg_base, ulong, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(vvcsi0_reg_base, "VVCSI0 Reg Base address of AHB register");

static ulong vvcsi1_reg_base = VVCSI1_BASE;
module_param(vvcsi1_reg_base, ulong, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(vvcsi1_reg_base, "VVCSI1 Reg Base address of AHB register");

static ulong vvcsi_reg_size = VVCSI_SIZE;
module_param(vvcsi_reg_size, ulong, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(vvcsi_reg_size, "VVCSI0/1 Reg address range of AHB register");

/* Sensor */
static ulong vvcam0_reg_base = VVCAM0_BASE;
module_param(vvcam0_reg_base, ulong, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(vvcam0_reg_base, "VVCAM0 Reg Base address of AHB register");

static ulong vvcam1_reg_base = VVCAM1_BASE;
module_param(vvcam1_reg_base, ulong, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(vvcam1_reg_base, "VVCAM1 Reg Base address of AHB register");

static ulong vvcam_reg_size = VVCAM_SIZE;
module_param(vvcam_reg_size, ulong, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(vvcam_reg_size, "VVCAM0/1 Reg address range of AHB register");

/* Dwe */
static ulong vvdwe_reg_base = VVDWE_BASE;
module_param(vvdwe_reg_base, ulong, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(vvdwe_reg_base, "VVDWE Reg Base address of AHB register");

static ulong vvdwe_reg_size = VVDWE_SIZE;
module_param(vvdwe_reg_size, ulong, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(vvdwe_reg_size, "VVDWE Reg address range of AHB register");

/* Vse */
static ulong vvvse_reg_base = VVVSE_BASE;
module_param(vvvse_reg_base, ulong, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(vvvse_reg_base, "VVVSE Reg Base address of AHB register");

static ulong vvvse_reg_size = VVVSE_SIZE;
module_param(vvvse_reg_size, ulong, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(vvvse_reg_size, "VVVSE Reg address range of AHB register");

/* Crtl */
static ulong vvctl_reg_base = VVCTRL_BASE;
module_param(vvctl_reg_base, ulong, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(vvctl_reg_base, "VVCTRL Reg Base address of AHB register");

static ulong vvctl_reg_size = VVCTRL_SIZE;
module_param(vvctl_reg_size, ulong, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(vvctl_reg_size, "VVCTRL Reg address range of AHB register");


static ulong vvImgBufBase = 0x10000000;
module_param(vvImgBufBase, ulong, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(vvImgBufBase, "Base addrss of memory reserved for ISP");

static ulong vvImgBufSize = 0x10000000;
module_param(vvImgBufSize, ulong, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(vvImgBufSize, "Size of memory reserved for ISP");


ulong* reg_base_size_array[] =
{
	&vvisp0_reg_base, /* devid 0 */
	&vvisp_reg_size,
	&vvisp1_reg_base, /* devid 1 */
	&vvisp_reg_size,
	&vvcsi0_reg_base, /* devid 2 */
	&vvcsi_reg_size,
	&vvcsi1_reg_base, /* devid 3 */
	&vvcsi_reg_size,
	&vvcam0_reg_base, /* devid 4 */
	&vvcam_reg_size,
	&vvcam1_reg_base, /* devid 5 */
	&vvcam_reg_size,
	&vvdwe_reg_base,  /* devid 6 */
	&vvdwe_reg_size,
	&vvvse_reg_base,  /* devid 7 */
	&vvvse_reg_size,
	&vvctl_reg_base,  /* devid 8 */
	&vvctl_reg_size,
};

ulong vvnative_get_reg_base_by_idx(int devidx)
{
	if((devidx > VVCAM_ISP_DEVICES)||(devidx < 0))
	{
		return 0;
	}
	return *reg_base_size_array[devidx*2];
}

ulong vvnative_get_reg_size_by_idx(int devidx)
{
	if((devidx > VVCAM_ISP_DEVICES)||(devidx < 0))
	{
		return 0;
	}
	return *reg_base_size_array[devidx*2 + 1];
}

int reg_dev_idx_array[] =
{
	0, /* devid 0 isp0*/
	1, /* devid 1 isp1 */
	0, /* devid 2 csi0*/
	1, /* devid 3 csi1*/
	0, /* devid 4 cam0*/
	1, /* devid 5 cam1*/
	0,  /* devid 6 dwe0*/
	0,  /* devid 7 vse0*/
	0,  /* devid 8 ctrl0*/
};


int vvnative_get_dev_idx(int devidx)
{
	if((devidx > VVCAM_ISP_DEVICES)||(devidx < 0))
	{
		return -1;
	}
	return reg_dev_idx_array[devidx];
}


ulong vvnative_get_img_buf_base(void)
{
	return vvImgBufBase;
}

ulong vvnative_get_img_buf_size(void)
{
	return vvImgBufSize;
}
