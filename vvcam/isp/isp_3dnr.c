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


void dnr3_hw_init(struct isp_ic_dev *dev)
{
	struct isp_3dnr_context *dnr3 = &dev->dnr3;
	u32 regVal = 0;
	int i, pos;
	/* spacial */
	u32 update_spacial = 900;
	u32 strength_curve_spacial = 64;
	u32 thr_edge_v_inv = 1024;
	u32 thr_edge_h_inv = 1024;
	u32 thr_range_s_inv = 1024;
	/* temperal */
	u32 update_temperal = 900;
	u32 strength_curve_temperal = 64;
	u32 range_t_h = 1;
	u32 range_t_v = 1;
	u32 thr_range_t_inv = 1024;
	u32 thr_motion_inv = 1024;
	u32 range_d = 1;
	u32 thr_delta_h_inv = 1023;
	u32 thr_delta_v_inv = 1023;
	u32 thr_delta_t_inv = 1023;
	u32 strength = dnr3->strength;
	strength = MIN(MAX(strength, 0), 128);

	if (dnr3->init) {
		u32 isp_denoise3d_ctrl = isp_read_reg(dev, REG_ADDR(isp_denoise3d_ctrl));
		REG_SET_SLICE(isp_denoise3d_ctrl, DENOISE3D_HORIZONTAL_EN,
			dnr3->enable_h);
		REG_SET_SLICE(isp_denoise3d_ctrl, DENOISE3D_VERTICAL_EN,
			dnr3->enable_v);
		REG_SET_SLICE(isp_denoise3d_ctrl, DENOISE3D_TEMPERAL_EN,
			dnr3->enable_temperal);
		REG_SET_SLICE(isp_denoise3d_ctrl, DENOISE3D_DILATE_EN,
			dnr3->enable_dilate);
		REG_SET_SLICE(isp_denoise3d_ctrl, DENOISE3D_ENABLE, dnr3->enable);
		isp_write_reg(dev, REG_ADDR(isp_denoise3d_ctrl), isp_denoise3d_ctrl);
	}

	regVal = 0;
	REG_SET_SLICE(regVal, DENOISE3D_STRENGTH_CURVE_SPACIAL,
		      strength_curve_spacial);
	REG_SET_SLICE(regVal, DENOISE3D_THR_EDGE_H_INV, thr_edge_h_inv);
	isp_write_reg(dev, REG_ADDR(isp_denoise3d_edge_h), regVal);
	regVal = 0;
	REG_SET_SLICE(regVal, DENOISE3D_STRENGTH_CURVE_TEMPERAL,
		      strength_curve_temperal);
	REG_SET_SLICE(regVal, DENOISE3D_THR_EDGE_V_INV, thr_edge_v_inv);
	isp_write_reg(dev, REG_ADDR(isp_denoise3d_edge_v), regVal);
	regVal = 0;
	REG_SET_SLICE(regVal, DENOISE3D_RANGE_S_INV, thr_range_s_inv);
	isp_write_reg(dev, REG_ADDR(isp_denoise3d_range_s), regVal);
	regVal = 0;
	REG_SET_SLICE(regVal, DENOISE3D_RANGE_T_H, range_t_h);
	REG_SET_SLICE(regVal, DENOISE3D_RANGE_T_V, range_t_v);
	REG_SET_SLICE(regVal, DENOISE3D_RANGE_T_INV, thr_range_t_inv);
	isp_write_reg(dev, REG_ADDR(isp_denoise3d_range_t), regVal);
	regVal = 0;
	REG_SET_SLICE(regVal, DENOISE3D_RANGE_D, range_d);
	REG_SET_SLICE(regVal, DENOISE3D_MOTION_INV, thr_motion_inv);
	isp_write_reg(dev, REG_ADDR(isp_denoise3d_motion), regVal);
	regVal = 0;
	REG_SET_SLICE(regVal, DENOISE3D_DELTA_H_INV, thr_delta_h_inv);
	REG_SET_SLICE(regVal, DENOISE3D_DELTA_V_INV, thr_delta_v_inv);
	REG_SET_SLICE(regVal, DENOISE3D_DELTA_T_INV, thr_delta_t_inv);
	isp_write_reg(dev, REG_ADDR(isp_denoise3d_delta_inv), regVal);
	/* spacial */

	for (i = 0; i < 6; i++) {
		regVal = 0;
		pos = i * 3;
		if (i < 5) {
			REG_SET_SLICE(regVal, DENOISE3D_SPACIAL_CURVE0,
				      dnr3->spacial_curve[pos + 0]);
			REG_SET_SLICE(regVal, DENOISE3D_SPACIAL_CURVE1,
				      dnr3->spacial_curve[pos + 1]);
			REG_SET_SLICE(regVal, DENOISE3D_SPACIAL_CURVE2,
				      dnr3->spacial_curve[pos + 2]);
		} else {
			REG_SET_SLICE(regVal, DENOISE3D_SPACIAL_CURVE1,
				      dnr3->spacial_curve[pos + 0]);
			REG_SET_SLICE(regVal, DENOISE3D_SPACIAL_CURVE2,
				      dnr3->spacial_curve[pos + 1]);
		}
		isp_write_reg(dev, REG_ADDR(isp_denoise3d_curve_s[i]), regVal);
		regVal = 0;
		if (i < 5) {
			REG_SET_SLICE(regVal, DENOISE3D_TEMPERAL_CURVE0,
				      dnr3->temperal_curve[pos + 0]);
			REG_SET_SLICE(regVal, DENOISE3D_TEMPERAL_CURVE1,
				      dnr3->temperal_curve[pos + 1]);
			REG_SET_SLICE(regVal, DENOISE3D_TEMPERAL_CURVE2,
				      dnr3->temperal_curve[pos + 2]);
		} else {
			REG_SET_SLICE(regVal, DENOISE3D_TEMPERAL_CURVE1,
				      dnr3->temperal_curve[pos + 0]);
			REG_SET_SLICE(regVal, DENOISE3D_TEMPERAL_CURVE2,
				      dnr3->temperal_curve[pos + 1]);
		}
		isp_write_reg(dev, REG_ADDR(isp_denoise3d_curve_t[i]), regVal);
	}

	regVal = 0;
	REG_SET_SLICE(regVal, DENOISE3D_UPDATE_SPACIAL, update_spacial);
	REG_SET_SLICE(regVal, DENOISE3D_UPDATE_TEMPERAL, update_temperal);
	isp_write_reg(dev, REG_ADDR(isp_denoise3d_strength), regVal);
}

int isp_s_3dnr_cmp(struct isp_ic_dev *dev) {
#ifndef ISP_3DNR
	return -1;
#elif !defined(ISP_3DNR_DDR_LESS)
	return -1;
#else
	struct isp_3dnr_compress_context *compress = &dev->dnr3.compress;

	u32 isp_denoise3d_weight1 = 0;
	u32 isp_denoise3d_weight2 = 0;
	int i = 0;
	for (i = 0; i < 4; i++) {
	    isp_denoise3d_weight1 |= (compress->weight_down[i] & DENOISE3D_WEIGHT_MASK) << (3 - i) * 4;
	}
	REG_SET_SLICE(isp_denoise3d_weight1, DENOISE3D_WEIGHT_UP_Y0, compress->weight_up_y[0]);
	REG_SET_SLICE(isp_denoise3d_weight1, DENOISE3D_WEIGHT_UP_Y1, compress->weight_up_y[1]);

	for (i = 0; i < 8; i++) {
	    isp_denoise3d_weight2 |= (compress->weight_up[i] & DENOISE3D_WEIGHT_MASK) << (7 - i) * 4;
	}
	isp_write_reg(dev, REG_ADDR(isp_denoise3d_weight1), isp_denoise3d_weight1);
	isp_write_reg(dev, REG_ADDR(isp_denoise3d_weight2), isp_denoise3d_weight2);

	return 0;
#endif
}

int isp_s_3dnr(struct isp_ic_dev *dev)
{
#ifndef ISP_3DNR
	return -1;
#else
	struct isp_3dnr_context *dnr3 = &dev->dnr3;

	u32 isp_denoise3d_strength, isp_denoise3d_motion, isp_denoise3d_delta_inv;
#ifdef ISP_3DNR_DDR_LESS
	u32 isp_denoise3d_ctrl;
#endif

	pr_info("enter %s\n", __func__);
	if (dnr3->update_bin) {
		dnr3_hw_init(dev);
        }

	isp_denoise3d_strength = isp_read_reg(dev, REG_ADDR(isp_denoise3d_strength));
	if (!dnr3->enable) {
	    REG_SET_SLICE(isp_denoise3d_strength, DENOISE3D_STRENGTH, 0);
	} else {
	    REG_SET_SLICE(isp_denoise3d_strength, DENOISE3D_STRENGTH, dnr3->strength);
	}
	isp_write_reg(dev, REG_ADDR(isp_denoise3d_strength),
		      isp_denoise3d_strength);

	isp_denoise3d_motion =
	    isp_read_reg(dev, REG_ADDR(isp_denoise3d_motion));
        REG_SET_SLICE(isp_denoise3d_motion, DENOISE3D_MOTION_INV, dnr3->motion_factor);
	isp_write_reg(dev, REG_ADDR(isp_denoise3d_motion),
		      isp_denoise3d_motion);
	isp_denoise3d_delta_inv =
	    isp_read_reg(dev, REG_ADDR(isp_denoise3d_delta_inv));
	REG_SET_SLICE(isp_denoise3d_delta_inv, DENOISE3D_DELTA_T_INV, dnr3->delta_factor);
	isp_write_reg(dev, REG_ADDR(isp_denoise3d_delta_inv),
		      isp_denoise3d_delta_inv);

	isp_write_reg(dev, REG_ADDR(isp_denoise3d_dummy_hblank), 0x80);

#ifndef ISP_3DNR_DDR_LESS
     u32 in_width, in_height;
     in_width = isp_read_reg(dev, REG_ADDR(isp_out_h_size);
     in_height = isp_read_reg(dev, REG_ADDR(isp_out_v_size));

     u32 size = in_width * in_height * 2; /* RAW12 */
	    /* update sp2config */
	isp_write_reg(dev, REG_ADDR(miv2_sp2_raw_base_ad_init), dev->dnr3.pa);
	isp_write_reg(dev, REG_ADDR(miv2_sp2_dma_raw_pic_start_ad),
		      dev->dnr3.pa);
	isp_write_reg(dev, REG_ADDR(miv2_sp2_raw_size_init), size);
	isp_write_reg(dev, REG_ADDR(miv2_sp2_raw_offs_cnt_init), 0);
	isp_write_reg(dev, REG_ADDR(miv2_sp2_raw_llength), in_width * 2);
	isp_write_reg(dev, REG_ADDR(miv2_sp2_raw_pic_width), in_width);
	isp_write_reg(dev, REG_ADDR(miv2_sp2_raw_pic_height), in_height);
	isp_write_reg(dev, REG_ADDR(miv2_sp2_raw_pic_size), size);
#else
	isp_s_3dnr_cmp(dev);
	isp_denoise3d_ctrl = isp_read_reg(dev, REG_ADDR(isp_denoise3d_ctrl));
	REG_SET_SLICE(isp_denoise3d_ctrl, DENOISE3D_WRITE_REF_EN, 1);
	isp_write_reg(dev, REG_ADDR(isp_denoise3d_ctrl), isp_denoise3d_ctrl);

#endif
#endif
	return 0;
}

int isp_u_3dnr(struct isp_ic_dev *dev, struct isp_3dnr_update *dnr3_update)
{
#ifndef ISP_3DNR
	pr_err("unsupported function: %s", __func__);
	return -EINVAL;
#else
	u32 regVal = 0;

	pr_info("enter %s\n", __func__);
	regVal = isp_read_reg(dev, REG_ADDR(isp_denoise3d_strength));
	REG_SET_SLICE(regVal, DENOISE3D_STRENGTH, dev->dnr3.strength);
	isp_write_reg(dev, REG_ADDR(isp_denoise3d_strength), regVal);
	regVal = 0;
	regVal = isp_read_reg(dev, REG_ADDR(isp_denoise3d_edge_h));
	REG_SET_SLICE(regVal, DENOISE3D_THR_EDGE_H_INV,
		      dnr3_update->thr_edge_h_inv);
	isp_write_reg(dev, REG_ADDR(isp_denoise3d_edge_h), regVal);
	regVal = 0;
	regVal = isp_read_reg(dev, REG_ADDR(isp_denoise3d_edge_v));
	REG_SET_SLICE(regVal, DENOISE3D_THR_EDGE_V_INV,
		      dnr3_update->thr_edge_v_inv);
	isp_write_reg(dev, REG_ADDR(isp_denoise3d_edge_v), regVal);
	regVal = 0;
	regVal = isp_read_reg(dev, REG_ADDR(isp_denoise3d_range_s));
	REG_SET_SLICE(regVal, DENOISE3D_RANGE_S_INV,
		      dnr3_update->thr_range_s_inv);
	isp_write_reg(dev, REG_ADDR(isp_denoise3d_range_s), regVal);
	regVal = 0;
	regVal = isp_read_reg(dev, REG_ADDR(isp_denoise3d_range_t));
	REG_SET_SLICE(regVal, DENOISE3D_RANGE_T_H, dnr3_update->range_t_h);
	REG_SET_SLICE(regVal, DENOISE3D_RANGE_T_V, dnr3_update->range_t_v);
	REG_SET_SLICE(regVal, DENOISE3D_RANGE_T_INV,
		      dnr3_update->thr_range_t_inv);
	isp_write_reg(dev, REG_ADDR(isp_denoise3d_range_t), regVal);
	regVal = 0;
	regVal = isp_read_reg(dev, REG_ADDR(isp_denoise3d_motion));
	REG_SET_SLICE(regVal, DENOISE3D_RANGE_D, dnr3_update->range_d);
	REG_SET_SLICE(regVal, DENOISE3D_MOTION_INV,
		      dnr3_update->thr_motion_inv);
	isp_write_reg(dev, REG_ADDR(isp_denoise3d_motion), regVal);
	regVal = 0;
	regVal = isp_read_reg(dev, REG_ADDR(isp_denoise3d_delta_inv));
	REG_SET_SLICE(regVal, DENOISE3D_DELTA_H_INV,
		      dnr3_update->thr_delta_h_inv);
	REG_SET_SLICE(regVal, DENOISE3D_DELTA_V_INV,
		      dnr3_update->thr_delta_v_inv);
	REG_SET_SLICE(regVal, DENOISE3D_DELTA_T_INV,
		      dnr3_update->thr_delta_t_inv);
	isp_write_reg(dev, REG_ADDR(isp_denoise3d_delta_inv), regVal);

	return 0;
#endif
}

int isp_g_3dnr(struct isp_ic_dev *dev, u32 * avg)
{
	if (!dev || !avg) {
		return -EINVAL;
	}
	*avg = isp_read_reg(dev, REG_ADDR(isp_denoise3d_average));
	return 0;
}

int isp_r_3dnr(struct isp_ic_dev *dev)
{
#ifndef ISP_3DNR
	return -1;
#else
	u32 in_width, in_height;
	u32 size;

	u32 miv2_ctrl = isp_read_reg(dev, REG_ADDR(miv2_ctrl));
	u32 miv2_imsc = isp_read_reg(dev, REG_ADDR(miv2_imsc));
	u32 miv2_sp2_ctrl = isp_read_reg(dev, REG_ADDR(miv2_sp2_ctrl));

	in_width = isp_read_reg(dev, REG_ADDR(isp_out_h_size));
	in_height = isp_read_reg(dev, REG_ADDR(isp_out_v_size));
	size = in_width * in_height * 2;	/* RAW12 */

	REG_SET_SLICE(miv2_ctrl, SP2_RAW_RDMA_PATH_ENABLE, 1);
	REG_SET_SLICE(miv2_ctrl, SP2_RAW_PATH_ENABLE, 1);
	isp_write_reg(dev, REG_ADDR(miv2_sp2_raw_base_ad_init), dev->dnr3.pa);
	isp_write_reg(dev, REG_ADDR(miv2_sp2_dma_raw_pic_start_ad),
		      dev->dnr3.pa);
	isp_write_reg(dev, REG_ADDR(miv2_sp2_dma_raw_pic_width), in_width);
	isp_write_reg(dev, REG_ADDR(miv2_sp2_dma_raw_pic_llength),
		      in_width * 2);
	isp_write_reg(dev, REG_ADDR(miv2_sp2_dma_raw_pic_lval), in_width * 2);

	isp_write_reg(dev, REG_ADDR(miv2_sp2_dma_raw_pic_size), size);
	REG_SET_SLICE(miv2_sp2_ctrl, SP2_RD_RAW_CFG_UPDATE, 1);
	REG_SET_SLICE(miv2_sp2_ctrl, SP2_RD_RAW_AUTO_UPDATE, 1);
#if 1	/* ndef ISP8000_V1901 */
	REG_SET_SLICE(miv2_sp2_ctrl, SP2_MI_CFG_UPD, 1);
#endif
	miv2_imsc |= SP2_DMA_RAW_READY_MASK;
	isp_write_reg(dev, REG_ADDR(miv2_sp2_ctrl), miv2_sp2_ctrl);
	isp_write_reg(dev, REG_ADDR(miv2_ctrl), miv2_ctrl);
	isp_write_reg(dev, REG_ADDR(miv2_imsc), miv2_imsc);
	return 0;
#endif
}

