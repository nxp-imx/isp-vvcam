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

int isp_s_dpf(struct isp_ic_dev *dev)
{
	struct isp_dpf_context *dpf = &dev->dpf;
	u32 value;
	int i = 0;
	u32 isp_ctrl = isp_read_reg(dev, REG_ADDR(isp_ctrl));
	u32 isp_dpf_mode = isp_read_reg(dev, REG_ADDR(isp_dpf_mode));

	if (!dpf->enable) {
		isp_write_reg(dev, REG_ADDR(isp_dpf_mode),
			      isp_dpf_mode & ~MRV_DPF_DPF_ENABLE_MASK);
		return 0;
	}

	isp_dpf_mode &=
	    (MRV_DPF_DPF_ENABLE_MASK | MRV_DPF_NLL_SEGMENTATION_MASK);

	switch (dpf->gain_usage) {
	case IC_DPF_GAIN_USAGE_DISABLED:
		REG_SET_SLICE(isp_dpf_mode, MRV_DPF_USE_NF_GAIN, 0);
		REG_SET_SLICE(isp_dpf_mode, MRV_DPF_LSC_GAIN_COMP, 0);
		REG_SET_SLICE(isp_dpf_mode, MRV_DPF_AWB_GAIN_COMP, 0);
		break;
	case IC_DPF_GAIN_USAGE_NF_GAINS:
		REG_SET_SLICE(isp_dpf_mode, MRV_DPF_USE_NF_GAIN, 1);
		REG_SET_SLICE(isp_dpf_mode, MRV_DPF_LSC_GAIN_COMP, 0);
		REG_SET_SLICE(isp_dpf_mode, MRV_DPF_AWB_GAIN_COMP, 1);
		break;
	case IC_DPF_GAIN_USAGE_LSC_GAINS:
		REG_SET_SLICE(isp_dpf_mode, MRV_DPF_USE_NF_GAIN, 0);
		REG_SET_SLICE(isp_dpf_mode, MRV_DPF_LSC_GAIN_COMP, 1);
		REG_SET_SLICE(isp_dpf_mode, MRV_DPF_AWB_GAIN_COMP, 0);
		break;
	case IC_DPF_GAIN_USAGE_NF_LSC_GAINS:
		REG_SET_SLICE(isp_dpf_mode, MRV_DPF_USE_NF_GAIN, 1);
		REG_SET_SLICE(isp_dpf_mode, MRV_DPF_LSC_GAIN_COMP, 1);
		REG_SET_SLICE(isp_dpf_mode, MRV_DPF_AWB_GAIN_COMP, 1);
		break;
	case IC_DPF_GAIN_USAGE_AWB_GAINS:
		REG_SET_SLICE(isp_dpf_mode, MRV_DPF_USE_NF_GAIN, 0);
		REG_SET_SLICE(isp_dpf_mode, MRV_DPF_LSC_GAIN_COMP, 0);
		REG_SET_SLICE(isp_dpf_mode, MRV_DPF_AWB_GAIN_COMP, 1);
		break;
	case IC_DPF_GAIN_USAGE_AWB_LSC_GAINS:
		REG_SET_SLICE(isp_dpf_mode, MRV_DPF_USE_NF_GAIN, 0);
		REG_SET_SLICE(isp_dpf_mode, MRV_DPF_LSC_GAIN_COMP, 1);
		REG_SET_SLICE(isp_dpf_mode, MRV_DPF_AWB_GAIN_COMP, 1);
		break;
	default:
		pr_err("%s: unsupported gain usage\n", __func__);
		break;
	}

	switch (dpf->filter_type) {
	case IC_DPF_RB_FILTERSIZE_13x9:
		REG_SET_SLICE(isp_dpf_mode, MRV_DPF_RB_FILTER_SIZE, 0U);
		break;
	case IC_DPF_RB_FILTERSIZE_9x9:
		REG_SET_SLICE(isp_dpf_mode, MRV_DPF_RB_FILTER_SIZE, 1U);
		break;
	default:
		pr_err
		    ("%s: unsupported filter kernel size for red/blue pixel\n",
		     __func__);
		break;
	}

	REG_SET_SLICE(isp_dpf_mode, MRV_DPF_R_FILTER_OFF, dpf->filter_r_off);
	REG_SET_SLICE(isp_dpf_mode, MRV_DPF_GR_FILTER_OFF, dpf->filter_gr_off);
	REG_SET_SLICE(isp_dpf_mode, MRV_DPF_GB_FILTER_OFF, dpf->filter_gb_off);
	REG_SET_SLICE(isp_dpf_mode, MRV_DPF_B_FILTER_OFF, dpf->filter_b_off);
	value = 0;
	REG_SET_SLICE(value, MRV_DPF_S_WEIGHT_G1, dpf->weight_g[0]);
	REG_SET_SLICE(value, MRV_DPF_S_WEIGHT_G2, dpf->weight_g[1]);
	REG_SET_SLICE(value, MRV_DPF_S_WEIGHT_G3, dpf->weight_g[2]);
	REG_SET_SLICE(value, MRV_DPF_S_WEIGHT_G4, dpf->weight_g[3]);
	isp_write_reg(dev, REG_ADDR(isp_dpf_s_weight_g_1_4), value);
	value = 0;
	REG_SET_SLICE(value, MRV_DPF_S_WEIGHT_G5, dpf->weight_g[4]);
	REG_SET_SLICE(value, MRV_DPF_S_WEIGHT_G6, dpf->weight_g[5]);
	isp_write_reg(dev, REG_ADDR(isp_dpf_s_weight_g_5_6), value);
	value = 0;
	REG_SET_SLICE(value, MRV_DPF_S_WEIGHT_RB1, dpf->weight_rb[0]);
	REG_SET_SLICE(value, MRV_DPF_S_WEIGHT_RB2, dpf->weight_rb[1]);
	REG_SET_SLICE(value, MRV_DPF_S_WEIGHT_RB3, dpf->weight_rb[2]);
	REG_SET_SLICE(value, MRV_DPF_S_WEIGHT_RB4, dpf->weight_rb[3]);
	isp_write_reg(dev, REG_ADDR(isp_dpf_s_weight_rb_1_4), value);
	value = 0;
	REG_SET_SLICE(value, MRV_DPF_S_WEIGHT_RB5, dpf->weight_rb[4]);
	REG_SET_SLICE(value, MRV_DPF_S_WEIGHT_RB6, dpf->weight_rb[5]);
	isp_write_reg(dev, REG_ADDR(isp_dpf_s_weight_rb_5_6), value);
	isp_write_reg(dev, REG_ADDR(isp_dpf_nf_gain_r), dpf->nf_gain_r);
	isp_write_reg(dev, REG_ADDR(isp_dpf_nf_gain_gr), dpf->nf_gain_gr);
	isp_write_reg(dev, REG_ADDR(isp_dpf_nf_gain_gb), dpf->nf_gain_gb);
	isp_write_reg(dev, REG_ADDR(isp_dpf_nf_gain_b), dpf->nf_gain_b);
	isp_write_reg(dev, REG_ADDR(isp_dpf_strength_r),
		      (MRV_DPF_INV_WEIGHT_R_MASK & dpf->strength_r));
	isp_write_reg(dev, REG_ADDR(isp_dpf_strength_g),
		      (MRV_DPF_INV_WEIGHT_G_MASK & dpf->strength_g));
	isp_write_reg(dev, REG_ADDR(isp_dpf_strength_b),
		      (MRV_DPF_INV_WEIGHT_B_MASK & dpf->strength_b));

	for (i = 0; i < 17; i++) {
		if (dpf->denoise_talbe[i] <= MRV_DPF_NLL_COEFF_N_MASK) {
			isp_write_reg(dev,
				      REG_ADDR(nlf_lookup_table_block_arr[i]),
				      dpf->denoise_talbe[i]);
		}
	}

	switch (dpf->x_scale) {
	case IC_NLL_SCALE_LINEAR:
		REG_SET_SLICE(isp_dpf_mode, MRV_DPF_NLL_SEGMENTATION, 0);
		break;
	case IC_NLL_SCALE_LOGARITHMIC:
		REG_SET_SLICE(isp_dpf_mode, MRV_DPF_NLL_SEGMENTATION, 1);
		break;
	default:
		break;
	}

	isp_write_reg(dev, REG_ADDR(isp_dpf_mode), isp_dpf_mode);
	isp_write_reg(dev, REG_ADDR(isp_dpf_mode),
		      isp_dpf_mode | MRV_DPF_DPF_ENABLE_MASK);
        REG_SET_SLICE(isp_ctrl, MRV_ISP_ISP_GEN_CFG_UPD, 1);
        isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);
	return 0;
}
