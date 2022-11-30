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
#ifdef ISP_DEMOSAIC2
int isp_enable_dmsc(struct isp_ic_dev *dev)
{
	u32 isp_dmsc_ctrl = isp_read_reg(dev, REG_ADDR(isp_dmsc_ctrl));
	u32 isp_dmsc_size_ctrl =
	    isp_read_reg(dev, REG_ADDR(isp_dmsc_size_ctrl));
	pr_info("enter %s\n", __func__);
	REG_SET_SLICE(isp_dmsc_ctrl, ISP_DEMOSAIC_BYPASS, 0U);
	REG_SET_SLICE(isp_dmsc_size_ctrl, ISP_DMSC_IMAGE_H_SIZE,
		      isp_read_reg(dev, REG_ADDR(isp_out_h_size)));
	REG_SET_SLICE(isp_dmsc_size_ctrl, ISP_DMSC_IMAGE_H_BLANK, 0x039c);
	isp_write_reg(dev, REG_ADDR(isp_dmsc_size_ctrl), isp_dmsc_size_ctrl);
	isp_write_reg(dev, REG_ADDR(isp_dmsc_ctrl), isp_dmsc_ctrl);
	isp_write_reg(dev, REG_ADDR(isp_dmsc_ctrl_shd), isp_dmsc_ctrl);
	return 0;
}

int isp_disable_dmsc(struct isp_ic_dev *dev)
{
	u32 isp_dmsc_ctrl = isp_read_reg(dev, REG_ADDR(isp_dmsc_ctrl));
	isp_dmsc_ctrl = 0;	/* clear fpga default bit 3 and 13 to keep the same value with cmodel. */

	pr_info("enter %s\n", __func__);
	REG_SET_SLICE(isp_dmsc_ctrl, ISP_DEMOSAIC_BYPASS, 1U);

	/*clear the reg default val to keep weith cmodel  */
	isp_write_reg(dev, REG_ADDR(isp_dmsc_shap_fact), 0);
	isp_write_reg(dev, REG_ADDR(isp_dmsc_shap_clip), 0);
	isp_write_reg(dev, REG_ADDR(isp_dmsc_shap_filt2), 0);
	isp_write_reg(dev, REG_ADDR(isp_dmsc_dpul_ctrl), 0);
	isp_write_reg(dev, REG_ADDR(isp_dmsc_skin_thr_cb), 0);
	isp_write_reg(dev, REG_ADDR(isp_dmsc_cac_a), 0);
	isp_write_reg(dev, REG_ADDR(isp_dmsc_cac_c), 0);
	isp_write_reg(dev, REG_ADDR(isp_dmsc_dmoi_ctrl), 0);
	isp_write_reg(dev, REG_ADDR(isp_dmsc_dmoi_thr), 0);
	isp_write_reg(dev, REG_ADDR(isp_dmsc_skin_thr_cr), 0);
	isp_write_reg(dev, REG_ADDR(isp_dmsc_skin_thr_y), 0);
	isp_write_reg(dev, REG_ADDR(isp_dmsc_ctrl), isp_dmsc_ctrl);
	return 0;
}

int isp_set_dmsc_intp(struct isp_ic_dev *dev)
{
	u32 isp_dmsc_intp_thr = isp_read_reg(dev, REG_ADDR(isp_dmsc_intp_thr));
	pr_info("enter %s\n", __func__);
	REG_SET_SLICE(isp_dmsc_intp_thr, ISP_DMSC_INTERPLATION_DIR_THR_MAX,
		      dev->demosaic.intp_cxt.intp_dir_thr_max);
	REG_SET_SLICE(isp_dmsc_intp_thr, ISP_DMSC_INTERPLATION_DIR_THR_MIN,
		      dev->demosaic.intp_cxt.intp_dir_thr_min);
	isp_write_reg(dev, REG_ADDR(isp_dmsc_intp_thr), isp_dmsc_intp_thr);
	return 0;
}

int isp_set_dmsc_dmoi(struct isp_ic_dev *dev)
{

	struct isp_dmoi_context *pDemoir = &dev->demosaic.dmoi_cxt;
	u32 isp_dmsc_dmoi_ctrl =
	    isp_read_reg(dev, REG_ADDR(isp_dmsc_dmoi_ctrl));
	u32 isp_dmsc_dmoi_thr = isp_read_reg(dev, REG_ADDR(isp_dmsc_dmoi_thr));
	u32 isp_dmsc_dmoi_patn_thr =
	    isp_read_reg(dev, REG_ADDR(isp_dmsc_dmoi_patn_thr));
	u32 isp_dmsc_ctrl = isp_read_reg(dev, REG_ADDR(isp_dmsc_ctrl));

	pr_info("enter %s\n", __func__);

	if (!pDemoir->enable) {
		REG_SET_SLICE(isp_dmsc_ctrl, ISP_DEMOSAIC_DEMOIRE_ENABLE, 0U);
		isp_write_reg(dev, REG_ADDR(isp_dmsc_ctrl), isp_dmsc_ctrl);
		return 0;
	}

	REG_SET_SLICE(isp_dmsc_dmoi_ctrl, ISP_DMSC_DEMOIRE_AREA_THR,
		      pDemoir->demoire_area_thr);
	REG_SET_SLICE(isp_dmsc_dmoi_ctrl, ISP_DMSC_DEMOIRE_SAT_SHRINK,
		      pDemoir->demoire_sat_shrink);
	REG_SET_SLICE(isp_dmsc_dmoi_thr, ISP_DMSC_DEMOIRE_R2,
		      pDemoir->demoire_r2);
	REG_SET_SLICE(isp_dmsc_dmoi_thr, ISP_DMSC_DEMOIRE_R1,
		      pDemoir->demoire_r1);
	REG_SET_SLICE(isp_dmsc_dmoi_thr, ISP_DMSC_DEMOIRE_T2_SHIFT,
		      pDemoir->demoire_t2_shift);
	REG_SET_SLICE(isp_dmsc_dmoi_thr, ISP_DMSC_DEMOIRE_T1,
		      pDemoir->demoire_t1);
	REG_SET_SLICE(isp_dmsc_dmoi_patn_thr, ISP_DMSC_DEMOIRE_EDGE_R2,
		      pDemoir->demoire_edge_r2);
	REG_SET_SLICE(isp_dmsc_dmoi_patn_thr, ISP_DMSC_DEMOIRE_EDGE_R1,
		      pDemoir->demoire_edge_r1);
	REG_SET_SLICE(isp_dmsc_dmoi_patn_thr, ISP_DMSC_DEMOIRE_EDGE_T2_SHIFT,
		      pDemoir->demoire_edge_t2_shift);
	REG_SET_SLICE(isp_dmsc_dmoi_patn_thr, ISP_DMSC_DEMOIRE_EDGE_T1,
		      pDemoir->demoire_edge_t1);
	isp_write_reg(dev, REG_ADDR(isp_dmsc_dmoi_ctrl), isp_dmsc_dmoi_ctrl);
	isp_write_reg(dev, REG_ADDR(isp_dmsc_dmoi_thr), isp_dmsc_dmoi_thr);
	isp_write_reg(dev, REG_ADDR(isp_dmsc_dmoi_patn_thr),
		      isp_dmsc_dmoi_patn_thr);
	REG_SET_SLICE(isp_dmsc_ctrl, ISP_DEMOSAIC_DEMOIRE_ENABLE, 1U);
	isp_write_reg(dev, REG_ADDR(isp_dmsc_ctrl), isp_dmsc_ctrl);
	return 0;
}

int isp_set_dmsc_sharpen(struct isp_ic_dev *dev)
{

	struct isp_shap_context *pSharpen = &dev->demosaic.shap_cxt;
	u32 isp_dmsc_shap_fact =
	    isp_read_reg(dev, REG_ADDR(isp_dmsc_shap_fact));
	u32 isp_dmsc_shap_clip =
	    isp_read_reg(dev, REG_ADDR(isp_dmsc_shap_clip));
	u32 isp_dmsc_shap_thr = isp_read_reg(dev, REG_ADDR(isp_dmsc_shap_thr));
	u32 isp_dmsc_shap_ratio =
	    isp_read_reg(dev, REG_ADDR(isp_dmsc_shap_ratio));
	u32 isp_dmsc_shap_line_ctrl =
	    isp_read_reg(dev, REG_ADDR(isp_dmsc_shap_line_ctrl));
	u32 isp_dmsc_shap_line_ratio =
	    isp_read_reg(dev, REG_ADDR(isp_dmsc_shap_line_ratio));
	u32 isp_dmsc_shap_filt1 =
	    isp_read_reg(dev, REG_ADDR(isp_dmsc_shap_filt1));
	u32 isp_dmsc_shap_filt2 =
	    isp_read_reg(dev, REG_ADDR(isp_dmsc_shap_filt2));
	u32 isp_dmsc_ctrl = isp_read_reg(dev, REG_ADDR(isp_dmsc_ctrl));

	pr_info("enter %s\n", __func__);

	if (!pSharpen->enabled) {
		REG_SET_SLICE(isp_dmsc_ctrl, ISP_DEMOSAIC_SHARPEN_ENBALE, 0);
		isp_write_reg(dev, REG_ADDR(isp_dmsc_ctrl), isp_dmsc_ctrl);
		isp_write_reg(dev, REG_ADDR(isp_dmsc_ctrl_shd), isp_dmsc_ctrl);
		return 0;
	}

	REG_SET_SLICE(isp_dmsc_shap_fact, ISP_DMSC_SHARPEN_FACTOR_BLACK,
		      pSharpen->sharpen_factor_black);
	REG_SET_SLICE(isp_dmsc_shap_fact, ISP_DMSC_SHARPEN_FACTOR_WHITE,
		      pSharpen->sharpen_factor_white);
	REG_SET_SLICE(isp_dmsc_shap_clip, ISP_DMSC_SHARPEN_CLIP_BLACK,
		      pSharpen->sharpen_clip_black);
	REG_SET_SLICE(isp_dmsc_shap_clip, ISP_DMSC_SHARPEN_CLIP_WHITE,
		      pSharpen->sharpen_clip_white);
	REG_SET_SLICE(isp_dmsc_shap_thr, ISP_DMSC_SHARPEN_T4_SHIFT,
		      pSharpen->sharpen_t4_shift);
	REG_SET_SLICE(isp_dmsc_shap_thr, ISP_DMSC_SHARPEN_T3,
		      pSharpen->sharpen_t3);
	REG_SET_SLICE(isp_dmsc_shap_thr, ISP_DMSC_SHARPEN_T2_SHIFT,
		      pSharpen->sharpen_t2_shift);
	REG_SET_SLICE(isp_dmsc_shap_thr, ISP_DMSC_SHARPEN_T1,
		      pSharpen->sharpen_t1);
	REG_SET_SLICE(isp_dmsc_shap_ratio, ISP_DMSC_SHARPEN_R3,
		      pSharpen->sharpen_r3);
	REG_SET_SLICE(isp_dmsc_shap_ratio, ISP_DMSC_SHARPEN_R2,
		      pSharpen->sharpen_r2);
	REG_SET_SLICE(isp_dmsc_shap_ratio, ISP_DMSC_SHARPEN_R1,
		      pSharpen->sharpen_r1);
	REG_SET_SLICE(isp_dmsc_shap_line_ctrl, ISP_DMSC_SHARPEN_LINE_SHIFT2,
		      pSharpen->dmsc_sharpen_line_shift2);
	REG_SET_SLICE(isp_dmsc_shap_line_ctrl, ISP_DMSC_SHARPEN_LINE_SHIFT1,
		      pSharpen->dmsc_sharpen_line_shift1);
	REG_SET_SLICE(isp_dmsc_shap_line_ctrl, ISP_DMSC_SHARPEN_LINE_T1,
		      pSharpen->dmsc_sharpen_line_t1);
	REG_SET_SLICE(isp_dmsc_shap_line_ctrl, ISP_DMSC_SHARPEN_LINE_STRENGTH,
		      pSharpen->sharpen_line_strength);
	REG_SET_SLICE(isp_dmsc_shap_line_ratio, ISP_DMSC_SHARPEN_LINE_R2,
		      pSharpen->sharpen_line_r2);
	REG_SET_SLICE(isp_dmsc_shap_line_ratio, ISP_DMSC_SHARPEN_LINE_R1,
		      pSharpen->sharpen_line_r1);
	REG_SET_SLICE(isp_dmsc_shap_line_ratio, ISP_DMSC_SHARPEN_LINE_STRENGTH,
		      pSharpen->sharpen_line_strength);
	REG_SET_SLICE(isp_dmsc_shap_filt1, ISP_DMSC_HF_FILT_00,
		      pSharpen->hf_filt_00);
	REG_SET_SLICE(isp_dmsc_shap_filt1, ISP_DMSC_HF_FILT_01,
		      pSharpen->hf_filt_01);
	REG_SET_SLICE(isp_dmsc_shap_filt1, ISP_DMSC_HF_FILT_02,
		      pSharpen->hf_filt_02);
	REG_SET_SLICE(isp_dmsc_shap_filt1, ISP_DMSC_HF_FILT_10,
		      pSharpen->hf_filt_10);
	REG_SET_SLICE(isp_dmsc_shap_filt1, ISP_DMSC_HF_FILT_11,
		      pSharpen->hf_filt_11);
	REG_SET_SLICE(isp_dmsc_shap_filt2, ISP_DMSC_HF_FILT_12,
		      pSharpen->hf_filt_12);
	REG_SET_SLICE(isp_dmsc_shap_filt2, ISP_DMSC_HF_FILT_20,
		      pSharpen->hf_filt_20);
	REG_SET_SLICE(isp_dmsc_shap_filt2, ISP_DMSC_HF_FILT_21,
		      pSharpen->hf_filt_21);
	REG_SET_SLICE(isp_dmsc_shap_filt2, ISP_DMSC_HF_FILT_22,
		      pSharpen->hf_filt_22);

	isp_write_reg(dev, REG_ADDR(isp_dmsc_shap_fact), isp_dmsc_shap_fact);
	isp_write_reg(dev, REG_ADDR(isp_dmsc_shap_clip), isp_dmsc_shap_clip);
	isp_write_reg(dev, REG_ADDR(isp_dmsc_shap_thr), isp_dmsc_shap_thr);
	isp_write_reg(dev, REG_ADDR(isp_dmsc_shap_ratio), isp_dmsc_shap_ratio);
	isp_write_reg(dev, REG_ADDR(isp_dmsc_shap_line_ctrl),
		      isp_dmsc_shap_line_ctrl);
	isp_write_reg(dev, REG_ADDR(isp_dmsc_shap_line_ratio),
		      isp_dmsc_shap_line_ratio);
	isp_write_reg(dev, REG_ADDR(isp_dmsc_shap_filt1), isp_dmsc_shap_filt1);
	isp_write_reg(dev, REG_ADDR(isp_dmsc_shap_filt2), isp_dmsc_shap_filt2);

	REG_SET_SLICE(isp_dmsc_ctrl, ISP_DEMOSAIC_SHARPEN_SIZE, pSharpen->sharpen_size);
	REG_SET_SLICE(isp_dmsc_ctrl, ISP_DEMOSAIC_SHARPEN_ENBALE, 1);
	isp_write_reg(dev, REG_ADDR(isp_dmsc_ctrl), isp_dmsc_ctrl);
	isp_write_reg(dev, REG_ADDR(isp_dmsc_ctrl_shd), isp_dmsc_ctrl);

	return 0;
}

int isp_set_dmsc_cac(struct isp_ic_dev *dev)
{
	struct isp_cac_context *cac = &dev->cac;
	u32 val = 0;
	u32 isp_dmsc_cac_ctrl = isp_read_reg(dev, REG_ADDR(isp_dmsc_cac_ctrl));

	pr_info("enter %s\n", __func__);

	if (!cac->enable) {
		REG_SET_SLICE(isp_dmsc_cac_ctrl, MRV_CAC_CAC_EN, 0);
		isp_write_reg(dev, REG_ADDR(isp_dmsc_cac_ctrl),
			      isp_dmsc_cac_ctrl);
		return 0;
	}

	REG_SET_SLICE(isp_dmsc_cac_ctrl, MRV_CAC_H_CLIP_MODE, cac->hmode);
	REG_SET_SLICE(isp_dmsc_cac_ctrl, MRV_CAC_V_CLIP_MODE, cac->vmode);
	isp_write_reg(dev, REG_ADDR(isp_dmsc_cac_count_start),
		      cac->hstart | (cac->vstart << 16));
	isp_write_reg(dev, REG_ADDR(isp_dmsc_cac_a), cac->ar | (cac->ab << 16));
	isp_write_reg(dev, REG_ADDR(isp_dmsc_cac_b), cac->br | (cac->bb << 16));
	isp_write_reg(dev, REG_ADDR(isp_dmsc_cac_c), cac->cr | (cac->cb << 16));

	REG_SET_SLICE(val, MRV_CAC_X_NS, cac->xns);
	REG_SET_SLICE(val, MRV_CAC_X_NF, cac->xnf);
	isp_write_reg(dev, REG_ADDR(isp_dmsc_cac_x_norm), val);
	val = 0;
	REG_SET_SLICE(val, MRV_CAC_Y_NS, cac->yns);
	REG_SET_SLICE(val, MRV_CAC_Y_NF, cac->ynf);
	isp_write_reg(dev, REG_ADDR(isp_dmsc_cac_y_norm), val);
	REG_SET_SLICE(isp_dmsc_cac_ctrl, MRV_CAC_CAC_EN, 1);
	isp_write_reg(dev, REG_ADDR(isp_dmsc_cac_ctrl), isp_dmsc_cac_ctrl);

	return 0;
}

int isp_set_dmsc_skin(struct isp_ic_dev *dev)
{
	struct isp_skin_context *pSkin = &dev->demosaic.skin_cxt;
	u32 isp_dmsc_dpul_ctrl =
	    isp_read_reg(dev, REG_ADDR(isp_dmsc_dpul_ctrl));
	u32 isp_dmsc_skin_thr_cb =
	    isp_read_reg(dev, REG_ADDR(isp_dmsc_skin_thr_cb));
	u32 isp_dmsc_skin_thr_cr =
	    isp_read_reg(dev, REG_ADDR(isp_dmsc_skin_thr_cr));
	u32 isp_dmsc_skin_thr_y =
	    isp_read_reg(dev, REG_ADDR(isp_dmsc_skin_thr_y));

	u32 isp_dmsc_ctrl = isp_read_reg(dev, REG_ADDR(isp_dmsc_ctrl));

	pr_info("enter %s\n", __func__);
	if (!pSkin->enable) {
		REG_SET_SLICE(isp_dmsc_ctrl, ISP_DEMOSAIC_SKIN_ENABLE, 0U);

		REG_SET_SLICE(isp_dmsc_ctrl, ISP_DEMOSAIC_DEPURPLE_ENABLE, 0U);
		isp_write_reg(dev, REG_ADDR(isp_dmsc_ctrl), isp_dmsc_ctrl);
		return 0;
	}

	REG_SET_SLICE(isp_dmsc_dpul_ctrl, ISP_DMSC_CBCR_MODE, pSkin->cbcr_mode);
	REG_SET_SLICE(isp_dmsc_dpul_ctrl, ISP_DMSC_DEPURPLE_SAT_SHRINK,
		      pSkin->depurple_sat_shrink);
	REG_SET_SLICE(isp_dmsc_dpul_ctrl, ISP_DMSC_DEPURPLE_THR,
		      pSkin->depurple_thr);
	REG_SET_SLICE(isp_dmsc_skin_thr_cb, ISP_DMSC_SKIN_CB_THR_MAX_2047,
		      pSkin->cb_thr_max_2047);
	REG_SET_SLICE(isp_dmsc_skin_thr_cb, ISP_DMSC_SKIN_CB_THR_MIN_2047,
		      pSkin->cb_thr_min_2047);
	REG_SET_SLICE(isp_dmsc_skin_thr_cr, ISP_DMSC_SKIN_CR_THR_MAX_2047,
		      pSkin->cr_thr_max_2047);
	REG_SET_SLICE(isp_dmsc_skin_thr_cr, ISP_DMSC_SKIN_CR_THR_MIN_2047,
		      pSkin->cr_thr_min_2047);
	REG_SET_SLICE(isp_dmsc_skin_thr_y, ISP_DMSC_SKIN_Y_THR_MAX,
		      pSkin->y_thr_max_2047);
	REG_SET_SLICE(isp_dmsc_skin_thr_y, ISP_DMSC_SKIN_Y_THR_MIN,
		      pSkin->y_thr_min_2047);

	isp_write_reg(dev, REG_ADDR(isp_dmsc_skin_thr_cb),
		      isp_dmsc_skin_thr_cb);
	isp_write_reg(dev, REG_ADDR(isp_dmsc_skin_thr_cr),
		      isp_dmsc_skin_thr_cr);
	isp_write_reg(dev, REG_ADDR(isp_dmsc_skin_thr_y), isp_dmsc_skin_thr_y);
	isp_write_reg(dev, REG_ADDR(isp_dmsc_dpul_ctrl), isp_dmsc_dpul_ctrl);
	isp_write_reg(dev, REG_ADDR(isp_dmsc_dpul_ctrl_shd),
		      isp_dmsc_dpul_ctrl);

	REG_SET_SLICE(isp_dmsc_ctrl, ISP_DEMOSAIC_DEPURPLE_ENABLE, 1U);
	REG_SET_SLICE(isp_dmsc_ctrl, ISP_DEMOSAIC_SKIN_ENABLE, 1U);
	isp_write_reg(dev, REG_ADDR(isp_dmsc_ctrl), isp_dmsc_ctrl);

	return 0;
}

int isp_s_dmsc(struct isp_ic_dev *dev)
{
	/*Clear the cmodel register default value to match the fpga default value */
	isp_disable_dmsc(dev);

	u32 isp_dmsc_ctrl = isp_read_reg(dev, REG_ADDR(isp_dmsc_ctrl));
	pr_info("enter %s\n", __func__);
	if (!dev->demosaic.enable) {
		isp_disable_dmsc(dev);
		return 0;
	}
	REG_SET_SLICE(isp_dmsc_ctrl, ISP_DEMOSAIC_THR,
		      dev->demosaic.demosaic_thr);
	REG_SET_SLICE(isp_dmsc_ctrl, ISP_DEMOSAIC_DENOISE_STRENGTH,
		      dev->demosaic.denoise_stren);
	isp_write_reg(dev, REG_ADDR(isp_dmsc_ctrl), isp_dmsc_ctrl);
	isp_write_reg(dev, REG_ADDR(isp_dmsc_ctrl_shd), isp_dmsc_ctrl);

	isp_set_dmsc_intp(dev);
	isp_set_dmsc_dmoi(dev);
	isp_set_dmsc_skin(dev);
	isp_enable_dmsc(dev);

	return 0;
}

#endif
