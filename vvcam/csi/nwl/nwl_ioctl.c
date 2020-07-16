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
#ifdef __KERNEL__
#include "nwl_driver.h"
#endif
#include "nwl_ioctl.h"
#include "nwl_regs.h"

#ifndef __KERNEL__
#include <hal/hal_api.h>
#include "common_dev.h"

#define NWL_EXTREG_OFFSET 0x308244
#define NWL_REG_OFFSET 0x300000

static HalHandle_t hal_handle;
void nwl_ic_set_hal(HalHandle_t hal)
{
	hal_handle = hal;
}

void nwl_write_reg(u32 offset, u32 val)
{
	offset += NWL_REG_OFFSET;
	HalWriteReg(hal_handle, offset, val);
}

u32 nwl_read_reg(u32 offset)
{
	offset += NWL_REG_OFFSET;
	return HalReadReg(hal_handle, offset);
}

u32 nwl_write_extreg(u32 offset, u32 val)
{
	offset += NWL_EXTREG_OFFSET;
	return HalReadReg(hal_handle, offset);
}

int nwl_set_stream(void *dev, int enable)
{
	u32 clock_status;
	u32 data_status;

	nwl_write_reg(MRV_MIPICSI1_NUM_LANES, 0x4);

	if (enable == true) {
		clock_status = 0x1;
		data_status = 0xFF;
	} else {
		clock_status = 0x0;
		data_status = 0x0;
	}
	nwl_write_reg(MRV_MIPICSI1_LANES_CLK, clock_status);
	nwl_write_reg(MRV_MIPICSI1_LANES_DATA, data_status);

	return 0;
}

int nwl_init(void)
{
	nwl_write_reg(MRV_MIPICSI1_NUM_LANES, 0x4);
	nwl_write_reg(MRV_MIPICSI1_LANES_CLK, 0x1);
	nwl_write_reg(MRV_MIPICSI1_LANES_DATA, 0xF);
	nwl_write_reg(MRV_MIPICSI1_IGNORE_VC, 0x1);
	nwl_write_extreg(MRV_MIPICSI1_OUT_SHIFT, 0x4);

	return 0;
}
#endif

int nwl_ioc_init(void)
{
	nwl_init();

	return 0;
}

int nwl_ioc_s_stream(void *dev, void *__user args)
{
	int enable;

	copy_from_user(&enable, args, sizeof(enable));
	nwl_set_stream(dev, enable);
	return 0;
}

long nwl_priv_ioctl(void *dev, unsigned int cmd, void *args)
{
	int ret = -1;

	switch (cmd) {
	case CSIIOC_INIT:
		ret = nwl_ioc_init();
		break;
	case CSIIOC_S_STREAM:{
			ret = nwl_ioc_s_stream(dev, args);
		}
		break;
	default:
		pr_err("Unsupported csi command %d.\n", cmd);
		break;
	}

	return ret;
}
