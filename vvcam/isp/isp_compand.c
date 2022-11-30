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

int isp_s_comp(struct isp_ic_dev *dev)
{
#ifndef ISP_COMPAND
	pr_err("unsupported function: %s", __func__);
	return -EINVAL;
#else
	struct isp_comp_context *comp = &dev->comp;
	int ri, valr;
	uint32_t x_data;
	uint32_t y_data;
	u32 isp_compand_ctrl = isp_read_reg(dev, REG_ADDR(isp_compand_ctrl));

	pr_info("enter %s\n", __func__);

	if (!comp->enable) {
		REG_SET_SLICE(isp_compand_ctrl, COMPAND_CTRL_BLS_ENABLE, 0);
		REG_SET_SLICE(isp_compand_ctrl, COMPAND_CTRL_COMPRESS_ENABLE, 0);
		REG_SET_SLICE(isp_compand_ctrl, COMPAND_CTRL_EXPAND_ENABLE, 0);
		isp_write_reg(dev, REG_ADDR(isp_compand_ctrl), isp_compand_ctrl);
		return 0;
	}

	if (comp->bls.enable) {
		isp_write_reg(dev, REG_ADDR(isp_compand_bls_a_fixed), comp->bls.a << (20 - comp->bls.bit_width));
		isp_write_reg(dev, REG_ADDR(isp_compand_bls_b_fixed), comp->bls.b << (20 - comp->bls.bit_width));
		isp_write_reg(dev, REG_ADDR(isp_compand_bls_c_fixed), comp->bls.c << (20 - comp->bls.bit_width));
		isp_write_reg(dev, REG_ADDR(isp_compand_bls_d_fixed), comp->bls.d << (20 - comp->bls.bit_width));
	}

	if (comp->expand.enable && comp->expand.update_curve) {
		x_data = 0;
		for (ri = 0; ri < 11; ri++) {
			valr = 0;

			x_data += 1 << comp->expand.px[ri * 6 + 0];
			if (x_data <= (1 << comp->expand.in_bit)) {
				valr |= comp->expand.px[ri * 6 + 0] + (20-comp->expand.in_bit);
			}

			x_data += 1 << comp->expand.px[ri * 6 + 1];
			if (x_data <= (1 << comp->expand.in_bit)) {
				valr |=  ((comp->expand.px[ri * 6 + 1] + (20-comp->expand.in_bit)) << 5);
			}

			x_data += 1 << comp->expand.px[ri * 6 + 2];
			if (x_data <= (1 << comp->expand.in_bit)) {
				valr |=  ((comp->expand.px[ri * 6 + 2] + (20-comp->expand.in_bit)) << 10);
			}

			x_data += 1 << comp->expand.px[ri * 6 + 3];
			if (x_data <= (1 << comp->expand.in_bit)) {
				valr |=  ((comp->expand.px[ri * 6 + 3] + (20-comp->expand.in_bit)) << 15);
			}

			if (ri != 10) {
				x_data += 1 << comp->expand.px[ri * 6 + 4];
				if (x_data <= (1 << comp->expand.in_bit)) {
					valr |=  ((comp->expand.px[ri * 6 + 4] + (20-comp->expand.in_bit)) << 20);
				}

				x_data += 1 << comp->expand.px[ri * 6 + 5];
				if (x_data <= (1 << comp->expand.in_bit)) {
					valr |=  ((comp->expand.px[ri * 6 + 5] + (20-comp->expand.in_bit)) << 25);
				}
			}

			isp_write_reg(dev, REG_ADDR(isp_compand_expand_px_0) + ri * 4, valr);
		}

		isp_write_reg(dev, REG_ADDR(isp_compand_expand_x_addr), 0x0);
		for (ri = 1; ri < 65; ri++) {
			x_data = (comp->expand.x_data[ri] << (20 - comp->expand.in_bit));
			if (x_data > 0xfffff)
				x_data = 0xfffff;
			isp_write_reg(dev, REG_ADDR(isp_compand_expand_x_write_data), x_data);
		}

		isp_write_reg(dev, REG_ADDR(isp_compand_expand_y_addr), 0x0);
		for (ri = 1; ri < 65; ri++) {
			y_data = (comp->expand.y_data[ri] << (20 - comp->expand.out_bit));
			if (y_data > 0xfffff)
				y_data = 0xfffff;
			isp_write_reg(dev, REG_ADDR(isp_compand_expand_y_write_data), y_data);
		}
	
	}

	if (comp->compress.enable && comp->compress.update_curve) {
		x_data = 0;
		for (ri = 0; ri < 11; ri++) {
			valr = 0;
			x_data += 1 << (comp->compress.px[ri * 6 + 0]);
			if (x_data <= (1 << comp->compress.in_bit)) {
				valr |= (comp->compress.px[ri * 6 + 0] + (20-comp->compress.in_bit));
			}

			x_data += 1 << (comp->compress.px[ri * 6 + 1]);
			if (x_data <= (1 << comp->compress.in_bit)) {
				valr |= ((comp->compress.px[ri * 6 + 1] + (20-comp->compress.in_bit)) << 5);
			}

			x_data += 1 << (comp->compress.px[ri * 6 + 2]);
			if (x_data <= (1 << comp->compress.in_bit)) {
				valr |= ((comp->compress.px[ri * 6 + 2] + (20-comp->compress.in_bit)) << 10);
			}

			x_data += 1 << (comp->compress.px[ri * 6 + 3]);
			if (x_data <= (1 << comp->compress.in_bit)) {
				valr |= ((comp->compress.px[ri * 6 + 3] + (20-comp->compress.in_bit)) << 15);
			}

			if (ri != 10) {
				x_data += 1 << (comp->compress.px[ri * 6 + 4]);
				if (x_data <= (1 << comp->compress.in_bit)) {
					valr |= ((comp->compress.px[ri * 6 + 4] + (20-comp->compress.in_bit)) << 20);
				}

				x_data += 1 << (comp->compress.px[ri * 6 + 5]);
				if (x_data <= (1 << comp->compress.in_bit)) {
					valr |= ((comp->compress.px[ri * 6 + 5] + (20-comp->compress.in_bit)) << 25);
				}
			}

			isp_write_reg(dev, REG_ADDR(isp_compand_compress_px_0) + ri * 4, valr);
		}

		isp_write_reg(dev, REG_ADDR(isp_compand_compress_x_addr), 0x0);
		for (ri = 1; ri < 65; ri++) {
			x_data = (comp->compress.x_data[ri] << (20 - comp->compress.in_bit));
			if (x_data > 0xfffff)
				x_data = 0xfffff;
			isp_write_reg(dev, REG_ADDR(isp_compand_compress_x_write_data), x_data);
		}

		isp_write_reg(dev, REG_ADDR(isp_compand_compress_y_addr), 0x0);
		for (ri = 1; ri < 65; ri++) {
			y_data = (comp->compress.y_data[ri] << (12 - comp->compress.out_bit));
			if (y_data > 0xfff)
				y_data = 0xfff;
			isp_write_reg(dev, REG_ADDR(isp_compand_compress_y_write_data), y_data);
		}
	}

	REG_SET_SLICE(isp_compand_ctrl, COMPAND_CTRL_BLS_ENABLE, comp->bls.enable);
	REG_SET_SLICE(isp_compand_ctrl, COMPAND_CTRL_COMPRESS_ENABLE, comp->compress.enable);
	REG_SET_SLICE(isp_compand_ctrl, COMPAND_CTRL_EXPAND_ENABLE, comp->expand.enable);
	isp_write_reg(dev, REG_ADDR(isp_compand_ctrl), isp_compand_ctrl);

	return 0;
#endif
}
