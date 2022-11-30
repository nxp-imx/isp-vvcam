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

#include <linux/io.h>
#include <linux/module.h>
#include "mrv_all_bits.h"
#include "isp_ioctl.h"
#include "isp_types.h"

extern MrvAllRegister_t *all_regs;

int isp_enable_rgbgamma(struct isp_ic_dev *dev)
{
#ifndef ISP_RGBGC
	pr_err("unsupported function %s\n", __func__);
	return -1;
#else

	u32 isp_ctrl = isp_read_reg(dev, REG_ADDR(isp_ctrl));

	REG_SET_SLICE(isp_ctrl, ISP_RGBGC_ENABLE, 1);
	isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);
	dev->rgbgamma.enable = true;
	return 0;
#endif
}

int isp_disable_rgbgamma(struct isp_ic_dev *dev)
{
#ifndef ISP_RGBGC
	pr_err("unsupported function %s\n", __func__);
	return -1;
#else
	u32 isp_ctrl = isp_read_reg(dev, REG_ADDR(isp_ctrl));

	REG_SET_SLICE(isp_ctrl, ISP_RGBGC_ENABLE, 0);
	isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);

	dev->rgbgamma.enable = false;
	return 0;
#endif
}

int isp_s_rgbgammapx(struct isp_ic_dev *dev, struct isp_rgbgamma_data *data)
{
#ifndef ISP_RGBGC
	pr_err("unsupported function %s\n", __func__);
	return -1;
#else
	u32 isp_gc_px_reg = REG_ADDR(isp_gcrgb_r_px_0);
	u32 *p_table = NULL;
	int i;
	u32 gc_px_data = 0;

	p_table = (u32 *)&data->rgbgc_r_px;
	for (i = 0; i < 64; i++) {
		gc_px_data |= (*(p_table + i) << (i % 6 * 5));
		if (i % 6 == 5 || i == 63) {
			isp_write_reg(dev, isp_gc_px_reg, gc_px_data);
			isp_gc_px_reg += 4;
			gc_px_data = 0;
		}
	}

	isp_gc_px_reg = REG_ADDR(isp_gcrgb_g_px_0);
	p_table = (u32 *)&data->rgbgc_g_px;
	for (i = 0; i < 64; i++) {
		gc_px_data |= (*(p_table + i) << (i % 6 * 5));
		if (i % 6 == 5 || i == 63) {
			isp_write_reg(dev, isp_gc_px_reg, gc_px_data);
			isp_gc_px_reg += 4;
			gc_px_data = 0;
		}
	}
	isp_gc_px_reg = REG_ADDR(isp_gcrgb_b_px_0);
	p_table = (u32 *)&data->rgbgc_b_px;
	for (i = 0; i < 64; i++) {
		gc_px_data |= (*(p_table + i) << (i % 6 * 5));
		if (i % 6 == 5 || i == 63) {
			isp_write_reg(dev, isp_gc_px_reg, gc_px_data);
			isp_gc_px_reg += 4;
			gc_px_data = 0;
		}
	}

#endif
   return 0;
}

int isp_s_rgbgammaWriteData(struct isp_ic_dev *dev,
			    struct isp_rgbgamma_data *data)
{
#ifndef ISP_RGBGC
	pr_err("unsupported function %s", __func__);
	return -1;
#else

	isp_write_reg(dev, REG_ADDR(isp_gcrgb_r_x_addr), 0);
	isp_write_reg(dev, REG_ADDR(isp_gcrgb_r_y_addr), 0);

	isp_write_reg(dev, REG_ADDR(isp_gcrgb_g_x_addr), 0);
	isp_write_reg(dev, REG_ADDR(isp_gcrgb_g_y_addr), 0);
	isp_write_reg(dev, REG_ADDR(isp_gcrgb_b_x_addr), 0);
	isp_write_reg(dev, REG_ADDR(isp_gcrgb_b_y_addr), 0);

	u32 isp_gc_y_data, isp_gc_x_data;
	int i;
	u32 *tblX, *tblY;

	tblX = data->rgbgc_r_datax;
	tblY = data->rgbgc_r_datay;
	for (i = 0; i < 64; i++) {
		isp_gc_y_data = *(tblY + i);
		isp_write_reg(dev, REG_ADDR(isp_gcrgb_r_y_write_data),
			      isp_gc_y_data);
	}
	for (i = 0; i < 63; i++) {
		isp_gc_x_data = *(tblX + i);
		isp_write_reg(dev, REG_ADDR(isp_gcrgb_r_x_write_data),
			      isp_gc_x_data);
	}

	tblX = data->rgbgc_g_datax;
	tblY = data->rgbgc_g_datay;
	for (i = 0; i < 64; i++) {
		isp_gc_y_data = *(tblY + i);
		isp_write_reg(dev, REG_ADDR(isp_gcrgb_g_y_write_data),
			      isp_gc_y_data);
	}
	for (i = 0; i < 63; i++) {
		isp_gc_x_data = *(tblX + i);
		isp_write_reg(dev, REG_ADDR(isp_gcrgb_g_x_write_data),
			      isp_gc_x_data);
	}
	tblX = data->rgbgc_b_datax;
	tblY = data->rgbgc_b_datay;
	for (i = 0; i < 64; i++) {
		isp_gc_y_data = *(tblY + i);
		isp_write_reg(dev, REG_ADDR(isp_gcrgb_b_y_write_data),
			      isp_gc_y_data);
	}
	for (i = 0; i < 63; i++) {
		isp_gc_x_data = *(tblX + i);
		isp_write_reg(dev, REG_ADDR(isp_gcrgb_b_x_write_data),
			      isp_gc_x_data);
	}
    return 0;
#endif
}

int isp_s_rgbgamma(struct isp_ic_dev *dev, struct isp_rgbgamma_data *data)
{
#ifndef ISP_RGBGC
	pr_err("unsupported function %s", __func__);
	return -1;
#else
	u32 isp_ctrl = isp_read_reg(dev, REG_ADDR(isp_ctrl));

	REG_SET_SLICE(isp_ctrl, ISP_RGBGC_ENABLE, 0);
	isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);

	isp_s_rgbgammapx(dev, data);
	isp_s_rgbgammaWriteData(dev, data);
    u8 ret = 0;
	if (dev->rgbgamma.enable) {
		 ret = isp_enable_rgbgamma(dev);
	}
    return ret;
#endif
}
