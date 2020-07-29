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
#include <linux/io.h>
#include <linux/module.h>
#endif
#include "isp_ioctl.h"
#include "mrv_all_bits.h"
#include "isp_types.h"

extern MrvAllRegister_t *all_regs;

u32 convertBlsValue(int val)
{
	int neg = val < 0;

	if (neg)
		val = 4095 + val;
	return (val << 8) | (neg << 20);
}

int isp_s_comp(struct isp_ic_dev *dev)
{
#ifndef ISP_COMPAND
	pr_err("unsupported function: %s", __func__);
	return -EINVAL;
#else
	struct isp_comp_context *comp = &dev->comp;
	int ri, valr, valy;
	u32 isp_compand_ctrl = isp_read_reg(dev, REG_ADDR(isp_compand_ctrl));

	pr_info("enter %s\n", __func__);

	if (!comp->enable) {
		REG_SET_SLICE(isp_compand_ctrl, COMPAND_CTRL_BLS_ENABLE, 0);
		REG_SET_SLICE(isp_compand_ctrl, COMPAND_CTRL_COMPRESS_ENABLE,
			      0);
		REG_SET_SLICE(isp_compand_ctrl, COMPAND_CTRL_COMPRESS_ENABLE,
			      0);
		isp_write_reg(dev, REG_ADDR(isp_compand_ctrl),
			      isp_compand_ctrl);
		return 0;
	}
	if (comp->bls) {
		isp_write_reg(dev, REG_ADDR(isp_compand_bls_a_fixed),
			      convertBlsValue(comp->a));
		isp_write_reg(dev, REG_ADDR(isp_compand_bls_b_fixed),
			      convertBlsValue(comp->b));
		isp_write_reg(dev, REG_ADDR(isp_compand_bls_c_fixed),
			      convertBlsValue(comp->c));
		isp_write_reg(dev, REG_ADDR(isp_compand_bls_d_fixed),
			      convertBlsValue(comp->d));
	}
	if (comp->expand && comp->update_tbl) {
		for (ri = 0; ri < 10; ri++) {
			valr = comp->expand_tbl[ri * 6 + 0] |
			    (comp->expand_tbl[ri * 6 + 1] << 5) |
			    (comp->expand_tbl[ri * 6 + 2] << 10) |
			    (comp->expand_tbl[ri * 6 + 3] << 15) |
			    (comp->expand_tbl[ri * 6 + 4] << 20) |
			    (comp->expand_tbl[ri * 6 + 5] << 25);
			isp_write_reg(dev,
				      REG_ADDR(isp_compand_expand_px_0) +
				      ri * 4, valr);
		}
		valr = comp->expand_tbl[ri * 6 + 0] |
		    (comp->expand_tbl[ri * 6 + 1] << 5) |
		    (comp->expand_tbl[ri * 6 + 2] << 10) |
		    (comp->expand_tbl[ri * 6 + 3] << 15);
		isp_write_reg(dev, REG_ADDR(isp_compand_expand_px_0) + ri * 4,
			      valr);
		isp_write_reg(dev, REG_ADDR(isp_compand_expand_x_addr), 0);
		valr = 0;
		for (ri = 0; ri < 64; ri++) {
			valr += (1 << comp->expand_tbl[ri]);
			if (valr > 0xfffff)
				valr = 0xfffff;
			isp_write_reg(dev,
				      REG_ADDR(isp_compand_expand_x_write_data),
				      valr);
		}
		isp_write_reg(dev, REG_ADDR(isp_compand_expand_y_addr), 0);
		valr = 0;
		for (ri = 0; ri < 64; ri++) {
			/* 12bit->16bit for ov2775 */
			if (comp->expand_tbl[ri] == 0) {
				valr += (1 << comp->expand_tbl[ri]);
			} else {
				valr += (1 << (comp->expand_tbl[ri] - 8));
			}
			if (valr <= 512) {
				valy = 2 * valr;
			} else if (valr <= 768) {
				valy = 4 * (valr - 256);
			} else if (valr <= 2560) {
				valy = 8 * (valr - 512);
			} else if (valr <= 4096) {
				valy = 32 * (valr - 2048);
			} else {
				valy = 65535;
			}
			if (valy > 65535) {
				valy = 65535;
			}
			isp_write_reg(dev,
				      REG_ADDR(isp_compand_expand_y_write_data),
				      valy << 4);
		}
	}

	if (comp->compress && comp->update_tbl) {
		for (ri = 0; ri < 10; ri++) {
			valr = comp->compress_tbl[ri * 6 + 0] |
			    (comp->compress_tbl[ri * 6 + 1] << 5) |
			    (comp->compress_tbl[ri * 6 + 2] << 10) |
			    (comp->compress_tbl[ri * 6 + 3] << 15) |
			    (comp->compress_tbl[ri * 6 + 4] << 20) |
			    (comp->compress_tbl[ri * 6 + 5] << 25);
			isp_write_reg(dev,
				      REG_ADDR(isp_compand_compress_px_0) +
				      ri * 4, valr);
		}
		valr = comp->compress_tbl[ri * 6 + 0] |
		    (comp->compress_tbl[ri * 6 + 1] << 5) |
		    (comp->compress_tbl[ri * 6 + 2] << 10) |
		    (comp->compress_tbl[ri * 6 + 3] << 15);
		isp_write_reg(dev, REG_ADDR(isp_compand_compress_px_0) + ri * 4,
			      valr);
		isp_write_reg(dev, REG_ADDR(isp_compand_compress_x_addr), 0);
		valr = 0;
		for (ri = 0; ri < 64; ri++) {
			valr += (1 << comp->compress_tbl[ri]);
			if (valr > 0xfffff)
				valr = 0xfffff;
			isp_write_reg(dev,
				      REG_ADDR
				      (isp_compand_compress_x_write_data),
				      valr);
		}
		isp_write_reg(dev, REG_ADDR(isp_compand_compress_y_addr), 0);
		valr = 0;
		for (ri = 0; ri < 64; ri++) {
			/* 16bit->12bit for ov2775 */
			if (comp->compress_tbl[ri] == 0) {
				valr += (1 << comp->compress_tbl[ri]);
			} else {
				valr += (1 << (comp->compress_tbl[ri] - 4));
			}
			if (valr <= 1024) {
				valy = valr >> 1;
			} else if (valr <= 2048) {
				valy = (valr >> 2) + 256;
			} else if (valr <= 16384) {
				valy = (valr >> 3) + 512;
			} else if (valr <= 65536) {
				valy = (valr >> 5) + 2048;
			} else {
				valy = 4095;
			}
			if (valy > 4095) {
				valy = 4095;
			}
			isp_write_reg(dev,
				      REG_ADDR
				      (isp_compand_compress_y_write_data),
				      valy);
		}
	}

	REG_SET_SLICE(isp_compand_ctrl, COMPAND_CTRL_BLS_ENABLE, comp->bls);
	REG_SET_SLICE(isp_compand_ctrl, COMPAND_CTRL_COMPRESS_ENABLE,
		      comp->compress);
	REG_SET_SLICE(isp_compand_ctrl, COMPAND_CTRL_EXPAND_ENABLE,
		      comp->expand);
	isp_write_reg(dev, REG_ADDR(isp_compand_ctrl), isp_compand_ctrl);

	return 0;
#endif
}
