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

int isp_enable_gcmono(struct isp_ic_dev *dev)
{
#ifndef ISP_GCMONO
	pr_err("unsupported function %s", __func__);
	return -1;
#else
	u32 isp_gcmono_ctrl = isp_read_reg(dev, REG_ADDR(isp_gcmono_ctrl));
	u32 isp_ctrl = isp_read_reg(dev, REG_ADDR(isp_ctrl));

	pr_info("enter %s\n", __func__);
	REG_SET_SLICE(isp_gcmono_ctrl, ISP_GCMONO_CFG_DONE, 1);
	REG_SET_SLICE(isp_gcmono_ctrl, ISP_GCMONO_SWITCH,
		      ISP_GCMONO_SWITCH_ENABLE);
	isp_write_reg(dev, REG_ADDR(isp_gcmono_ctrl), isp_gcmono_ctrl);
	REG_SET_SLICE(isp_ctrl, ISP_GCMONO_MODE, dev->gcmono.mode);
	REG_SET_SLICE(isp_ctrl, ISP_GCMONO_ENABLE, 1);
	isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);
	dev->gcmono.enable = true;
	return 0;
#endif
}

int isp_disable_gcmono(struct isp_ic_dev *dev)
{
#ifndef ISP_GCMONO
	pr_err("unsupported function %s", __func__);
	return -1;
#else
	u32 isp_gcmono_ctrl = isp_read_reg(dev, REG_ADDR(isp_gcmono_ctrl));
	u32 isp_ctrl = isp_read_reg(dev, REG_ADDR(isp_ctrl));

	pr_info("enter %s\n", __func__);
	REG_SET_SLICE(isp_gcmono_ctrl, ISP_GCMONO_SWITCH,
		      ISP_GCMONO_SWITCH_DISABLE);
	isp_write_reg(dev, REG_ADDR(isp_gcmono_ctrl), isp_gcmono_ctrl);
	REG_SET_SLICE(isp_ctrl, ISP_GCMONO_ENABLE, 0);
	isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);
	dev->gcmono.enable = false;
	return 0;
#endif
}

int isp_s_gcmonopx(struct isp_ic_dev *dev, struct isp_gcmono_data *data)
{
#ifndef ISP_GCMONO
	pr_err("unsupported function %s", __func__);
	return -1;
#else
	u32 isp_gc_px_reg = REG_ADDR(isp_gcmono_px_0);
	u32 *p_table = NULL;
	int i;
	u32 gc_px_data = 0;

	pr_info("enter %s\n", __func__);
	p_table = (u32 *)&data->px;
	for (i = 0; i < 64; i++) {
		gc_px_data |= (*(p_table + i) << (i % 6 * 5));
		if (i % 6 == 5 || i == 63) {
			isp_write_reg(dev, isp_gc_px_reg, gc_px_data);
			isp_gc_px_reg += 4;
			gc_px_data = 0;
		}
	}
#endif
}

int isp_s_gcmonoWriteData(struct isp_ic_dev *dev, u32 *tblX, u32 *tblY)
{
#ifndef ISP_GCMONO
	pr_err("unsupported function %s", __func__);
	return -1;
#else
	u32 isp_gc_y_data, isp_gc_x_data;
	u32 *p_table = NULL;
	int i;
	u32 gc_px_data = 0;

	pr_info("enter %s\n", __func__);
	isp_write_reg(dev, REG_ADDR(isp_gcmono_y_addr), 0);
	isp_write_reg(dev, REG_ADDR(isp_gcmono_x_addr), 0);
	for (i = 0; i < 64; i++) {
		isp_gc_y_data = *(tblY + i);
		isp_write_reg(dev, REG_ADDR(isp_gcmono_y_write_data),
			      isp_gc_y_data);
	}
	for (i = 0; i < 63; i++) {
		isp_gc_x_data = *(tblX + i);
		isp_write_reg(dev, REG_ADDR(isp_gcmono_x_write_data),
			      isp_gc_x_data);
	}
#endif
}

int isp_s_gcmono(struct isp_ic_dev *dev, struct isp_gcmono_data *data)
{
#ifndef ISP_GCMONO
	pr_err("unsupported function %s", __func__);
	return -1;
#else
	u32 isp_gcmono_ctrl = isp_read_reg(dev, REG_ADDR(isp_gcmono_ctrl));
	u32 isp_gc_para_base = 0;
	u8 *p_table = NULL;
	int i;

	pr_info("enter %s\n", __func__);
	REG_SET_SLICE(isp_gcmono_ctrl, ISP_GCMONO_SWITCH,
		      ISP_GCMONO_SWITCH_DISABLE);
	REG_SET_SLICE(isp_gcmono_ctrl, ISP_GCMONO_CFG_DONE,
		      ISP_GCMONO_CFG_DONE_SET_CURVE);
	isp_write_reg(dev, REG_ADDR(isp_gcmono_ctrl), isp_gcmono_ctrl);
	p_table = (u8 *)&data->basePara;
	for (i = 0; i < 1024; i++) {
		isp_gc_para_base |= (*(p_table + i) << (i % 4 * 8));
		if (i % 4 == 3) {
			isp_write_reg(dev, REG_ADDR(isp_gcmono_para_base),
				      isp_gc_para_base);
			isp_gc_para_base = 0;
		}
	}
	isp_s_gcmonopx(dev, data);
	isp_s_gcmonoWriteData(dev, data->dataX, data->dataY);
	if (dev->gcmono.enable) {
		isp_enable_gcmono(dev);
	}
#endif
}
