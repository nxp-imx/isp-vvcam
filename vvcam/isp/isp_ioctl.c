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

/* process public and sample isp command. for complex modules, need new files.*/
#include "mrv_all_bits.h"
#include "isp_ioctl.h"
#include "isp_types.h"
#include <linux/regmap.h>
#include <linux/of_reserved_mem.h>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#endif

#ifdef CONFIG_VSI_ISP_DEBUG
#define isp_info(fmt, ...)	pr_info(fmt, ##__VA_ARGS__)
#define isp_debug(fmt, ...)  pr_debug(fmt, ##__VA_ARGS__)
#define isp_err(fmt, ...)  pr_err(fmt, ##__VA_ARGS__)
#else
#define isp_info(fmt, ...)
#define isp_debug(fmt, ...)
#define isp_err(fmt, ...)  pr_err(fmt, ##__VA_ARGS__)
#endif

#ifdef __clang__
#pragma clang diagnostic pop
#endif



volatile MrvAllRegister_t *all_regs = NULL;


void isp_write_reg(struct isp_ic_dev *dev, u32 offset, u32 val)
{
	if (offset >= ISP_REG_SIZE)
		return;
	writel(val, dev->base + offset);
	if ((offset >= REG_ADDR(mi_mp_y_base_ad_init))
		&& (offset <= REG_ADDR(mi_mp_y_pic_size)))
		writel(val, dev->base + offset);
}

u32 isp_read_reg(struct isp_ic_dev *dev, u32 offset)
{
	u32 val = 0;

	if (offset >= ISP_REG_SIZE)
		return 0;
	val = readl(dev->base + offset);
	if ((offset >= REG_ADDR(mi_mp_y_base_ad_init))
		&& (offset <= REG_ADDR(mi_mp_y_pic_size)))
		val = readl(dev->base + offset);
	return val;
}

int isp_reset(struct isp_ic_dev *dev)
{
	isp_info("enter %s\n", __func__);
	isp_write_reg(dev, REG_ADDR(vi_ircl), 0xFFFFFFBF);
	mdelay(2);
	isp_write_reg(dev, REG_ADDR(vi_ircl), 0x0);
	return 0;
}

int isp_enable_tpg(struct isp_ic_dev *dev)
{
	u32 addr, isp_tpg_ctrl;

	isp_info("enter %s\n", __func__);
	addr = REG_ADDR(isp_tpg_ctrl);
	isp_tpg_ctrl = isp_read_reg(dev, addr);
	REG_SET_SLICE(isp_tpg_ctrl, TPG_ENABLE, 1);
	isp_write_reg(dev, addr, isp_tpg_ctrl);
	return 0;
}

int isp_disable_tpg(struct isp_ic_dev *dev)
{
	u32 addr, isp_tpg_ctrl;

	isp_info("enter %s\n", __func__);
	addr = REG_ADDR(isp_tpg_ctrl);
	isp_tpg_ctrl = isp_read_reg(dev, addr);
	REG_SET_SLICE(isp_tpg_ctrl, TPG_ENABLE, 0);
	isp_write_reg(dev, addr, isp_tpg_ctrl);
	return 0;
}

int isp_enable_bls(struct isp_ic_dev *dev)
{
#ifndef ISP_BLS
	return -1;
#else
	u32 isp_bls_ctrl = isp_read_reg(dev, REG_ADDR(isp_bls_ctrl));

	isp_info("enter %s\n", __func__);
	REG_SET_SLICE(isp_bls_ctrl, MRV_BLS_BLS_ENABLE,
			  MRV_BLS_BLS_ENABLE_PROCESS);
	isp_write_reg(dev, REG_ADDR(isp_bls_ctrl), isp_bls_ctrl);
	return 0;
#endif
}

int isp_disable_bls(struct isp_ic_dev *dev)
{
#ifndef ISP_BLS
	return -1;
#else
	u32 isp_bls_ctrl = isp_read_reg(dev, REG_ADDR(isp_bls_ctrl));

	isp_info("enter %s\n", __func__);
	REG_SET_SLICE(isp_bls_ctrl, MRV_BLS_BLS_ENABLE,
			  MRV_BLS_BLS_ENABLE_BYPASS);
	isp_write_reg(dev, REG_ADDR(isp_bls_ctrl), isp_bls_ctrl);
	return 0;
#endif
}

int isp_enable(struct isp_ic_dev *dev)
{
	u32 isp_ctrl, isp_imsc;

	isp_info("enter %s\n", __func__);
	isp_imsc = isp_read_reg(dev, REG_ADDR(isp_imsc));
	isp_imsc |= (MRV_ISP_IMSC_ISP_OFF_MASK | MRV_ISP_IMSC_FRAME_MASK);
	isp_write_reg(dev, REG_ADDR(isp_imsc), isp_imsc);
	isp_ctrl = isp_read_reg(dev, REG_ADDR(isp_ctrl));
	REG_SET_SLICE(isp_ctrl, MRV_ISP_ISP_INFORM_ENABLE, 1);
	isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);
	REG_SET_SLICE(isp_ctrl, MRV_ISP_ISP_ENABLE, 1);
	isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);
	/*Set lsc tbl after isp enable*/
	if (dev->update_lsc_tbl) {
		isp_s_lsc_tbl(dev);
		dev->update_lsc_tbl = false;
	}

	REG_SET_SLICE(isp_ctrl, MRV_ISP_ISP_CFG_UPD, 1);
	isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);

	return 0;
}

int isp_disable(struct isp_ic_dev *dev)
{
	u32 isp_ctrl;

	isp_info("enter %s\n", __func__);
	isp_ctrl = isp_read_reg(dev, REG_ADDR(isp_ctrl));
	REG_SET_SLICE(isp_ctrl, MRV_ISP_ISP_ENABLE, 0);
	isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);
	REG_SET_SLICE(isp_ctrl, MRV_ISP_ISP_INFORM_ENABLE, 0);
	isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);
	REG_SET_SLICE(isp_ctrl, MRV_ISP_ISP_GEN_CFG_UPD, 1);
	isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);
	isp_write_reg(dev, REG_ADDR(isp_isr), MRV_ISP_ISR_ISP_OFF_MASK);
	return 0;
}

bool is_isp_enable(struct isp_ic_dev *dev)
{
	return isp_read_reg(dev, REG_ADDR(isp_ctrl)) & 0x01;
}

int isp_enable_lsc(struct isp_ic_dev *dev)
{

	u32 isp_lsc_ctrl = isp_read_reg(dev, REG_ADDR(isp_lsc_ctrl));
	isp_info("enter %s\n", __func__);
	REG_SET_SLICE(isp_lsc_ctrl, MRV_LSC_LSC_EN, 1U);
	isp_write_reg(dev, REG_ADDR(isp_lsc_ctrl), isp_lsc_ctrl);

	{
	uint32_t isp_ctrl = isp_read_reg(dev, REG_ADDR(isp_ctrl));
	REG_SET_SLICE(isp_ctrl, MRV_ISP_ISP_GEN_CFG_UPD, 1);
	isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);
	}

	return 0;
}

int isp_disable_lsc(struct isp_ic_dev *dev)
{
	u32 isp_lsc_ctrl = isp_read_reg(dev, REG_ADDR(isp_lsc_ctrl));

	isp_info("enter %s\n", __func__);
	REG_SET_SLICE(isp_lsc_ctrl, MRV_LSC_LSC_EN, 0U);
	isp_write_reg(dev, REG_ADDR(isp_lsc_ctrl), isp_lsc_ctrl);

	{
	uint32_t isp_ctrl = isp_read_reg(dev, REG_ADDR(isp_ctrl));
	REG_SET_SLICE(isp_ctrl, MRV_ISP_ISP_GEN_CFG_UPD, 1);
	isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);
	}

	return 0;
}

#if defined(ISP8000NANO_V1802)
static int isp_gpr_input_control(struct isp_ic_dev *dev)
{
	struct isp_context isp_ctx = *(&dev->ctx);
	unsigned int fmt_offset = 3;
	unsigned int isp_dewarp_control_val;

	if (dev->mix_gpr == NULL)
		return -ENOMEM;

	if (dev->id == 0)
		fmt_offset = 3;
	else
		fmt_offset = 13;

	regmap_read(dev->mix_gpr, 0x138, &isp_dewarp_control_val);
	if (isp_dewarp_control_val == 0)
		isp_dewarp_control_val = 0x8d8360;

	switch (isp_ctx.input_selection) {
	case MRV_ISP_INPUT_SELECTION_12EXT:
		isp_dewarp_control_val &= ~(0x3f << fmt_offset);
		isp_dewarp_control_val |= (0x2c << fmt_offset);
		break;
	case MRV_ISP_INPUT_SELECTION_10ZERO:
	case MRV_ISP_INPUT_SELECTION_10MSB:
		isp_dewarp_control_val &= ~(0x3f << fmt_offset);
		isp_dewarp_control_val |= (0x2b << fmt_offset);
		break;
	case MRV_ISP_INPUT_SELECTION_8ZERO:
	case MRV_ISP_INPUT_SELECTION_8MSB:
		isp_dewarp_control_val &= ~(0x3f << fmt_offset);
		isp_dewarp_control_val |= (0x2a << fmt_offset);
		break;
	default:
		return 0;
	}
	regmap_write(dev->mix_gpr, 0x138, isp_dewarp_control_val);
	return 0;
}
#endif

int isp_s_input(struct isp_ic_dev *dev)
{
	struct isp_context isp_ctx = *(&dev->ctx);
	u32 isp_ctrl, isp_acq_prop, isp_demosaic;
	u32 isp_stitching_ctrl;

	isp_info("enter %s\n", __func__);
	isp_ctrl = isp_read_reg(dev, REG_ADDR(isp_ctrl));
	REG_SET_SLICE(isp_ctrl, MRV_ISP_ISP_MODE, isp_ctx.mode);
	isp_acq_prop = isp_read_reg(dev, REG_ADDR(isp_acq_prop));
	REG_SET_SLICE(isp_acq_prop, MRV_ISP_SAMPLE_EDGE, isp_ctx.sample_edge);
	REG_SET_SLICE(isp_acq_prop, MRV_ISP_HSYNC_POL,
			  isp_ctx.hSyncLowPolarity);
	REG_SET_SLICE(isp_acq_prop, MRV_ISP_VSYNC_POL,
			  isp_ctx.vSyncLowPolarity);
	REG_SET_SLICE(isp_acq_prop, MRV_ISP_BAYER_PAT, isp_ctx.bayer_pattern);
	REG_SET_SLICE(isp_acq_prop, MRV_ISP_CONV_422, isp_ctx.sub_sampling);
	REG_SET_SLICE(isp_acq_prop, MRV_ISP_CCIR_SEQ, isp_ctx.seq_ccir);
	REG_SET_SLICE(isp_acq_prop, MRV_ISP_FIELD_SELECTION,
			  isp_ctx.field_selection);
	REG_SET_SLICE(isp_acq_prop, MRV_ISP_INPUT_SELECTION,
			  isp_ctx.input_selection);
	REG_SET_SLICE(isp_acq_prop, MRV_ISP_LATENCY_FIFO_SELECTION,
			  isp_ctx.latency_fifo);

	isp_write_reg(dev, REG_ADDR(isp_acq_prop), isp_acq_prop);
	isp_write_reg(dev, REG_ADDR(isp_acq_h_offs), isp_ctx.acqWindow.x);
	isp_write_reg(dev, REG_ADDR(isp_acq_v_offs), isp_ctx.acqWindow.y);
	isp_write_reg(dev, REG_ADDR(isp_acq_h_size), isp_ctx.acqWindow.width);
	isp_write_reg(dev, REG_ADDR(isp_acq_v_size), isp_ctx.acqWindow.height);

	isp_write_reg(dev, REG_ADDR(isp_stitching_frame_width), isp_ctx.acqWindow.width);
	isp_write_reg(dev, REG_ADDR(isp_stitching_frame_height), isp_ctx.acqWindow.height);

	isp_write_reg(dev, REG_ADDR(isp_out_h_offs),
			  (isp_ctx.ofWindow.x & MRV_ISP_ISP_OUT_H_OFFS_MASK));
	isp_write_reg(dev, REG_ADDR(isp_out_v_offs),
			  (isp_ctx.ofWindow.y & MRV_ISP_ISP_OUT_V_OFFS_MASK));
	isp_write_reg(dev, REG_ADDR(isp_out_h_size),
			  (isp_ctx.ofWindow.width & MRV_ISP_ISP_OUT_H_SIZE_MASK));
	isp_write_reg(dev, REG_ADDR(isp_out_v_size),
			  (isp_ctx.ofWindow.height & MRV_ISP_ISP_OUT_V_SIZE_MASK));

	isp_write_reg(dev, REG_ADDR(isp_is_h_offs),
			  (isp_ctx.isWindow.x & MRV_IS_IS_H_OFFS_MASK));
	isp_write_reg(dev, REG_ADDR(isp_is_v_offs),
			  (isp_ctx.isWindow.y & MRV_IS_IS_V_OFFS_MASK));
	isp_write_reg(dev, REG_ADDR(isp_is_h_size),
			  (isp_ctx.isWindow.width & MRV_IS_IS_H_SIZE_MASK));
	isp_write_reg(dev, REG_ADDR(isp_is_v_size),
			  (isp_ctx.isWindow.height & MRV_IS_IS_V_SIZE_MASK));

	isp_demosaic = isp_read_reg(dev, REG_ADDR(isp_demosaic));
	REG_SET_SLICE(isp_demosaic, MRV_ISP_DEMOSAIC_BYPASS,
			  isp_ctx.bypass_mode);
	REG_SET_SLICE(isp_demosaic, MRV_ISP_DEMOSAIC_TH,
			  isp_ctx.demosaic_threshold);
	isp_write_reg(dev, REG_ADDR(isp_demosaic), isp_demosaic);
	isp_write_reg(dev, REG_ADDR(isp_stitching_hdr_mode), isp_ctx.stitching_mode);
	isp_stitching_ctrl = isp_read_reg(dev, REG_ADDR(isp_stitching_ctrl));
	REG_SET_SLICE(isp_stitching_ctrl, STITCHING_BAYER_PATTERN, isp_ctx.bayer_pattern);
	isp_write_reg(dev, REG_ADDR(isp_stitching_ctrl), isp_stitching_ctrl);
	isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);

#if defined(ISP8000NANO_V1802)
	isp_gpr_input_control(dev);
#endif
	return 0;
}

int isp_s_digital_gain(struct isp_ic_dev *dev)
{
	struct isp_digital_gain_cxt dgain = *(&dev->dgain);
	u32 isp_dgain_rb = isp_read_reg(dev, REG_ADDR(isp_dgain_rb));
	u32 isp_dgain_g = isp_read_reg(dev, REG_ADDR(isp_dgain_g));
	u32 isp_ctrl = isp_read_reg(dev, REG_ADDR(isp_ctrl));
	if (!dgain.enable) {
		isp_err("Disable isp digital gain");
		REG_SET_SLICE(isp_ctrl, MRV_ISP_DIGITAL_GAIN_EN, 0U);
		isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);
		return 0;
	}

	isp_info("enter %s\n", __func__);
	REG_SET_SLICE(isp_dgain_rb, ISP_DIGITAL_GAIN_R, dgain.gain_r);
	REG_SET_SLICE(isp_dgain_rb, ISP_DIGITAL_GAIN_B, dgain.gain_b);

	REG_SET_SLICE(isp_dgain_g, ISP_DIGITAL_GAIN_GR, dgain.gain_gr);
	REG_SET_SLICE(isp_dgain_g, ISP_DIGITAL_GAIN_GB, dgain.gain_gb);
	REG_SET_SLICE(isp_ctrl, MRV_ISP_DIGITAL_GAIN_EN, 1U);

	isp_write_reg(dev, REG_ADDR(isp_dgain_rb), isp_dgain_rb);
	isp_write_reg(dev, REG_ADDR(isp_dgain_g), isp_dgain_g);
	isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);
	return 0;
}

int isp_s_demosaic(struct isp_ic_dev *dev)
{
	struct isp_context isp_ctx = *(&dev->ctx);
	u32 isp_demosaic;
	isp_info("enter %s\n", __func__);
	isp_demosaic = isp_read_reg(dev, REG_ADDR(isp_demosaic));
	REG_SET_SLICE(isp_demosaic, MRV_ISP_DEMOSAIC_BYPASS,
			  isp_ctx.bypass_mode);
	REG_SET_SLICE(isp_demosaic, MRV_ISP_DEMOSAIC_TH,
			  isp_ctx.demosaic_threshold);
	isp_write_reg(dev, REG_ADDR(isp_demosaic), isp_demosaic);
	return 0;;
}

int isp_s_tpg(struct isp_ic_dev *dev)
{
	struct isp_tpg_context tpg = *(&dev->tpg);
	u32 addr, regVal;

	isp_info("enter %s\n", __func__);
	addr = REG_ADDR(isp_tpg_ctrl);
	regVal = isp_read_reg(dev, addr);
	REG_SET_SLICE(regVal, TPG_IMG_NUM, tpg.image_type);
	REG_SET_SLICE(regVal, TPG_CFA_PAT, tpg.bayer_pattern);
	REG_SET_SLICE(regVal, TPG_COLOR_DEPTH, tpg.color_depth);
	REG_SET_SLICE(regVal, TPG_RESOLUTION, tpg.resolution);
	isp_write_reg(dev, addr, regVal);
	regVal = 0;
	REG_SET_SLICE(regVal, TPG_PIX_GAP_IN, tpg.pixleGap);
	REG_SET_SLICE(regVal, TPG_LINE_GAP_IN, tpg.lineGap);
	isp_write_reg(dev, REG_ADDR(isp_tpg_gap_in), regVal);
	regVal = 0;
	REG_SET_SLICE(regVal, TPG_PIX_GAP_STD_IN, tpg.gapStandard);
	isp_write_reg(dev, REG_ADDR(isp_tpg_gap_std_in), regVal);
	regVal = 0;
	REG_SET_SLICE(regVal, TPG_RANDOM_SEED, tpg.randomSeed);
	isp_write_reg(dev, REG_ADDR(isp_tpg_random_seed), regVal);
	REG_SET_SLICE(regVal, TPG_HTOTAL_IN, tpg.user_mode_h.total);
	REG_SET_SLICE(regVal, TPG_VTOTAL_IN, tpg.user_mode_v.total);
	isp_write_reg(dev, REG_ADDR(isp_tpg_total_in), regVal);
	regVal = 0;
	REG_SET_SLICE(regVal, TPG_HACT_IN, tpg.user_mode_h.act);
	REG_SET_SLICE(regVal, TPG_VACT_IN, tpg.user_mode_v.act);
	isp_write_reg(dev, REG_ADDR(isp_tpg_act_in), regVal);
	regVal = 0;
	REG_SET_SLICE(regVal, TPG_FP_H_IN, tpg.user_mode_h.fp);
	REG_SET_SLICE(regVal, TPG_FP_V_IN, tpg.user_mode_v.fp);
	isp_write_reg(dev, REG_ADDR(isp_tpg_fp_in), regVal);
	regVal = 0;
	REG_SET_SLICE(regVal, TPG_BP_H_IN, tpg.user_mode_h.bp);
	REG_SET_SLICE(regVal, TPG_BP_V_IN, tpg.user_mode_v.bp);
	isp_write_reg(dev, REG_ADDR(isp_tpg_bp_in), regVal);
	regVal = 0;
	REG_SET_SLICE(regVal, TPG_HS_W_IN, tpg.user_mode_h.sync);
	REG_SET_SLICE(regVal, TPG_VS_W_IN, tpg.user_mode_v.sync);
	isp_write_reg(dev, REG_ADDR(isp_tpg_w_in), regVal);
	return 0;
}

int isp_s_mcm(struct isp_ic_dev *dev)
{
	struct isp_mcm_context *mcm = &dev->mcm;
	u32 mcm_ctrl;
	u32 mcm_retiming0;
	u32 mcm_retiming1;

	isp_info("enter %s\n", __func__);
	mcm_ctrl = isp_read_reg(dev, REG_ADDR(mcm_ctrl));
	mcm_retiming0 = isp_read_reg(dev, REG_ADDR(mcm_retiming0));
	mcm_retiming1 = isp_read_reg(dev, REG_ADDR(mcm_retiming1));

	if (mcm->bypass_enable) {
		REG_SET_SLICE(mcm_ctrl, MCM_BYPASS_EN, 1);
	} else {
		REG_SET_SLICE(mcm_ctrl, MCM_BYPASS_EN, 0);
	}

	REG_SET_SLICE(mcm_retiming0, MCM_VSYNC_DURATION, mcm->vsync_duration);
	REG_SET_SLICE(mcm_retiming0, MCM_VSYNC_BLANK, mcm->vsync_blank);
	REG_SET_SLICE(mcm_retiming1, MCM_HSYNC_PREAMPLE, mcm->hsync_preample);
	REG_SET_SLICE(mcm_retiming1, MCM_HSYNC_BLANK, mcm->hsync_blank);

	isp_write_reg(dev, REG_ADDR(mcm_ctrl), mcm_ctrl);
	isp_write_reg(dev, REG_ADDR(mcm_retiming0), mcm_retiming0);
	isp_write_reg(dev, REG_ADDR(mcm_retiming1), mcm_retiming1);

	return 0;
}

int isp_s_mux(struct isp_ic_dev *dev)
{
	struct isp_mux_context mux = *(&dev->mux);
	u32 vi_dpcl;

	isp_info("enter %s\n", __func__);
	vi_dpcl = isp_read_reg(dev, REG_ADDR(vi_dpcl));
	REG_SET_SLICE(vi_dpcl, MRV_VI_MP_MUX, mux.mp_mux);
	REG_SET_SLICE(vi_dpcl, MRV_VI_DMA_SPMUX, mux.sp_mux);
	REG_SET_SLICE(vi_dpcl, MRV_VI_CHAN_MODE, mux.chan_mode);
	REG_SET_SLICE(vi_dpcl, MRV_VI_DMA_IEMUX, mux.ie_mux);
	REG_SET_SLICE(vi_dpcl, MRV_VI_DMA_SWITCH, mux.dma_read_switch);
	REG_SET_SLICE(vi_dpcl, MRV_IF_SELECT, mux.if_select);
	isp_write_reg(dev, REG_ADDR(vi_dpcl), vi_dpcl);
	return 0;
}

int isp_s_bls(struct isp_ic_dev *dev)
{
#ifndef ISP_BLS
	return -1;
#else
	struct isp_bls_context bls = *(&dev->bls);
	u32 isp_bls_ctrl = isp_read_reg(dev, REG_ADDR(isp_bls_ctrl));

	isp_info("enter %s\n", __func__);
	REG_SET_SLICE(isp_bls_ctrl, MRV_BLS_BLS_MODE, bls.mode);
	isp_write_reg(dev, REG_ADDR(isp_bls_ctrl), isp_bls_ctrl);
	isp_write_reg(dev, REG_ADDR(isp_bls_a_fixed), bls.a);
	isp_write_reg(dev, REG_ADDR(isp_bls_b_fixed), bls.b);
	isp_write_reg(dev, REG_ADDR(isp_bls_c_fixed), bls.c);
	isp_write_reg(dev, REG_ADDR(isp_bls_d_fixed), bls.d);
	return 0;
#endif
}

int isp_enable_awb(struct isp_ic_dev *dev)
{
	u32 isp_awb_prop = isp_read_reg(dev, REG_ADDR(isp_awb_prop));
	u32 isp_imsc = isp_read_reg(dev, REG_ADDR(isp_imsc));

	isp_info("enter %s\n", __func__);
	REG_SET_SLICE(isp_awb_prop, MRV_ISP_AWB_MODE, MRV_ISP_AWB_MODE_MEAS);
	isp_write_reg(dev, REG_ADDR(isp_awb_prop), isp_awb_prop);
	isp_write_reg(dev, REG_ADDR(isp_imsc),
			  isp_imsc | MRV_ISP_IMSC_AWB_DONE_MASK);
	return 0;
}

int isp_disable_awb(struct isp_ic_dev *dev)
{
	u32 isp_awb_prop = isp_read_reg(dev, REG_ADDR(isp_awb_prop));
	u32 isp_imsc = isp_read_reg(dev, REG_ADDR(isp_imsc));

	isp_info("enter %s\n", __func__);
	REG_SET_SLICE(isp_awb_prop, MRV_ISP_AWB_MODE, MRV_ISP_AWB_MODE_NOMEAS);
	isp_write_reg(dev, REG_ADDR(isp_awb_prop), isp_awb_prop);
	isp_write_reg(dev, REG_ADDR(isp_imsc),
			  isp_imsc & ~MRV_ISP_IMSC_AWB_DONE_MASK);
	return 0;
}

int isp_s_awb(struct isp_ic_dev *dev)
{
	struct isp_awb_context awb = *(&dev->awb);
	u32 isp_awb_thresh = 0;
	u32 isp_awb_ref = 0;
	u32 isp_awb_prop = 0;
	u32 wb_gain_rb_reg = 0;
	u32 wb_gain_g_reg = 0;

	/* isp_info("enter %s\n", __func__); */
	isp_awb_prop = isp_read_reg(dev, REG_ADDR(isp_awb_prop));

	if (awb.mode == MRV_ISP_AWB_MEAS_MODE_YCBCR) {
		REG_SET_SLICE(isp_awb_prop, MRV_ISP_AWB_MEAS_MODE,
				  MRV_ISP_AWB_MEAS_MODE_YCBCR);
		if (awb.max_y == 0) {
			REG_SET_SLICE(isp_awb_prop, MRV_ISP_AWB_MAX_EN,
					  MRV_ISP_AWB_MAX_EN_DISABLE);
		} else {
			REG_SET_SLICE(isp_awb_prop, MRV_ISP_AWB_MAX_EN,
					  MRV_ISP_AWB_MAX_EN_ENABLE);
		}
	} else if (awb.mode == MRV_ISP_AWB_MEAS_MODE_RGB) {
		REG_SET_SLICE(isp_awb_prop, MRV_ISP_AWB_MAX_EN,
				  MRV_ISP_AWB_MAX_EN_DISABLE);
		REG_SET_SLICE(isp_awb_prop, MRV_ISP_AWB_MEAS_MODE,
				  MRV_ISP_AWB_MEAS_MODE_RGB);
	}
	isp_write_reg(dev, REG_ADDR(isp_awb_prop), isp_awb_prop);

	REG_SET_SLICE(isp_awb_thresh, MRV_ISP_AWB_MAX_Y, awb.max_y);
	REG_SET_SLICE(isp_awb_thresh, MRV_ISP_AWB_MIN_Y__MAX_G,
			  awb.min_y_max_g);
	REG_SET_SLICE(isp_awb_thresh, MRV_ISP_AWB_MAX_CSUM, awb.max_c_sum);
	REG_SET_SLICE(isp_awb_thresh, MRV_ISP_AWB_MIN_C, awb.min_c);
	isp_write_reg(dev, REG_ADDR(isp_awb_thresh), isp_awb_thresh);

	REG_SET_SLICE(isp_awb_ref, MRV_ISP_AWB_REF_CR__MAX_R, awb.refcr_max_r);
	REG_SET_SLICE(isp_awb_ref, MRV_ISP_AWB_REF_CB__MAX_B, awb.refcb_max_b);
	isp_write_reg(dev, REG_ADDR(isp_awb_ref), isp_awb_ref);
	isp_write_reg(dev, REG_ADDR(isp_awb_frames), 0);
	isp_write_reg(dev, REG_ADDR(isp_awb_h_offs),
			  (MRV_ISP_AWB_H_OFFS_MASK & awb.window.x));
	isp_write_reg(dev, REG_ADDR(isp_awb_v_offs),
			  (MRV_ISP_AWB_V_OFFS_MASK & awb.window.y));
	isp_write_reg(dev, REG_ADDR(isp_awb_h_size),
			  (MRV_ISP_AWB_H_SIZE_MASK & awb.window.width));
	isp_write_reg(dev, REG_ADDR(isp_awb_v_size),
			  (MRV_ISP_AWB_V_SIZE_MASK & awb.window.height));

	REG_SET_SLICE(wb_gain_rb_reg, MRV_ISP_AWB_GAIN_R, awb.gain_r);
	REG_SET_SLICE(wb_gain_rb_reg, MRV_ISP_AWB_GAIN_B, awb.gain_b);
	isp_write_reg(dev, REG_ADDR(isp_awb_gain_rb), wb_gain_rb_reg);

	REG_SET_SLICE(wb_gain_g_reg, MRV_ISP_AWB_GAIN_GR, awb.gain_gr);
	REG_SET_SLICE(wb_gain_g_reg, MRV_ISP_AWB_GAIN_GB, awb.gain_gb);
	isp_write_reg(dev, REG_ADDR(isp_awb_gain_g), wb_gain_g_reg);

	return 0;
}

int isp_awb_control(struct isp_ic_dev *dev)
{
	unsigned long flags;
	int ret = 0;

	spin_lock_irqsave(&dev->irqlock, flags);

	if(!is_isp_enable(dev)) {
		ret = isp_s_awb(dev);
	} else {
		dev->awb.changed = true;
	}

	spin_unlock_irqrestore(&dev->irqlock, flags);

	return ret;
}

int isp_s_is(struct isp_ic_dev *dev)
{
	struct isp_is_context is = *(&dev->is);
	u32 isp_is_ctrl;
	u32 isp_is_displace;
	u32 isp_ctrl;

	isp_info("enter %s\n", __func__);

	isp_is_ctrl = isp_read_reg(dev, REG_ADDR(isp_is_ctrl));

	if (!is.enable) {
		REG_SET_SLICE(isp_is_ctrl, MRV_IS_IS_EN, 0);
		isp_write_reg(dev, REG_ADDR(isp_is_ctrl), isp_is_ctrl);
		return 0;
	}

	REG_SET_SLICE(isp_is_ctrl, MRV_IS_IS_EN, 1);
	isp_write_reg(dev, REG_ADDR(isp_is_h_offs), is.window.x);
	isp_write_reg(dev, REG_ADDR(isp_is_v_offs), is.window.y);
	isp_write_reg(dev, REG_ADDR(isp_is_h_size), is.window.width);
	isp_write_reg(dev, REG_ADDR(isp_is_v_size), is.window.height);
	isp_write_reg(dev, REG_ADDR(isp_is_recenter),
			  is.recenter & MRV_IS_IS_RECENTER_MASK);
	isp_write_reg(dev, REG_ADDR(isp_is_max_dx), is.max_dx);
	isp_write_reg(dev, REG_ADDR(isp_is_max_dy), is.max_dy);
	isp_is_displace = isp_read_reg(dev, REG_ADDR(isp_is_displace));
	REG_SET_SLICE(isp_is_displace, MRV_IS_DX, is.displace_x);
	REG_SET_SLICE(isp_is_displace, MRV_IS_DY, is.displace_y);
	isp_write_reg(dev, REG_ADDR(isp_is_displace), isp_is_displace);
	isp_write_reg(dev, REG_ADDR(isp_is_ctrl), isp_is_ctrl);
	if (is.update) {
		isp_ctrl = isp_read_reg(dev, REG_ADDR(isp_ctrl));
		REG_SET_SLICE(isp_ctrl, MRV_ISP_ISP_GEN_CFG_UPD, 1);
		isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);
		is.update = false;
	}
	return 0;
}

int isp_s_raw_is(struct isp_ic_dev *dev)
{
#ifndef ISP_RAWIS
	return -EINVAL;
#else
	struct isp_is_context rawis = *(&dev->rawis);
	u32 isp_raw_is_ctrl;
	u32 isp_raw_is_displace;
	u32 isp_ctrl;

	isp_info("enter %s\n", __func__);

	isp_raw_is_ctrl = isp_read_reg(dev, REG_ADDR(isp_raw_is_ctrl));

	if (!rawis.enable) {
		isp_write_reg(dev, REG_ADDR(isp_raw_is_h_size),
				  rawis.window.width);
		isp_write_reg(dev, REG_ADDR(isp_raw_is_v_size),
				  rawis.window.height);

		REG_SET_SLICE(isp_raw_is_ctrl, MRV_ISP_RAW_IS_EN, 0);
		isp_write_reg(dev, REG_ADDR(isp_raw_is_ctrl), isp_raw_is_ctrl);
		return 0;
	}

	REG_SET_SLICE(isp_raw_is_ctrl, MRV_ISP_RAW_IS_EN, 1);
	isp_write_reg(dev, REG_ADDR(isp_raw_is_h_offs), rawis.window.x);
	isp_write_reg(dev, REG_ADDR(isp_raw_is_v_offs), rawis.window.y);
	isp_write_reg(dev, REG_ADDR(isp_raw_is_h_size), rawis.window.width);
	isp_write_reg(dev, REG_ADDR(isp_raw_is_v_size), rawis.window.height);
	isp_write_reg(dev, REG_ADDR(isp_raw_is_recenter),
			  rawis.recenter & MRV_IS_IS_RECENTER_MASK);
	isp_write_reg(dev, REG_ADDR(isp_raw_is_max_dx), rawis.max_dx);
	isp_write_reg(dev, REG_ADDR(isp_raw_is_max_dy), rawis.max_dy);
	isp_raw_is_displace = isp_read_reg(dev, REG_ADDR(isp_raw_is_displace));
	REG_SET_SLICE(isp_raw_is_displace, MRV_ISP_RAW_IS_DX, rawis.displace_x);
	REG_SET_SLICE(isp_raw_is_displace, MRV_ISP_RAW_IS_DY, rawis.displace_y);
	isp_write_reg(dev, REG_ADDR(isp_raw_is_displace), isp_raw_is_displace);
	isp_write_reg(dev, REG_ADDR(isp_raw_is_ctrl), isp_raw_is_ctrl);
	/*dont update the configuration at the sub module function*/
	return 0;
#endif
}

int isp_s_cnr(struct isp_ic_dev *dev)
{
	struct isp_cnr_context *cnr = &dev->cnr;
	u32 isp_ctrl;

	isp_info("enter %s\n", __func__);
	isp_ctrl = isp_read_reg(dev, REG_ADDR(isp_ctrl));

	if (!cnr->enable) {
		REG_SET_SLICE(isp_ctrl, MRV_ISP_CNR_EN, 0);
		isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);
		return 0;
	}

	REG_SET_SLICE(isp_ctrl, MRV_ISP_CNR_EN, 1);
	isp_write_reg(dev, REG_ADDR(isp_cnr_linesize), cnr->line_width);
	isp_write_reg(dev, REG_ADDR(isp_cnr_threshold_c1), cnr->threshold_1);
	isp_write_reg(dev, REG_ADDR(isp_cnr_threshold_c2), cnr->threshold_2);
	isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);
	return 0;
}

int isp_start_stream(struct isp_ic_dev *dev, u32 numFrames)
{
	u32 isp_imsc, isp_ctrl;

	isp_info("enter %s\n", __func__);
	isp_write_reg(dev, REG_ADDR(isp_sh_ctrl), 0x10);
	isp_write_reg(dev, REG_ADDR(isp_acq_nr_frames),
			  (MRV_ISP_ACQ_NR_FRAMES_MASK & numFrames));
	isp_imsc = isp_read_reg(dev, REG_ADDR(isp_imsc));
	isp_imsc |=
		(MRV_ISP_IMSC_ISP_OFF_MASK | MRV_ISP_IMSC_FRAME_MASK |
		 MRV_ISP_IMSC_FRAME_IN_MASK);

	isp_write_reg(dev, REG_ADDR(isp_icr), 0xFFFFFFFF);
	isp_write_reg(dev, REG_ADDR(isp_imsc), isp_imsc);
	isp_ctrl = isp_read_reg(dev, REG_ADDR(isp_ctrl));

	REG_SET_SLICE(isp_ctrl, MRV_ISP_ISP_INFORM_ENABLE, 1);
	REG_SET_SLICE(isp_ctrl, MRV_ISP_ISP_ENABLE, 1);
	isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);
	/*Set lsc tbl after isp enable*/
	if (dev->update_lsc_tbl) {
		isp_s_lsc_tbl(dev);
		dev->update_lsc_tbl = false;
	}
	REG_SET_SLICE(isp_ctrl, MRV_ISP_ISP_GEN_CFG_UPD, 1);
	REG_SET_SLICE(isp_ctrl, MRV_ISP_ISP_CFG_UPD, 1);
	isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);

	return 0;
}

int isp_stop_stream(struct isp_ic_dev *dev)
{
	isp_info("enter %s\n", __func__);
	isp_write_reg(dev, REG_ADDR(isp_imsc), 0);
	isp_disable(dev);
	return 0;
}

int isp_s_cc(struct isp_ic_dev *dev)
{
	struct isp_cc_context *cc = &dev->cc;
	u32 isp_ctrl, addr;
	int i;

	isp_info("enter %s\n", __func__);
	isp_ctrl = isp_read_reg(dev, REG_ADDR(isp_ctrl));
	REG_SET_SLICE(isp_ctrl, MRV_ISP_ISP_CSM_Y_RANGE, cc->conv_range_y_full);
	REG_SET_SLICE(isp_ctrl, MRV_ISP_ISP_CSM_C_RANGE, cc->conv_range_c_full);
	isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);

	dev->cc.changed = false;

	if (cc->update_curve) {
		addr = REG_ADDR(isp_cc_coeff_0);
		for (i = 0; i < 9; i++) {
			isp_write_reg(dev, addr + i * 4,
					  MRV_ISP_CC_COEFF_0_MASK & cc->lCoeff[i]);
		}
	}
	return 0;
}

int isp_cc_control(struct isp_ic_dev *dev)
{
	unsigned long flags;
	int ret = 0;

	spin_lock_irqsave(&dev->irqlock, flags);
	if (!is_isp_enable(dev)) {
		ret = isp_s_cc(dev);
	} else {
		dev->cc.changed = true;
	}

	spin_unlock_irqrestore(&dev->irqlock, flags);

	return ret;
}

int isp_s_xtalk(struct isp_ic_dev *dev)
{
	struct isp_xtalk_context xtalk = *(&dev->xtalk);
	int i;


	for (i = 0; i < 9; i++) {
		isp_write_reg(dev, REG_ADDR(cross_talk_coef_block_arr[i]),
				  MRV_ISP_CT_COEFF_MASK & xtalk.lCoeff[i]);
	}

	isp_write_reg(dev, REG_ADDR(isp_ct_offset_r),
			  (MRV_ISP_CT_OFFSET_R_MASK & xtalk.r));
	isp_write_reg(dev, REG_ADDR(isp_ct_offset_g),
			  (MRV_ISP_CT_OFFSET_G_MASK & xtalk.g));
	isp_write_reg(dev, REG_ADDR(isp_ct_offset_b),
			  (MRV_ISP_CT_OFFSET_B_MASK & xtalk.b));
	return 0;
}

int isp_enable_wb(struct isp_ic_dev *dev, bool bEnable)
{
	u32 isp_ctrl = isp_read_reg(dev, REG_ADDR(isp_ctrl));

	isp_info("enter %s\n", __func__);
	REG_SET_SLICE(isp_ctrl, MRV_ISP_ISP_AWB_ENABLE, bEnable);
	isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);
	return 0;
}

int isp_enable_gamma_out(struct isp_ic_dev *dev, bool bEnable)
{
	u32 isp_ctrl;

	isp_info("enter %s\n", __func__);

	isp_ctrl = isp_read_reg(dev, REG_ADDR(isp_ctrl));
	REG_SET_SLICE(isp_ctrl, MRV_ISP_ISP_GAMMA_OUT_ENABLE, bEnable);
	isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);
	dev->gamma_out.enable_changed = false;

	return 0;
}

int isp_enable_gamma_out_ctrl(struct isp_ic_dev *dev, bool bEnable)
{
	unsigned long flags;
	int ret = 0;

	spin_lock_irqsave(&dev->irqlock, flags);

	dev->gamma_out.enableGamma = bEnable;
	if(!is_isp_enable(dev)) {
		ret = isp_enable_gamma_out(dev, bEnable);
	} else {
		dev->gamma_out.enable_changed = true;
	}

	spin_unlock_irqrestore(&dev->irqlock, flags);
	return ret;
}

int isp_s_gamma_out(struct isp_ic_dev *dev)
{
	u32 isp_gamma_out_mode;
	int i;
	struct isp_gamma_out_context *gamma = &dev->gamma_out;

	isp_gamma_out_mode = isp_read_reg(dev, REG_ADDR(isp_gamma_out_mode));
	REG_SET_SLICE(isp_gamma_out_mode, MRV_ISP_EQU_SEGM, gamma->mode);
	isp_write_reg(dev, REG_ADDR(isp_gamma_out_mode), isp_gamma_out_mode);

	for (i = 0; i < 17; i++) {
		isp_write_reg(dev, REG_ADDR(gamma_out_y_block_arr[i]),
					MRV_ISP_ISP_GAMMA_OUT_Y_MASK & gamma->curve[i]);
	}

	gamma->changed = false;

	return 0;
}

int isp_s_gamma_out_ctrl(struct isp_ic_dev *dev)
{
	unsigned long flags;
	int ret = 0;

	spin_lock_irqsave(&dev->irqlock, flags);

	if (!is_isp_enable(dev)) {
		ret = isp_s_gamma_out(dev);
	} else {
		dev->gamma_out.changed = true;
	}

	spin_unlock_irqrestore(&dev->irqlock, flags);
	return ret;
}

int isp_s_lsc_tbl(struct isp_ic_dev *dev)
{
	int i, n;
	u32 isp_ctrl;
	u32 sram_addr;
	u32 isp_lsc_status;
	struct isp_lsc_context *lsc = (&dev->lsc);

	isp_debug("enter %s\n", __func__);

	/*need to set tbl after isp_ctrl enable In ISP8000NANO_V1802*/
	isp_ctrl = isp_read_reg(dev, REG_ADDR(isp_ctrl));

	/* Enable isp to enable ram clock for write correct table to ram. */
	if (!(isp_ctrl & 0x01)) {
		dev->update_lsc_tbl = true;
		return 0;
	}

	isp_lsc_status = isp_read_reg(dev, REG_ADDR(isp_lsc_status));
	sram_addr = (isp_lsc_status & 0x2U) ? 0U : 153U;
	isp_write_reg(dev, REG_ADDR(isp_lsc_r_table_addr), sram_addr);
	isp_write_reg(dev, REG_ADDR(isp_lsc_gr_table_addr), sram_addr);
	isp_write_reg(dev, REG_ADDR(isp_lsc_gb_table_addr), sram_addr);
	isp_write_reg(dev, REG_ADDR(isp_lsc_b_table_addr), sram_addr);

	for (n = 0;
		 n <
		 ((CAMERIC_MAX_LSC_SECTORS + 1) * (CAMERIC_MAX_LSC_SECTORS + 1));
		 n += CAMERIC_MAX_LSC_SECTORS + 1) {
		/* 17 sectors with 2 values in one DWORD = 9 DWORDs (8 steps + 1 outside loop) */
		for (i = 0; i < (CAMERIC_MAX_LSC_SECTORS); i += 2) {
			isp_write_reg(dev, REG_ADDR(isp_lsc_r_table_data),
					  lsc->r[n +
						 i] | (lsc->r[n + i + 1] << 12));
			isp_write_reg(dev, REG_ADDR(isp_lsc_gr_table_data),
					  lsc->gr[n +
						  i] | (lsc->gr[n + i + 1] << 12));
			isp_write_reg(dev, REG_ADDR(isp_lsc_gb_table_data),
					  lsc->gb[n +
						  i] | (lsc->gb[n + i + 1] << 12));
			isp_write_reg(dev, REG_ADDR(isp_lsc_b_table_data),
					  lsc->b[n +
						 i] | (lsc->b[n + i + 1] << 12));
		}
		isp_write_reg(dev, REG_ADDR(isp_lsc_r_table_data),
				  lsc->r[n + CAMERIC_MAX_LSC_SECTORS]);
		isp_write_reg(dev, REG_ADDR(isp_lsc_gr_table_data),
				  lsc->gr[n + CAMERIC_MAX_LSC_SECTORS]);
		isp_write_reg(dev, REG_ADDR(isp_lsc_gb_table_data),
				  lsc->gb[n + CAMERIC_MAX_LSC_SECTORS]);
		isp_write_reg(dev, REG_ADDR(isp_lsc_b_table_data),
				  lsc->b[n + CAMERIC_MAX_LSC_SECTORS]);
	}

	isp_write_reg(dev, REG_ADDR(isp_lsc_table_sel),
			  (isp_lsc_status & 0x2U) ? 0U : 1U);

	return 0;
}


int isp_s_lsc_sec(struct isp_ic_dev *dev)
{
	int i;
	struct isp_lsc_context *lsc = (&dev->lsc);

	for (i = 0; i < CAEMRIC_GRAD_TBL_SIZE; i += 2) {
		isp_write_reg(dev, REG_ADDR(isp_lsc_xsize_01) + i * 2,
				  (lsc->x_size[i] & MRV_LSC_Y_SECT_SIZE_0_MASK) |
				  ((lsc->x_size[i + 1]
				<< MRV_LSC_X_SECT_SIZE_1_SHIFT)
				   & MRV_LSC_X_SECT_SIZE_1_MASK));
		isp_write_reg(dev, REG_ADDR(isp_lsc_ysize_01) + i * 2,
				  (lsc->y_size[i] & MRV_LSC_Y_SECT_SIZE_0_MASK) |
				  ((lsc->y_size[i + 1]
				<< MRV_LSC_Y_SECT_SIZE_1_SHIFT)
				   & MRV_LSC_Y_SECT_SIZE_1_MASK));
		isp_write_reg(dev, REG_ADDR(isp_lsc_xgrad_01) + i * 2,
				  (lsc->x_grad[i] & MRV_LSC_XGRAD_0_MASK) |
				  ((lsc->x_grad[i + 1]
				<< MRV_LSC_XGRAD_1_SHIFT)
				   & MRV_LSC_XGRAD_1_MASK));
		isp_write_reg(dev, REG_ADDR(isp_lsc_ygrad_01) + i * 2,
				  (lsc->y_grad[i] & MRV_LSC_YGRAD_0_MASK) |
				  ((lsc->y_grad[i + 1]
				<< MRV_LSC_YGRAD_1_SHIFT)
				   & MRV_LSC_YGRAD_1_MASK));
	}

	return 0;
}

int isp_ioc_read_reg(struct isp_ic_dev *dev, void *__user args)
{
	struct isp_reg_t reg;

	viv_check_retval(copy_from_user(&reg, args, sizeof(reg)));
	reg.val = isp_read_reg(dev, reg.offset);
	viv_check_retval(copy_to_user(args, &reg, sizeof(reg)));
	return 0;
}

int isp_ioc_write_reg(struct isp_ic_dev *dev, void *__user args)
{
	struct isp_reg_t reg;

	viv_check_retval((copy_from_user(&reg, args, sizeof(reg))));
	isp_write_reg(dev, reg.offset, reg.val);
	return 0;
}

int isp_ioc_disable_isp_off(struct isp_ic_dev *dev, void *args)
{
	u32 isp_imsc;

	isp_info("enter %s\n", __func__);
	isp_imsc = isp_read_reg(dev, REG_ADDR(isp_imsc));
	isp_imsc &= ~MRV_ISP_IMSC_ISP_OFF_MASK;
	isp_write_reg(dev, REG_ADDR(isp_imsc), isp_imsc);

	return 0;
}

int isp_g_awbmean(struct isp_ic_dev *dev, struct isp_awb_mean *mean)
{
	u32 reg = isp_read_reg(dev, REG_ADDR(isp_awb_mean));

	mean->g = REG_GET_SLICE(reg, MRV_ISP_AWB_MEAN_Y__G);
	mean->b = REG_GET_SLICE(reg, MRV_ISP_AWB_MEAN_CB__B);
	mean->r = REG_GET_SLICE(reg, MRV_ISP_AWB_MEAN_CR__R);
	mean->no_white_count = isp_read_reg(dev, REG_ADDR(isp_awb_white_cnt));

	return 0;
}

int isp_s_ee(struct isp_ic_dev *dev)
{
#ifndef ISP_EE
	return -EINVAL;
#else
	struct isp_ee_context *ee = &dev->ee;
	u32 isp_ee_ctrl = isp_read_reg(dev, REG_ADDR(isp_ee_ctrl));
	u32 gain = 0;

	isp_info("enter %s\n", __func__);

	if (!ee->enable) {
		isp_write_reg(dev, REG_ADDR(isp_ee_ctrl),
				  isp_ee_ctrl & ~EE_CTRL_ENABLE_MASK);
		return 0;
	}

	REG_SET_SLICE(isp_ee_ctrl, EE_CTRL_INPUT_SEL, ee->input_sel);
	REG_SET_SLICE(isp_ee_ctrl, EE_CTRL_SOURCE_STRENGTH, ee->src_strength);
	REG_SET_SLICE(isp_ee_ctrl, EE_CTRL_STRENGTH, ee->strength);
	REG_SET_SLICE(gain, EE_UV_GAIN, ee->uv_gain);
	REG_SET_SLICE(gain, EE_EDGE_GAIN, ee->edge_gain);
	isp_write_reg(dev, REG_ADDR(isp_ee_y_gain), ee->y_gain);
	isp_write_reg(dev, REG_ADDR(isp_ee_uv_gain), gain);
	isp_write_reg(dev, REG_ADDR(isp_ee_ctrl),
			  isp_ee_ctrl | EE_CTRL_ENABLE_MASK);
	return 0;
#endif
}

int isp_s_exp(struct isp_ic_dev *dev)
{
	struct isp_exp_context *exp = &dev->exp;
	u32 isp_exp_ctrl = isp_read_reg(dev, REG_ADDR(isp_exp_ctrl));
	u32 isp_imsc = isp_read_reg(dev, REG_ADDR(isp_imsc));

	isp_info("enter %s\n", __func__);

	if (!exp->enable) {
		REG_SET_SLICE(isp_exp_ctrl, MRV_AE_EXP_START, 0);
		isp_write_reg(dev, REG_ADDR(isp_exp_ctrl), isp_exp_ctrl);
		isp_write_reg(dev, REG_ADDR(isp_imsc),
				  isp_imsc & ~MRV_ISP_IMSC_EXP_END_MASK);
		return 0;
	}

	isp_write_reg(dev, REG_ADDR(isp_exp_h_offset),
			  (MRV_AE_ISP_EXP_H_OFFSET_MASK & exp->window.x));
	isp_write_reg(dev, REG_ADDR(isp_exp_v_offset),
			  (MRV_AE_ISP_EXP_V_OFFSET_MASK & exp->window.y));
	isp_write_reg(dev, REG_ADDR(isp_exp_h_size),
			  (MRV_AE_ISP_EXP_H_SIZE_MASK & exp->window.width));
	isp_write_reg(dev, REG_ADDR(isp_exp_v_size),
			  (MRV_AE_ISP_EXP_V_SIZE_MASK & exp->window.height));
#ifdef ISP_AE_SHADOW
	isp_write_reg(dev, REG_ADDR(isp_exp_h_offset_shd),
			  (MRV_AE_ISP_EXP_H_OFFSET_MASK & exp->window.x));
	isp_write_reg(dev, REG_ADDR(isp_exp_v_offset_shd),
			  (MRV_AE_ISP_EXP_V_OFFSET_MASK & exp->window.y));
	isp_write_reg(dev, REG_ADDR(isp_exp_h_size_shd),
			  (MRV_AE_ISP_EXP_H_SIZE_MASK & exp->window.width));
	isp_write_reg(dev, REG_ADDR(isp_exp_v_size_shd),
			  (MRV_AE_ISP_EXP_V_SIZE_MASK & exp->window.height));
#endif
	REG_SET_SLICE(isp_exp_ctrl, MRV_AE_EXP_MEAS_MODE, exp->mode);
	REG_SET_SLICE(isp_exp_ctrl, MRV_AE_EXP_START, 1);
	isp_write_reg(dev, REG_ADDR(isp_exp_ctrl), isp_exp_ctrl);
	isp_write_reg(dev, REG_ADDR(isp_imsc),
			  isp_imsc | MRV_ISP_IMSC_EXP_END_MASK);

	return 0;
}

int isp_g_expmean(struct isp_ic_dev *dev, u8 *mean)
{
	int i = 0;

	if (!dev || !mean)
		return -EINVAL;
	for (; i < 25; i++) {
		mean[i] = isp_read_reg(dev, REG_ADDR(isp_exp_mean_00) + i * 4);
	}

	return 0;
}

#ifdef ISP_HIST256
#define HIST_BIN_TOTAL 256
#else
#define HIST_BIN_TOTAL 16
#endif

int isp_s_hist(struct isp_ic_dev *dev)
{
	struct isp_hist_context *hist = &dev->hist;

#ifdef ISP_HIST256
	u32 isp_hist256_prop = isp_read_reg(dev, REG_ADDR(isp_hist256_prop));
	u32 isp_imsc = isp_read_reg(dev, REG_ADDR(isp_imsc));
	int i;

	if (!hist->enable) {
		REG_SET_SLICE(isp_hist256_prop, MRV_HIST_MODE,
				  MRV_HIST_MODE_NONE);
		isp_write_reg(dev, REG_ADDR(isp_hist256_prop),
				  isp_hist256_prop);
		isp_write_reg(dev, REG_ADDR(isp_imsc),
				  isp_imsc & ~MRV_ISP_IMSC_HIST_MEASURE_RDY_MASK);
		return 0;
	}

	isp_write_reg(dev, REG_ADDR(isp_hist256_h_offs),
			  (MRV_HIST_H_OFFSET_MASK & hist->window.x));
	isp_write_reg(dev, REG_ADDR(isp_hist256_v_offs),
			  (MRV_HIST_V_OFFSET_MASK & hist->window.y));
	isp_write_reg(dev, REG_ADDR(isp_hist256_h_size),
			  (MRV_HIST_H_SIZE_MASK & hist->window.width));
	isp_write_reg(dev, REG_ADDR(isp_hist256_v_size),
			  (MRV_HIST_V_SIZE_MASK & hist->window.height));

	for (i = 0; i < 24; i += 4) {
		isp_write_reg(dev, REG_ADDR(isp_hist256_weight_00to30) + i,
				  hist->weight[i +
					   0] | (hist->weight[i +
								  1] << 8) |
				  (hist->weight[i + 2] << 16) | (hist->weight[i +
									  3] <<
								 24));
	}

	isp_write_reg(dev, REG_ADDR(isp_hist256_weight_44), hist->weight[24]);
	REG_SET_SLICE(isp_hist256_prop, MRV_HIST_STEPSIZE, hist->step_size);
	REG_SET_SLICE(isp_hist256_prop, MRV_HIST_MODE, hist->mode);
	isp_write_reg(dev, REG_ADDR(isp_hist256_prop), isp_hist256_prop);
	isp_write_reg(dev, REG_ADDR(isp_imsc),
			  isp_imsc | MRV_ISP_IMSC_HIST_MEASURE_RDY_MASK);
#else
	u32 isp_hist_prop = isp_read_reg(dev, REG_ADDR(isp_hist_prop));
	u32 isp_imsc = isp_read_reg(dev, REG_ADDR(isp_imsc));
	int i;

	isp_info("enter %s\n", __func__);
	if (!hist->enable) {
		REG_SET_SLICE(isp_hist_prop, MRV_HIST_MODE, MRV_HIST_MODE_NONE);
		isp_write_reg(dev, REG_ADDR(isp_hist_prop), isp_hist_prop);
		isp_write_reg(dev, REG_ADDR(isp_imsc),
				  isp_imsc & ~MRV_ISP_IMSC_HIST_MEASURE_RDY_MASK);
		return 0;
	}

	isp_write_reg(dev, REG_ADDR(isp_hist_h_offs),
			  (MRV_HIST_H_OFFSET_MASK & hist->window.x));
	isp_write_reg(dev, REG_ADDR(isp_hist_v_offs),
			  (MRV_HIST_V_OFFSET_MASK & hist->window.y));
	isp_write_reg(dev, REG_ADDR(isp_hist_h_size),
			  (MRV_HIST_H_SIZE_MASK & hist->window.width));
	isp_write_reg(dev, REG_ADDR(isp_hist_v_size),
			  (MRV_HIST_V_SIZE_MASK & hist->window.height));

	for (i = 0; i < 24; i += 4) {
		isp_write_reg(dev, REG_ADDR(isp_hist_weight_00to30) + i,
				  hist->weight[i +
					   0] | (hist->weight[i +
								  1] << 8) |
				  (hist->weight[i + 2] << 16) | (hist->weight[i +
									  3] <<
								 24));
	}

	isp_write_reg(dev, REG_ADDR(isp_hist_weight_44), hist->weight[24]);
	REG_SET_SLICE(isp_hist_prop, MRV_HIST_STEPSIZE, hist->step_size);
	REG_SET_SLICE(isp_hist_prop, MRV_HIST_MODE, hist->mode);
	isp_write_reg(dev, REG_ADDR(isp_hist_prop), isp_hist_prop);
	isp_write_reg(dev, REG_ADDR(isp_imsc),
			  isp_imsc | MRV_ISP_IMSC_HIST_MEASURE_RDY_MASK);
#endif
	return 0;
}

int isp_g_histmean(struct isp_ic_dev *dev, u32 *mean)
{
	int i = 0;

	if (!dev || !mean)
		return -EINVAL;
#ifdef ISP_HIST256
	for (; i < HIST_BIN_TOTAL; i++) {
		mean[i] = isp_read_reg(dev, REG_ADDR(isp_hist256_bin_n));
	}
#else
	for (; i < HIST_BIN_TOTAL; i++) {
		mean[i] = isp_read_reg(dev,
					   REG_ADDR(histogram_measurement_result_arr[i]));

	}
#endif
	return 0;
}

int isp_s_ge(struct isp_ic_dev *dev)
{
#ifndef ISP_GREENEQUILIBRATE
	return -1;
#else
	struct isp_ge_context *ge = &dev->ge;
	u32 green_equilibrate_ctrl =
		isp_read_reg(dev, REG_ADDR(green_equilibrate_ctrl));
	u32 green_equilibrate_hcnt_dummy = 0;

	isp_info("enter %s\n", __func__);

	if (!ge->enable) {
		REG_SET_SLICE(green_equilibrate_ctrl,
				  ISP_GREEN_EQUILIBTATE_ENABLE, 0);
		isp_write_reg(dev, REG_ADDR(green_equilibrate_ctrl),
				  green_equilibrate_ctrl);
		return 0;
	}

	REG_SET_SLICE(green_equilibrate_ctrl, ISP_GREEN_EQUILIBTATE_TH,
			  ge->threshold);
	REG_SET_SLICE(green_equilibrate_ctrl, ISP_GREEN_EQUILIBTATE_ENABLE, 1);
	REG_SET_SLICE(green_equilibrate_hcnt_dummy,
			  ISP_GREEN_EQUILIBTATE_HCNT_DUMMY, ge->h_dummy);
	isp_write_reg(dev, REG_ADDR(green_equilibrate_ctrl),
			  green_equilibrate_ctrl);
	isp_write_reg(dev, REG_ADDR(green_equilibrate_hcnt_dummy),
			  green_equilibrate_hcnt_dummy);
	isp_write_reg(dev, REG_ADDR(green_equilibrate_ctrl_shd),
			  green_equilibrate_ctrl);
	return 0;
#endif
}

int isp_s_ca(struct isp_ic_dev *dev)
{
#ifndef ISP_CA
	return -1;
#else
	struct isp_ca_context *ca = &dev->ca;
	u32 isp_curve_ctrl = isp_read_reg(dev, REG_ADDR(isp_curve_ctrl));
	int i = 0;
	isp_info("enter %s\n", __func__);
	if (!ca->enable) {
		REG_SET_SLICE(isp_curve_ctrl, ISP_CURVE_ENABLE, 0);
		isp_write_reg(dev, REG_ADDR(isp_curve_ctrl), isp_curve_ctrl);
		return 0;
	}

	isp_write_reg(dev, REG_ADDR(isp_curve_lut_x_addr), 0);
	isp_write_reg(dev, REG_ADDR(isp_curve_lut_luma_addr), 0);
	isp_write_reg(dev, REG_ADDR(isp_curve_lut_chroma_addr), 0);
	isp_write_reg(dev, REG_ADDR(isp_curve_lut_shift_addr), 0);

	for (i = 0; i < CA_CURVE_DATA_TABLE_LEN; i++) {
		isp_write_reg(dev, REG_ADDR(isp_curve_lut_x_write_data),
				  dev->ca.lut_x[i]);
		isp_write_reg(dev, REG_ADDR(isp_curve_lut_luma_write_data),
				  dev->ca.lut_luma[i]);
		isp_write_reg(dev, REG_ADDR(isp_curve_lut_chroma_write_data),
				  dev->ca.lut_chroma[i]);
		isp_write_reg(dev, REG_ADDR(isp_curve_lut_shift_write_data),
				  dev->ca.lut_shift[i]);
	}
	REG_SET_SLICE(isp_curve_ctrl, ISP_CURVE_MODE, dev->ca.mode);
	REG_SET_SLICE(isp_curve_ctrl, ISP_CURVE_ENABLE, 1);
	isp_write_reg(dev, REG_ADDR(isp_curve_ctrl), isp_curve_ctrl);

	return 0;
#endif
}

int isp_s_dpcc(struct isp_ic_dev *dev)
{
	struct isp_dpcc_context *dpcc = &dev->dpcc;
	const u32 reg_gap = 20;
	int i;
	u32 isp_dpcc_mode = isp_read_reg(dev, REG_ADDR(isp_dpcc_mode));

	isp_info("enter %s\n", __func__);

	if (!dpcc->enable) {
		REG_SET_SLICE(isp_dpcc_mode, MRV_DPCC_ISP_DPCC_ENABLE, 0);
	} else {
		REG_SET_SLICE(isp_dpcc_mode, MRV_DPCC_ISP_DPCC_ENABLE, 1);
	}

	isp_write_reg(dev, REG_ADDR(isp_dpcc_mode), dpcc->mode);
	isp_write_reg(dev, REG_ADDR(isp_dpcc_output_mode), dpcc->outmode);
	isp_write_reg(dev, REG_ADDR(isp_dpcc_set_use), dpcc->set_use);

	for (i = 0; i < 3; i++) {
		isp_write_reg(dev, REG_ADDR(isp_dpcc_methods_set_1) + i * 4,
				  0x1FFF & dpcc->methods_set[i]);
		isp_write_reg(dev,
				  REG_ADDR(isp_dpcc_line_thresh_1) + i * reg_gap,
				  0xFFFF & dpcc->params[i].line_thresh);
		isp_write_reg(dev,
				  REG_ADDR(isp_dpcc_line_mad_fac_1) + i * reg_gap,
				  0x3F3F & dpcc->params[i].line_mad_fac);
		isp_write_reg(dev, REG_ADDR(isp_dpcc_pg_fac_1) + i * reg_gap,
				  0x3F3F & dpcc->params[i].pg_fac);
		isp_write_reg(dev,
				  REG_ADDR(isp_dpcc_rnd_thresh_1) + i * reg_gap,
				  0xFFFF & dpcc->params[i].rnd_thresh);
		isp_write_reg(dev, REG_ADDR(isp_dpcc_rg_fac_1) + i * reg_gap,
				  0x3F3F & dpcc->params[i].rg_fac);
	}

	isp_write_reg(dev, REG_ADDR(isp_dpcc_ro_limits), dpcc->ro_limits);
	isp_write_reg(dev, REG_ADDR(isp_dpcc_rnd_offs), dpcc->rnd_offs);
	isp_write_reg(dev, REG_ADDR(isp_dpcc_mode), isp_dpcc_mode);

	return 0;
}


int isp_s_flt(struct isp_ic_dev *dev)
{
	struct flt_denoise_type {
		u32 thresh_sh0;
		u32 thresh_sh1;
		u32 thresh_bl0;
		u32 thresh_bl1;
		u32 stage_select;
	};

	struct flt_sharpen_type {
		u32 fac_sh0;
		u32 fac_sh1;
		u32 fac_mid;
		u32 fac_bl0;
		u32 fac_bl1;
	};

	static struct flt_denoise_type denoise_tbl[] = {
		{0, 0, 0, 0, 6},
		{18,33,8,2,6},
		{26,44,13,5,4},
		{36,51,23,10,2},
		{41,67,26,15,3},
		{75,10,50,20,3},
		{90, 120, 60, 26, 2},
		{120, 150, 80, 51, 2},
		{170, 200, 140, 100, 2},
		{250, 300, 180, 150, 2},
		{1023, 1023, 1023, 1023, 2},
		{1023, 1023, 1023, 1023, 0},
	};

	static struct flt_sharpen_type sharpen_tbl[] = {
		{0x4, 0x4, 0x4, 0x2, 0x0},
		{0x7, 0x8, 0x6, 0x2, 0x0},
		{0xA, 0xC, 0x8, 0x4, 0x0},
		{0xC, 0x10, 0xA, 0x6, 0x2},
		{0x16, 0x16, 0xC, 0x8, 0x4},
		{0x14, 0x1B, 0x10, 0xA, 0x4},
		{0x1A, 0x20, 0x13, 0xC, 0x6},
		{0x1E, 0x26, 0x17, 0x10, 0x8},
		{0x24, 0x2C, 0x1D, 0x15, 0x0D},
		{0x2A, 0x30, 0x22, 0x1A, 0x14},
		{0x30, 0x3F, 0x28, 0x24, 0x20},
	};

	struct isp_flt_context *flt = &dev->flt;
	u32 isp_flt_mode = isp_read_reg(dev, REG_ADDR(isp_filt_mode));
	if (!flt->enable) {
		isp_write_reg(dev, REG_ADDR(isp_filt_thresh_sh0), 0);
		isp_write_reg(dev, REG_ADDR(isp_filt_thresh_sh1), 0);
		isp_write_reg(dev, REG_ADDR(isp_filt_thresh_bl0), 0);
		isp_write_reg(dev, REG_ADDR(isp_filt_thresh_bl1), 0);

		isp_write_reg(dev, REG_ADDR(isp_filt_fac_sh0), 0);
		isp_write_reg(dev, REG_ADDR(isp_filt_fac_sh1), 0);
		isp_write_reg(dev, REG_ADDR(isp_filt_fac_mid), 0);
		isp_write_reg(dev, REG_ADDR(isp_filt_fac_bl0), 0);
		isp_write_reg(dev, REG_ADDR(isp_filt_fac_bl1), 0);

		REG_SET_SLICE(isp_flt_mode, MRV_FILT_FILT_ENABLE, 0);
		isp_write_reg(dev, REG_ADDR(isp_filt_mode), isp_flt_mode);
		dev->flt.changed = false;
		return 0;
	}

	if (flt->denoise >= 0) {
		isp_write_reg(dev, REG_ADDR(isp_filt_thresh_sh0),
					denoise_tbl[flt->denoise].thresh_sh0);
		isp_write_reg(dev, REG_ADDR(isp_filt_thresh_sh1),
					denoise_tbl[flt->denoise].thresh_sh1);
		isp_write_reg(dev, REG_ADDR(isp_filt_thresh_bl0),
					denoise_tbl[flt->denoise].thresh_bl0);
		isp_write_reg(dev, REG_ADDR(isp_filt_thresh_bl1),
					denoise_tbl[flt->denoise].thresh_bl1);
		REG_SET_SLICE(isp_flt_mode, MRV_FILT_STAGE1_SELECT,
					denoise_tbl[flt->denoise].stage_select);

		REG_SET_SLICE(isp_flt_mode, MRV_FILT_FILT_CHR_V_MODE,
					flt->chrV);
		REG_SET_SLICE(isp_flt_mode, MRV_FILT_FILT_CHR_H_MODE,
					flt->chrH);
	}

	if (flt->sharpen >= 0) {
		isp_write_reg(dev, REG_ADDR(isp_filt_fac_sh0),
					sharpen_tbl[flt->sharpen].fac_sh0);
		isp_write_reg(dev, REG_ADDR(isp_filt_fac_sh1),
					sharpen_tbl[flt->sharpen].fac_sh1);
		isp_write_reg(dev, REG_ADDR(isp_filt_fac_mid),
					sharpen_tbl[flt->sharpen].fac_mid);
		isp_write_reg(dev, REG_ADDR(isp_filt_fac_bl0),
					sharpen_tbl[flt->sharpen].fac_bl0);
		isp_write_reg(dev, REG_ADDR(isp_filt_fac_bl1),
					sharpen_tbl[flt->sharpen].fac_bl1);
	}

	REG_SET_SLICE(isp_flt_mode, MRV_FILT_FILT_MODE,
				MRV_FILT_FILT_MODE_DYNAMIC);
	isp_write_reg(dev, REG_ADDR(isp_filt_mode), isp_flt_mode);
	REG_SET_SLICE(isp_flt_mode, MRV_FILT_FILT_ENABLE, 1);
	isp_write_reg(dev, REG_ADDR(isp_filt_mode), isp_flt_mode);
	isp_write_reg(dev, REG_ADDR(isp_filt_lum_weight), 0x00032040);
	flt->changed = false;

	return 0;
}

int isp_s_flt_ctrl(struct isp_ic_dev *dev)
{
	unsigned long flags;
	int ret = 0;
	spin_lock_irqsave(&dev->irqlock, flags);
	if (!is_isp_enable(dev)) {
		ret =  isp_s_flt(dev);
	} else {
		dev->flt.changed = true;
	}
	spin_unlock_irqrestore(&dev->irqlock, flags);

	return ret;
}

int isp_s_cac(struct isp_ic_dev *dev)
{
	struct isp_cac_context *cac = &dev->cac;
	u32 val = 0;
	u32 isp_cac_ctrl = isp_read_reg(dev, REG_ADDR(isp_cac_ctrl));

	isp_info("enter %s\n", __func__);

	if (!cac->enable) {
		REG_SET_SLICE(isp_cac_ctrl, MRV_CAC_CAC_EN, 0);
		isp_write_reg(dev, REG_ADDR(isp_cac_ctrl), isp_cac_ctrl);
		return 0;
	}

	REG_SET_SLICE(isp_cac_ctrl, MRV_CAC_H_CLIP_MODE, cac->hmode);
	REG_SET_SLICE(isp_cac_ctrl, MRV_CAC_V_CLIP_MODE, cac->vmode);
	isp_write_reg(dev, REG_ADDR(isp_cac_count_start),
			  cac->hstart | (cac->vstart << 16));
	isp_write_reg(dev, REG_ADDR(isp_cac_a), cac->ar | (cac->ab << 16));
	isp_write_reg(dev, REG_ADDR(isp_cac_b), cac->br | (cac->bb << 16));
	isp_write_reg(dev, REG_ADDR(isp_cac_c), cac->cr | (cac->cb << 16));

	REG_SET_SLICE(val, MRV_CAC_X_NS, cac->xns);
	REG_SET_SLICE(val, MRV_CAC_X_NF, cac->xnf);
	isp_write_reg(dev, REG_ADDR(isp_cac_x_norm), val);
	val = 0;
	REG_SET_SLICE(val, MRV_CAC_Y_NS, cac->yns);
	REG_SET_SLICE(val, MRV_CAC_Y_NF, cac->ynf);
	isp_write_reg(dev, REG_ADDR(isp_cac_y_norm), val);
	REG_SET_SLICE(isp_cac_ctrl, MRV_CAC_CAC_EN, 1);
	isp_write_reg(dev, REG_ADDR(isp_cac_ctrl), isp_cac_ctrl);

	return 0;
}

int isp_s_deg(struct isp_ic_dev *dev)
{
	struct isp_deg_context *deg = &dev->deg;
	int i;
	u32 isp_gamma_dx_lo = 0;
	u32 isp_gamma_dx_hi = 0;
	u32 isp_ctrl = isp_read_reg(dev, REG_ADDR(isp_ctrl));

	isp_info("enter %s\n", __func__);

	if (!deg->enable) {
		REG_SET_SLICE(isp_ctrl, MRV_ISP_ISP_GAMMA_IN_ENABLE, 0);
		isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);
		return 0;
	}

	for (i = 0; i < 8; i++) {
		isp_gamma_dx_lo |= deg->segment[i] << (i * 4);
		isp_gamma_dx_hi |= deg->segment[i + 8] << (i * 4);
	}

	isp_write_reg(dev, REG_ADDR(isp_gamma_dx_lo), isp_gamma_dx_lo);
	isp_write_reg(dev, REG_ADDR(isp_gamma_dx_hi), isp_gamma_dx_hi);

	for (i = 0; i < 17; i++) {
		isp_write_reg(dev, REG_ADDR(degamma_r_y_block_arr[i]),
				  deg->r[i]);
		isp_write_reg(dev, REG_ADDR(degamma_g_y_block_arr[i]),
				  deg->g[i]);
		isp_write_reg(dev, REG_ADDR(degamma_b_y_block_arr[i]),
				  deg->b[i]);
	}

	REG_SET_SLICE(isp_ctrl, MRV_ISP_ISP_GAMMA_IN_ENABLE, 1);
	isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);

	return 0;
}

u32 get_eff_coeff(int decimal)
{
	u32 value = 0;

	if (decimal <= -6)
		value = 15;
	else if (decimal <= -3)
		value = 14;
	else if (decimal == -2)
		value = 13;
	else if (decimal == -1)
		value = 12;
	else if (decimal == 0)
		value = 0;
	else if (decimal == 1)
		value = 8;
	else if (decimal == 2)
		value = 9;
	else if (decimal < 6)
		value = 10;
	else
		value = 11;

	return value;
}

int isp_s_ie(struct isp_ic_dev *dev)
{
	struct isp_ie_context *ie = &dev->ie;
	u32 img_eff_ctrl = isp_read_reg(dev, REG_ADDR(img_eff_ctrl));
	u32 vi_iccl = isp_read_reg(dev, REG_ADDR(vi_iccl));
	u32 vi_ircl = isp_read_reg(dev, REG_ADDR(vi_ircl));
	u32 img_eff_tint = isp_read_reg(dev, REG_ADDR(img_eff_tint));
	u32 img_eff_color_sel = isp_read_reg(dev, REG_ADDR(img_eff_color_sel));
	u32 mat[9];
	u32 sharpen = 0;
	int i;

	isp_info("enter %s\n", __func__);

	REG_SET_SLICE(vi_ircl, MRV_VI_IE_SOFT_RST, 1);
	isp_write_reg(dev, REG_ADDR(vi_ircl), vi_ircl);

	if (!ie->enable) {
		REG_SET_SLICE(img_eff_ctrl, MRV_IMGEFF_CFG_UPD,
				  MRV_IMGEFF_CFG_UPD_UPDATE);
		REG_SET_SLICE(img_eff_ctrl, MRV_IMGEFF_BYPASS_MODE,
				  MRV_IMGEFF_BYPASS_MODE_BYPASS);
		REG_SET_SLICE(vi_iccl, MRV_VI_IE_CLK_ENABLE, 0);
		isp_write_reg(dev, REG_ADDR(vi_iccl), vi_iccl);
		isp_write_reg(dev, REG_ADDR(img_eff_ctrl), img_eff_ctrl);
		return 0;
	}

	REG_SET_SLICE(vi_ircl, MRV_VI_IE_SOFT_RST, 0);
	isp_write_reg(dev, REG_ADDR(vi_ircl), vi_ircl);

	REG_SET_SLICE(vi_iccl, MRV_VI_IE_CLK_ENABLE, 1);
	isp_write_reg(dev, REG_ADDR(vi_iccl), vi_iccl);
	REG_SET_SLICE(img_eff_ctrl, MRV_IMGEFF_EFFECT_MODE, ie->mode);
	REG_SET_SLICE(img_eff_ctrl, MRV_IMGEFF_FULL_RANGE, ie->full_range);

	for (i = 0; i < 9; i++)
		mat[i] = get_eff_coeff(ie->m[i]);

	if (ie->mode == MRV_IMGEFF_EFFECT_MODE_SEPIA) {
		img_eff_tint = isp_read_reg(dev, REG_ADDR(img_eff_tint));
		REG_SET_SLICE(img_eff_tint, MRV_IMGEFF_INCR_CR, ie->tint_cr);
		REG_SET_SLICE(img_eff_tint, MRV_IMGEFF_INCR_CB, ie->tint_cb);
		isp_write_reg(dev, REG_ADDR(img_eff_tint), img_eff_tint);
	} else if (ie->mode == MRV_IMGEFF_EFFECT_MODE_COLOR_SEL) {
		REG_SET_SLICE(img_eff_color_sel, MRV_IMGEFF_COLOR_SELECTION,
				  ie->color_sel);
		REG_SET_SLICE(img_eff_color_sel, MRV_IMGEFF_COLOR_THRESHOLD,
				  ie->color_thresh);
		isp_write_reg(dev, REG_ADDR(img_eff_color_sel),
				  img_eff_color_sel);
	} else if (ie->mode == MRV_IMGEFF_EFFECT_MODE_EMBOSS) {
		isp_write_reg(dev, REG_ADDR(img_eff_mat_1),
				  mat[0] | (mat[1] << 4) | (mat[2] << 8) | (mat[3]
									<< 12));
		isp_write_reg(dev, REG_ADDR(img_eff_mat_2),
				  mat[4] | (mat[5] << 4) | (mat[6] << 8) | (mat[7]
									<< 12));
		isp_write_reg(dev, REG_ADDR(img_eff_mat_3), mat[8]);
	} else if (ie->mode == MRV_IMGEFF_EFFECT_MODE_SKETCH ||
		   ie->mode == MRV_IMGEFF_EFFECT_MODE_SHARPEN) {
		isp_write_reg(dev, REG_ADDR(img_eff_mat_3),
				  (mat[0] << 4) | (mat[1] << 8) | (mat[2] << 12));
		isp_write_reg(dev, REG_ADDR(img_eff_mat_4),
				  mat[3] | (mat[4] << 4) | (mat[5] << 8) | (mat[6]
									<< 12));
		isp_write_reg(dev, REG_ADDR(img_eff_mat_5),
				  mat[7] | (mat[8] << 4));
		REG_SET_SLICE(sharpen, MRV_IMGEFF_SHARP_FACTOR,
				  ie->sharpen_factor);
		REG_SET_SLICE(sharpen, MRV_IMGEFF_CORING_THR,
				  ie->sharpen_thresh);
		isp_write_reg(dev, REG_ADDR(img_eff_sharpen), sharpen);
	}
	REG_SET_SLICE(img_eff_ctrl, MRV_IMGEFF_CFG_UPD,
			  MRV_IMGEFF_CFG_UPD_UPDATE);
	REG_SET_SLICE(img_eff_ctrl, MRV_IMGEFF_BYPASS_MODE,
			  MRV_IMGEFF_BYPASS_MODE_PROCESS);
	isp_write_reg(dev, REG_ADDR(img_eff_ctrl), img_eff_ctrl);

	return 0;
}

int isp_s_vsm(struct isp_ic_dev *dev)
{
	struct isp_vsm_context *vsm = &dev->vsm;
	u32 isp_vsm_mode = isp_read_reg(dev, REG_ADDR(isp_vsm_mode));
	u32 isp_imsc = isp_read_reg(dev, REG_ADDR(isp_imsc));

	isp_info("enter %s\n", __func__);

	if (!vsm->enable) {
		REG_SET_SLICE(isp_vsm_mode, ISP_VSM_MEAS_EN, 0);
		REG_SET_SLICE(isp_vsm_mode, ISP_VSM_MEAS_IRQ_ENABLE, 0);
		isp_write_reg(dev, REG_ADDR(isp_vsm_mode), isp_vsm_mode);
		isp_write_reg(dev, REG_ADDR(isp_imsc),
				  isp_imsc & ~MRV_ISP_IMSC_VSM_END_MASK);
		return 0;
	}

	isp_write_reg(dev, REG_ADDR(isp_vsm_h_offs), vsm->window.x);
	isp_write_reg(dev, REG_ADDR(isp_vsm_v_offs), vsm->window.y);
	isp_write_reg(dev, REG_ADDR(isp_vsm_h_size),
			  vsm->window.width & 0xFFFFE);
	isp_write_reg(dev, REG_ADDR(isp_vsm_v_size),
			  vsm->window.height & 0xFFFFE);
	isp_write_reg(dev, REG_ADDR(isp_vsm_h_segments), vsm->h_seg);
	isp_write_reg(dev, REG_ADDR(isp_vsm_v_segments), vsm->v_seg);
	REG_SET_SLICE(isp_vsm_mode, ISP_VSM_MEAS_EN, 1);
	REG_SET_SLICE(isp_vsm_mode, ISP_VSM_MEAS_IRQ_ENABLE, 1);
	isp_write_reg(dev, REG_ADDR(isp_vsm_mode), isp_vsm_mode);
	isp_write_reg(dev, REG_ADDR(isp_imsc),
			  isp_imsc | MRV_ISP_IMSC_VSM_END_MASK);

	return 0;
}

int isp_g_vsm(struct isp_ic_dev *dev, struct isp_vsm_result *vsm)
{
	isp_info("enter %s\n", __func__);
	vsm->x = isp_read_reg(dev, REG_ADDR(isp_vsm_delta_h));
	vsm->y = isp_read_reg(dev, REG_ADDR(isp_vsm_delta_v));

	return 0;
}

u32 get_afm_shift(u32 count, u32 thresh)
{
	u32 grad = count;
	u32 shift = 0;

	while (grad > (thresh)) {
		++shift;
		grad >>= 1;
	}

	return shift;
}

int isp_s_afm(struct isp_ic_dev *dev)
{
	struct isp_afm_context *afm = &dev->afm;
	u32 mask =
		(MRV_ISP_IMSC_AFM_FIN_MASK | MRV_ISP_IMSC_AFM_LUM_OF_MASK |
		 MRV_ISP_IMSC_AFM_SUM_OF_MASK);

	u32 shift = 0;
	int i;

	u32 isp_afm_ctrl = isp_read_reg(dev, REG_ADDR(isp_afm_ctrl));
	u32 isp_imsc = isp_read_reg(dev, REG_ADDR(isp_imsc));

	isp_info("enter %s\n", __func__);

	if (!afm->enable) {
		REG_SET_SLICE(isp_afm_ctrl, MRV_AFM_AFM_EN, 0);
		isp_imsc &= ~mask;
		isp_write_reg(dev, REG_ADDR(isp_afm_ctrl), isp_afm_ctrl);
		isp_write_reg(dev, REG_ADDR(isp_imsc), isp_imsc);
		return 0;
	}

	for (i = 0; i < 3; i++) {
		isp_write_reg(dev, REG_ADDR(isp_afm_lt_a) + i * 8,
				  (afm->window[i].x << 16) | afm->window[i].y);
		isp_write_reg(dev, REG_ADDR(isp_afm_rb_a) + i * 8,
				  ((afm->window[i].x + afm->window[i].width -
				1) << 16) | ((afm->window[i].y +
						  afm->window[i].height - 1)));
	}

	REG_SET_SLICE(shift, MRV_AFM_LUM_VAR_SHIFT, afm->lum_shift);
	REG_SET_SLICE(shift, MRV_AFM_AFM_VAR_SHIFT, afm->afm_shift);
	isp_write_reg(dev, REG_ADDR(isp_afm_var_shift), shift);
	isp_write_reg(dev, REG_ADDR(isp_afm_thres), afm->thresh);
	REG_SET_SLICE(isp_afm_ctrl, MRV_AFM_AFM_EN, 1);
	isp_imsc |= mask;
	isp_write_reg(dev, REG_ADDR(isp_afm_ctrl), isp_afm_ctrl);
	isp_write_reg(dev, REG_ADDR(isp_imsc), isp_imsc);
	return 0;
}

int isp_g_afm(struct isp_ic_dev *dev, struct isp_afm_result *afm)
{
	isp_info("enter %s\n", __func__);
	afm->sum_a = isp_read_reg(dev, REG_ADDR(isp_afm_sum_a));
	afm->sum_b = isp_read_reg(dev, REG_ADDR(isp_afm_sum_b));
	afm->sum_c = isp_read_reg(dev, REG_ADDR(isp_afm_sum_c));
	afm->lum_a = isp_read_reg(dev, REG_ADDR(isp_afm_lum_a));
	afm->lum_b = isp_read_reg(dev, REG_ADDR(isp_afm_lum_b));
	afm->lum_c = isp_read_reg(dev, REG_ADDR(isp_afm_lum_c));
	return 0;
}

int isp_s_exp2(struct isp_ic_dev *dev)
{
#ifndef ISP_AEV2
	return -EINVAL;
#else
	struct isp_exp2_context *exp2 = &dev->exp2;
	u32 isp_expv2_ctrl = isp_read_reg(dev, REG_ADDR(isp_expv2_ctrl));
	u32 grid_w, grid_h;
	u32 size, offset, size_inv, weight;

	isp_info("enter %s\n", __func__);

	grid_w = (exp2->window.width / 32) - 1;
	grid_h = (exp2->window.height / 32) - 1;

	if (!exp2->enable) {
		REG_SET_SLICE(isp_expv2_ctrl, MRV_AE_ISP_EXPV2_ENABLE, 0);
		isp_write_reg(dev, REG_ADDR(isp_expv2_ctrl), isp_expv2_ctrl);
		return 0;
	}

	size = 0;
	REG_SET_SLICE(size, MRV_AE_ISP_EXPV2_SIZE_H, grid_w);
	REG_SET_SLICE(size, MRV_AE_ISP_EXPV2_SIZE_V, grid_h);
	offset = 0;
	REG_SET_SLICE(offset, MRV_AE_ISP_EXPV2_OFFSET_H, exp2->window.x);
	REG_SET_SLICE(offset, MRV_AE_ISP_EXPV2_OFFSET_V, exp2->window.y);
	size_inv = 0;
	REG_SET_SLICE(size_inv, MRV_AE_ISP_EXPV2_SIZE_INVERT_H, 65536 / grid_w);
	REG_SET_SLICE(size_inv, MRV_AE_ISP_EXPV2_SIZE_INVERT_V, 65536 / grid_h);
	weight = 0;
	REG_SET_SLICE(weight, MRV_AE_ISP_EXPV2_PIX_WEIGHT_R, exp2->r)
		REG_SET_SLICE(weight, MRV_AE_ISP_EXPV2_PIX_WEIGHT_GR, exp2->gr)
		REG_SET_SLICE(weight, MRV_AE_ISP_EXPV2_PIX_WEIGHT_GB, exp2->gb)
		REG_SET_SLICE(weight, MRV_AE_ISP_EXPV2_PIX_WEIGHT_B, exp2->b)
		isp_write_reg(dev, REG_ADDR(isp_expv2_offset), offset);
	isp_write_reg(dev, REG_ADDR(isp_expv2_size_invert), size_inv);
	isp_write_reg(dev, REG_ADDR(isp_expv2_size), size);
#ifdef ISP_AE_SHADOW
	isp_write_reg(dev, REG_ADDR(isp_expv2_offset_shd), offset);
	isp_write_reg(dev, REG_ADDR(isp_expv2_size_invert_shd), size_inv);
	isp_write_reg(dev, REG_ADDR(isp_expv2_size_shd), size);
#endif
	isp_write_reg(dev, REG_ADDR(isp_expv2_pixel_weight), weight);

	isp_write_reg(dev, REG_ADDR(miv2_mp_jdp_base_ad_init), dev->exp2.pa);
	isp_write_reg(dev, REG_ADDR(miv2_mp_jdp_size_init), AEV2_DMA_SIZE);
	isp_write_reg(dev, REG_ADDR(miv2_mp_jdp_offs_cnt_init), 0);
	isp_write_reg(dev, REG_ADDR(miv2_mp_jdp_llength), AEV2_DMA_SIZE);
	isp_write_reg(dev, REG_ADDR(miv2_mp_jdp_pic_width), AEV2_DMA_SIZE / 4);
	isp_write_reg(dev, REG_ADDR(miv2_mp_jdp_pic_height), 1);
	isp_write_reg(dev, REG_ADDR(miv2_mp_jdp_pic_size), AEV2_DMA_SIZE);
	REG_SET_SLICE(isp_expv2_ctrl, MRV_AE_ISP_EXPV2_ENABLE, 1);
	isp_write_reg(dev, REG_ADDR(isp_expv2_ctrl), isp_expv2_ctrl);

	return 0;
#endif
}

int isp_s_2dnr(struct isp_ic_dev *dev)
{
#ifndef ISP_2DNR
	return -EINVAL;
#else
	struct isp_2dnr_context *dnr2 = &dev->dnr2;
	u32 isp_denoise2d_control =
		isp_read_reg(dev, REG_ADDR(isp_denoise2d_control));
	u32 value, addr, strength;
	u32 isp_ctrl;
	int i;

	isp_info("enter %s\n", __func__);

	if (!dnr2->enable) {
		REG_SET_SLICE(isp_denoise2d_control, ISP_2DNR_ENABLE, 0);
		isp_write_reg(dev, REG_ADDR(isp_denoise2d_control),
				  isp_denoise2d_control);
		return 0;
	}

	strength = isp_read_reg(dev, REG_ADDR(isp_denoise2d_strength));
	REG_SET_SLICE(strength, ISP_2DNR_PRGAMMA_STRENGTH, dnr2->pre_gamma);
	REG_SET_SLICE(strength, ISP_2DNR_STRENGTH, dnr2->strength);
	isp_write_reg(dev, REG_ADDR(isp_denoise2d_strength), strength);

	addr = REG_ADDR(isp_denoise2d_sigma_y[0]);
	for (i = 0; i < 60; i += 5) {
		value = 0;
		REG_SET_SLICE(value, ISP_2DNR_SIGMAY0, dnr2->sigma[i]);
		REG_SET_SLICE(value, ISP_2DNR_SIGMAY1, dnr2->sigma[i + 1]);
		REG_SET_SLICE(value, ISP_2DNR_SIGMAY2A,
				  dnr2->sigma[i + 2] >> 6);
		isp_write_reg(dev, addr, value);
		value = 0;
		addr += 4;
		REG_SET_SLICE(value, ISP_2DNR_SIGMAY2B,
				  dnr2->sigma[i + 2] & 0x3f);
		REG_SET_SLICE(value, ISP_2DNR_SIGMAY0, dnr2->sigma[i + 3]);
		REG_SET_SLICE(value, ISP_2DNR_SIGMAY1, dnr2->sigma[i + 4]);
		isp_write_reg(dev, addr, value);
		addr += 4;
	}

	isp_ctrl = isp_read_reg(dev, REG_ADDR(isp_ctrl));

	isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);
	REG_SET_SLICE(isp_denoise2d_control, ISP_2DNR_ENABLE, 1);

#ifdef ISP_2DNR_V2
	isp_write_reg(dev, REG_ADDR(isp_denoise2d_sigma_sqr), dnr2->sigma_sqr);
	isp_write_reg(dev, REG_ADDR(isp_denoise2d_sigma_sqr_shd),
			  dnr2->sigma_sqr);
	isp_write_reg(dev, REG_ADDR(isp_denoise2d_weight_mul_factor),
			  dnr2->weight);
	isp_write_reg(dev, REG_ADDR(isp_denoise2d_weight_mul_factor_shd),
			  dnr2->weight);
	/* refer to HW spec for HBLANK */
	/* isp_write_reg(dev, REG_ADDR(isp_denoise2d_dummy_hblank), 0); */

	isp_write_reg(dev, REG_ADDR(isp_denoise2d_strength_shd), strength);
	isp_write_reg(dev, REG_ADDR(isp_denoise2d_control_shd),
			  isp_denoise2d_control);
#endif
	isp_write_reg(dev, REG_ADDR(isp_denoise2d_control),
			  isp_denoise2d_control);
	return 0;
#endif
}

int isp_s_simp(struct isp_ic_dev *dev)
{
	struct isp_simp_context *simp = &dev->simp;
	u32 vi_ircl = isp_read_reg(dev, REG_ADDR(vi_ircl));
	u32 vi_iccl = isp_read_reg(dev, REG_ADDR(vi_iccl));
	u32 super_imp_ctrl = isp_read_reg(dev, REG_ADDR(super_imp_ctrl));

	isp_info("enter %s\n", __func__);

	REG_SET_SLICE(vi_ircl, MRV_VI_SIMP_SOFT_RST, 1);
	isp_write_reg(dev, REG_ADDR(vi_ircl), vi_ircl);

	if (!simp->enable) {
		REG_SET_SLICE(vi_iccl, MRV_VI_SIMP_CLK_ENABLE, 0);
		isp_write_reg(dev, REG_ADDR(vi_iccl), vi_iccl);
		return 0;
	}

	REG_SET_SLICE(vi_ircl, MRV_VI_SIMP_SOFT_RST, 0);
	isp_write_reg(dev, REG_ADDR(super_imp_offset_x), simp->x);
	isp_write_reg(dev, REG_ADDR(super_imp_offset_y), simp->y);
	isp_write_reg(dev, REG_ADDR(super_imp_color_y), simp->r);
	isp_write_reg(dev, REG_ADDR(super_imp_color_cb), simp->g);
	isp_write_reg(dev, REG_ADDR(super_imp_color_cr), simp->b);
	REG_SET_SLICE(super_imp_ctrl, MRV_SI_TRANSPARENCY_MODE,
			  simp->transparency_mode);
	REG_SET_SLICE(super_imp_ctrl, MRV_SI_REF_IMAGE, simp->ref_image);
	isp_write_reg(dev, REG_ADDR(super_imp_ctrl), super_imp_ctrl);
	isp_write_reg(dev, REG_ADDR(vi_ircl), vi_ircl);
	REG_SET_SLICE(vi_iccl, MRV_VI_SIMP_CLK_ENABLE, 1);
	isp_write_reg(dev, REG_ADDR(vi_iccl), vi_iccl);

	return 0;
}

int isp_s_cproc(struct isp_ic_dev *dev)
{
	struct isp_cproc_context *cproc = &dev->cproc;
	u32 vi_iccl = isp_read_reg(dev, REG_ADDR(vi_iccl));
	u32 cproc_ctrl = isp_read_reg(dev, REG_ADDR(cproc_ctrl));

	dev->cproc.changed = false;


	if (!cproc->enable) {
		REG_SET_SLICE(cproc_ctrl, MRV_CPROC_CPROC_ENABLE, 0);
		isp_write_reg(dev, REG_ADDR(cproc_ctrl), cproc_ctrl);
		return 0;
	}

	isp_write_reg(dev, REG_ADDR(cproc_contrast), cproc->contrast);
	isp_write_reg(dev, REG_ADDR(cproc_brightness), cproc->brightness);
	isp_write_reg(dev, REG_ADDR(cproc_saturation), cproc->saturation);
	isp_write_reg(dev, REG_ADDR(cproc_hue), cproc->hue);
	REG_SET_SLICE(cproc_ctrl, MRV_CPROC_CPROC_ENABLE, 1);
	REG_SET_SLICE(cproc_ctrl, MRV_CPROC_CPROC_C_OUT_RANGE,
				cproc->c_out_full);
	REG_SET_SLICE(cproc_ctrl, MRV_CPROC_CPROC_Y_OUT_RANGE,
				cproc->y_out_full);
	REG_SET_SLICE(cproc_ctrl, MRV_CPROC_CPROC_Y_IN_RANGE, cproc->y_in_full);
	REG_SET_SLICE(vi_iccl, MRV_VI_CP_CLK_ENABLE, 1);
	isp_write_reg(dev, REG_ADDR(vi_iccl), vi_iccl);
	isp_write_reg(dev, REG_ADDR(cproc_ctrl), cproc_ctrl);

	return 0;
}

int isp_cproc_control(struct isp_ic_dev *dev)
{
	unsigned long flags;
	int ret = 0;

	spin_lock_irqsave(&dev->irqlock, flags);
	if (!is_isp_enable(dev)) {
		ret = isp_s_cproc(dev);
	} else {
		dev->cproc.changed = true;
	}

	spin_unlock_irqrestore(&dev->irqlock, flags);

	return ret;
}

int isp_s_elawb(struct isp_ic_dev *dev)
{
	struct isp_elawb_context *elawb = &dev->elawb;
	u32 awb_meas_mode = isp_read_reg(dev, REG_ADDR(awb_meas_mode));
	u32 isp_imsc = isp_read_reg(dev, REG_ADDR(isp_imsc));
	u32 id = elawb->id;
	u32 data;

	if (!elawb->enable) {
		REG_SET_SLICE(awb_meas_mode, ISP_AWB_MEAS_IRQ_ENABLE, 0);
		REG_SET_SLICE(awb_meas_mode, ISP_AWB_MEAS_EN, 0);
		isp_write_reg(dev, REG_ADDR(awb_meas_mode), awb_meas_mode);
		isp_write_reg(dev, REG_ADDR(isp_imsc),
				  isp_imsc & ~MRV_ISP_IMSC_AWB_DONE_MASK);
		return 0;
	}

	isp_write_reg(dev, REG_ADDR(awb_meas_h_offs), elawb->window.x);
	isp_write_reg(dev, REG_ADDR(awb_meas_v_offs), elawb->window.y);
	isp_write_reg(dev, REG_ADDR(awb_meas_h_size), elawb->window.width);
	isp_write_reg(dev, REG_ADDR(awb_meas_v_size), elawb->window.height);

	if (id > 0 && id < 9) {
		isp_write_reg(dev, REG_ADDR(awb_meas_center[id - 1].x),
				  elawb->info[id - 1].x);
		isp_write_reg(dev, REG_ADDR(awb_meas_center[id - 1].y),
				  elawb->info[id - 1].y);
		isp_write_reg(dev, REG_ADDR(awb_meas_axis[id - 1].a1),
				  elawb->info[id - 1].a1);
		isp_write_reg(dev, REG_ADDR(awb_meas_axis[id - 1].a2),
				  elawb->info[id - 1].a2);
		isp_write_reg(dev, REG_ADDR(awb_meas_axis[id - 1].a3),
				  elawb->info[id - 1].a3);
		isp_write_reg(dev, REG_ADDR(awb_meas_axis[id - 1].a4),
				  elawb->info[id - 1].a4);
		isp_write_reg(dev, REG_ADDR(awb_meas_rmax[id - 1]),
				  elawb->info[id - 1].r_max_sqr);
	}

	data = 0;
	REG_SET_SLICE(data, MRV_ISP_AWB_GAIN_R, elawb->r);
	REG_SET_SLICE(data, MRV_ISP_AWB_GAIN_B, elawb->b);
	isp_write_reg(dev, REG_ADDR(isp_awb_gain_rb), data);
	data = 0;
	REG_SET_SLICE(data, MRV_ISP_AWB_GAIN_GR, elawb->gr);
	REG_SET_SLICE(data, MRV_ISP_AWB_GAIN_GB, elawb->gb);
	isp_write_reg(dev, REG_ADDR(isp_awb_gain_g), data);

	REG_SET_SLICE(awb_meas_mode, ISP_AWB_MEAS_IRQ_ENABLE, 1);
	REG_SET_SLICE(awb_meas_mode, ISP_AWB_MEAS_EN, 1);
	isp_write_reg(dev, REG_ADDR(awb_meas_mode), awb_meas_mode);
	isp_write_reg(dev, REG_ADDR(isp_imsc),
			  isp_imsc | MRV_ISP_IMSC_AWB_DONE_MASK);

	return 0;
}

int isp_ioc_qcap(struct isp_ic_dev *dev, void *args)
{

	/* use public VIDIOC_QUERYCAP to query the type of v4l-subdevs. */
#ifndef USE_FPGA
	struct v4l2_capability *cap = (struct v4l2_capability *)args;
	strcpy((char *)cap->driver, "viv_isp_subdev");
	cap->bus_info[0] = (__u8)dev->id;
#else
	struct v4l2_capability cap;
	strcpy((char *)cap.driver, "viv_isp_subdev");
	cap->bus_info[0] = (__u8)dev->id;
	isp_info("enter %s viv_isp_subdev\n", __func__);
	viv_check_retval(copy_to_user
			 ((struct v4l2_capability *)args, &cap, sizeof(cap)));
#endif
	return 0;
}

int isp_ioc_g_status(struct isp_ic_dev *dev, void *args)
{
	u32 val = 0;
	viv_check_retval(copy_to_user(args, &val, sizeof(val)));
	return 0;
}

int isp_ioc_g_feature(struct isp_ic_dev *dev, void *args)
{
	u32 val = 0;

#ifdef ISP_EE
	val |= ISP_EE_SUPPORT;
#endif
#ifdef ISP_WDR3
	val |= ISP_WDR3_SUPPORT;
#endif
#ifdef ISP_2DNR
	val |= ISP_2DNR_SUPPORT;
#endif
#ifdef ISP_3DNR
	val |= ISP_3DNR_SUPPORT;
#endif
#ifdef ISP_WDR_V3
	val |= ISP_WDR3_SUPPORT;
#endif
#ifdef ISP_MIV2
	val |= ISP_MIV2_SUPPORT;
#endif
#ifdef ISP_AEV2
	val |= ISP_AEV2_SUPPORT;
#endif
#ifdef ISP_HDR_STITCH
	val |= ISP_HDR_STITCH_SUPPORT;
#endif
	viv_check_retval(copy_to_user(args, &val, sizeof(val)));

	return 0;
}

int isp_ioc_g_feature_veresion(struct isp_ic_dev *dev, void *args)
{
	u32 val = 0;

	viv_check_retval(copy_to_user(args, &val, sizeof(val)));

	return 0;
}

static long isp_get_extmem(struct isp_ic_dev *dev, void *args)
{
	long ret = 0;
	struct isp_extmem_info ext_mem;

	struct reserved_mem *rmem = (struct reserved_mem *)dev->rmem;
	if (rmem) {
		ext_mem.addr = rmem->base;
		ext_mem.size = rmem->size;
	} else {
		ext_mem.addr = 0;
		ext_mem.size = 0;
		pr_err("%s:isp cannot get reserve mem\n",__func__);
	}
	ret = copy_to_user(args, &ext_mem, sizeof(struct isp_extmem_info));

	return ret;
}

int isp_s_wdr(struct isp_ic_dev *dev)
{
	isp_wdr_context_t* wdr = &dev->wdr;

	/*update wdr configuration	after frame end when isp enable*/
	uint32_t isp_wdr_offset, isp_wdr_ctrl;

	isp_wdr_offset = isp_read_reg(dev, REG_ADDR(isp_wdr_offset));
	REG_SET_SLICE( isp_wdr_offset, MRV_WDR_LUM_OFFSET, wdr->LumOffset );
	REG_SET_SLICE( isp_wdr_offset, MRV_WDR_RGB_OFFSET, wdr->RgbOffset );
	isp_write_reg(dev, REG_ADDR(isp_wdr_offset), isp_wdr_offset);

	isp_wdr_ctrl = isp_read_reg(dev, REG_ADDR(isp_wdr_ctrl));
	REG_SET_SLICE(isp_wdr_ctrl, MRV_WDR_ENABLE, wdr->enabled);
	isp_write_reg(dev, REG_ADDR(isp_wdr_ctrl), isp_wdr_ctrl);
	wdr->changed = false;

	return 0;
}

int isp_s_wdr_ctrl(struct isp_ic_dev *dev)
{
	unsigned long flags;
	int ret = 0;

	spin_lock_irqsave(&dev->irqlock, flags);

	if (!is_isp_enable(dev)) {
		ret = isp_s_wdr(dev);
	} else {
		dev->wdr.changed = true;
	}

	spin_unlock_irqrestore(&dev->irqlock, flags);

	return ret;
}

static int isp_s_wdr_curve(struct isp_ic_dev *dev)
{
	isp_wdr_context_t* wdr = &dev->wdr;

	int i, j;
	uint32_t dYi = 0U;
	for ( i=0; i<4; i++ )
	{
		for ( j=8; j>0; j-- )
		{
			dYi <<= 4;
			dYi += wdr->dY[ (i*8 + j) ];
		}

		switch(i)
		{
		case 0:
			isp_write_reg(dev, REG_ADDR(isp_wdr_tonecurve_1), dYi);
			break;
		case 1:
			isp_write_reg(dev, REG_ADDR(isp_wdr_tonecurve_2), dYi);
			break;
		case 2:
			isp_write_reg(dev, REG_ADDR(isp_wdr_tonecurve_3), dYi);
			break;
		default:
			isp_write_reg(dev, REG_ADDR(isp_wdr_tonecurve_4), dYi);
			break;
		}
	}

	for ( i=0; i<33; i++ )
	{
		isp_write_reg(dev, REG_ADDR(wdr_tone_mapping_curve_y_block_arr[i]), wdr->Ym[i]);
	}

	dYi = 0x00000000;

	isp_write_reg(dev, REG_ADDR(isp_wdr_offset), dYi);
	isp_write_reg(dev, REG_ADDR(isp_wdr_deltamin), 0x00100000);

	return 0;

}


long isp_priv_ioctl(struct isp_ic_dev *dev, unsigned int cmd, void *args)
{
	int ret = -1;
	if (!dev) {
		return ret;
	}

	switch (cmd) {
	case ISPIOC_RESET:
		if((ret = isp_mi_stop(dev)) != 0 )
		{
			pr_err("[%s:%d]stop mi error before resetting!\n", __func__, __LINE__);
			break;
		}
		if((ret = isp_stop_stream(dev)) != 0)
		{
			pr_err("[%s:%d]stop isp stream before resetting!\n", __func__, __LINE__);
			break;
		}
		ret = isp_reset(dev);
		break;

	case ISPIOC_WRITE_REG:
		ret = isp_ioc_write_reg(dev, args);
		break;
	case ISPIOC_READ_REG:
		ret = isp_ioc_read_reg(dev, args);
		break;
	case ISPIOC_ENABLE_TPG:
		ret = isp_enable_tpg(dev);
		break;
	case ISPIOC_DISABLE_TPG:
		ret = isp_disable_tpg(dev);
		break;
	case ISPIOC_ENABLE_BLS:
		ret = isp_enable_bls(dev);
		break;
	case ISPIOC_DISABLE_BLS:
		ret = isp_disable_bls(dev);
		break;
	case ISPIOC_START_DMA_READ:
		ret = isp_ioc_start_dma_read(dev, args);
		break;
	case ISPIOC_MI_STOP:
		ret = isp_mi_stop(dev);
		break;
	case ISPIOC_DISABLE_ISP_OFF:
		ret = isp_ioc_disable_isp_off(dev, args);
		break;
	case ISPIOC_ISP_STOP:
		ret = isp_stop_stream(dev);
		if(!ret) {
			dev->streaming = false;
		}
		break;
	case ISPIOC_ENABLE:
		ret = isp_enable(dev);
		break;
	case ISPIOC_DISABLE:
		ret = isp_disable(dev);
		break;
	case ISPIOC_ISP_STATUS:{
		bool enable = is_isp_enable(dev);
		viv_check_retval(copy_to_user
				 (args, &enable, sizeof(bool)));
		ret = 0;
		break;
		}
	case ISPIOC_ENABLE_LSC:
		ret = isp_enable_lsc(dev);
		break;
	case ISPIOC_DISABLE_LSC:
		ret = isp_disable_lsc(dev);
		break;
	case ISPIOC_S_DIGITAL_GAIN:
		viv_check_retval(copy_from_user
				 (&dev->dgain, args, sizeof(dev->dgain)));
		ret = isp_s_digital_gain(dev);
		break;
#ifdef ISP_DEMOSAIC2
	case ISPIOC_S_DMSC_INTP:
		viv_check_retval(copy_from_user
				 (&dev->demosaic.intp_cxt, args,
				  sizeof(dev->demosaic.intp_cxt)));
		ret = isp_set_dmsc_intp(dev);
		break;
	case ISPIOC_S_DMSC_DMOI:
		viv_check_retval(copy_from_user
				 (&dev->demosaic.dmoi_cxt, args,
				  sizeof(dev->demosaic.dmoi_cxt)));
		ret = isp_set_dmsc_dmoi(dev);
		break;
	case ISPIOC_S_DMSC_SKIN:
		viv_check_retval(copy_from_user
				 (&dev->demosaic.skin_cxt, args,
				  sizeof(dev->demosaic.skin_cxt)));
		ret = isp_set_dmsc_skin(dev);
		break;
	case ISPIOC_S_DMSC_SHAP:
		viv_check_retval(copy_from_user
				 (&dev->demosaic.shap_cxt, args,
				  sizeof(dev->demosaic.shap_cxt)));
		ret = isp_set_dmsc_sharpen(dev);
		break;
	case ISPIOC_S_DMSC_CAC:
		viv_check_retval(copy_from_user
				 (&dev->cac, args, sizeof(dev->cac)));

		ret = isp_set_dmsc_cac(dev);
		break;
	case ISPIOC_S_DMSC:
		viv_check_retval(copy_from_user
				 (&dev->demosaic, args, sizeof(dev->demosaic)));
		ret = isp_s_dmsc(dev);
		break;
#endif
	case ISPIOC_ENABLE_AWB:
		ret = isp_enable_awb(dev);
		break;
	case ISPIOC_DISABLE_AWB:
		ret = isp_disable_awb(dev);
		break;
	case ISPIOC_ENABLE_WB:
		ret = isp_enable_wb(dev, 1);
		break;
	case ISPIOC_DISABLE_WB:
		ret = isp_enable_wb(dev, 0);
		break;
	case ISPIOC_ENABLE_GAMMA_OUT:
		ret = isp_enable_gamma_out_ctrl(dev, 1);
		break;
	case ISPIOC_DISABLE_GAMMA_OUT:
		ret = isp_enable_gamma_out_ctrl(dev, 0);
		break;
	case ISPIOC_R_3DNR:
		viv_check_retval(copy_from_user
				 (&dev->dnr3, args, sizeof(dev->dnr3)));
		ret = isp_r_3dnr(dev);
		break;
	case ISPIOC_S_IS:
		viv_check_retval(copy_from_user
				 (&dev->is, args, sizeof(dev->is)));
		ret = isp_s_is(dev);
		break;
	case ISPIOC_S_RAW_IS:
		viv_check_retval(copy_from_user
				 (&dev->rawis, args, sizeof(dev->rawis)));
		ret = isp_s_raw_is(dev);
		break;
	case ISPIOC_S_CC:
		viv_check_retval(copy_from_user
				 (&dev->cc, args, sizeof(dev->cc)));
		ret = isp_cc_control(dev);
		break;
	case ISPIOC_S_EE:
		viv_check_retval(copy_from_user
				 (&dev->ee, args, sizeof(dev->ee)));
		ret = isp_s_ee(dev);
		break;
	case ISPIOC_S_IE:
		viv_check_retval(copy_from_user
				 (&dev->ie, args, sizeof(dev->ie)));
		ret = isp_s_ie(dev);
		break;
	case ISPIOC_S_TPG:
		viv_check_retval(copy_from_user
				 (&dev->tpg, args, sizeof(dev->tpg)));
		ret = isp_s_tpg(dev);
		break;
	case ISPIOC_S_BLS:
		viv_check_retval(copy_from_user
				 (&dev->bls, args, sizeof(dev->bls)));
		ret = isp_s_bls(dev);
		break;
	case ISPIOC_S_MCM:
		viv_check_retval(copy_from_user
		(&dev->mcm, args, sizeof(dev->mcm)));
		ret = isp_s_mcm(dev);
		break;
	case ISPIOC_S_MUX:
		viv_check_retval(copy_from_user
				 (&dev->mux, args, sizeof(dev->mux)));
		ret = isp_s_mux(dev);
		break;
	case ISPIOC_S_AWB:
		viv_check_retval(copy_from_user
				 (&dev->awb, args, sizeof(dev->awb)));
		ret = isp_awb_control(dev);
		break;
	case ISPIOC_S_LSC_TBL:
		viv_check_retval(copy_from_user
				 (&dev->lsc, args, sizeof(dev->lsc)));
		ret = isp_s_lsc_tbl(dev);
		break;
	case ISPIOC_S_LSC_SEC:
		viv_check_retval(copy_from_user
				 (&dev->lsc, args, sizeof(dev->lsc)));
		ret = isp_s_lsc_sec(dev);
		break;
	case ISPIOC_S_DPF:
		viv_check_retval(copy_from_user
				 (&dev->dpf, args, sizeof(dev->dpf)));
		ret = isp_s_dpf(dev);
		break;
	case ISPIOC_S_EXP:
		viv_check_retval(copy_from_user
				 (&dev->exp, args, sizeof(dev->exp)));
		ret = isp_s_exp(dev);
		break;
	case ISPIOC_S_CNR:
		viv_check_retval(copy_from_user
				 (&dev->cnr, args, sizeof(dev->cnr)));
		ret = isp_s_cnr(dev);
		break;
	case ISPIOC_S_FLT:
	{
		viv_check_retval(copy_from_user
				 (&dev->flt, args, sizeof(dev->flt)));
		ret = isp_s_flt_ctrl(dev);
		break;
	}
	case ISPIOC_S_CAC:
		viv_check_retval(copy_from_user
				 (&dev->cac, args, sizeof(dev->cac)));
		ret = isp_s_cac(dev);
		break;
	case ISPIOC_S_DEG:
		viv_check_retval(copy_from_user
				 (&dev->deg, args, sizeof(dev->deg)));
		ret = isp_s_deg(dev);
		break;
	case ISPIOC_S_VSM:
		viv_check_retval(copy_from_user
				 (&dev->vsm, args, sizeof(dev->vsm)));
		ret = isp_s_vsm(dev);
		break;
	case ISPIOC_S_AFM:
		viv_check_retval(copy_from_user
				 (&dev->afm, args, sizeof(dev->afm)));
		ret = isp_s_afm(dev);
		break;
	case ISPIOC_S_HDR:
		viv_check_retval(copy_from_user
				 (&dev->hdr, args, sizeof(dev->hdr)));
		ret = isp_s_hdr(dev);
		break;
	case ISPIOC_ENABLE_HDR:
		viv_check_retval(copy_from_user
				 (&dev->hdr, args, sizeof(dev->hdr)));
		ret = isp_enable_hdr(dev);
		break;
	case ISPIOC_DISABLE_HDR:
		viv_check_retval(copy_from_user
				 (&dev->hdr, args, sizeof(dev->hdr)));
		ret = isp_disable_hdr(dev);
		break;
	case ISPIOC_S_HIST:
		viv_check_retval(copy_from_user
				 (&dev->hist, args, sizeof(dev->hist)));
		ret = isp_s_hist(dev);
		break;
	case ISPIOC_S_DPCC:
		viv_check_retval(copy_from_user
				 (&dev->dpcc, args, sizeof(dev->dpcc)));
		ret = isp_s_dpcc(dev);
		break;
	case ISPIOC_ENABLE_WDR3:
		ret = isp_enable_wdr3(dev);
		break;
	case ISPIOC_DISABLE_WDR3:
		ret = isp_disable_wdr3(dev);
		break;
	case ISPIOC_U_WDR3:
		viv_check_retval(copy_from_user
				 (&dev->wdr3, args, sizeof(dev->wdr3)));
		ret = isp_u_wdr3(dev);
		break;
	case ISPIOC_S_WDR3:
		viv_check_retval(copy_from_user
				 (&dev->wdr3, args, sizeof(dev->wdr3)));
		ret = isp_s_wdr3(dev);
		break;
	case ISPIOC_S_EXP2:
		viv_check_retval(copy_from_user
				 (&dev->exp2, args, sizeof(dev->exp2)));
		ret = isp_s_exp2(dev);
		break;
	case ISPIOC_S_2DNR:
		viv_check_retval(copy_from_user
				 (&dev->dnr2, args, sizeof(dev->dnr2)));
		ret = isp_s_2dnr(dev);
		break;
	case ISPIOC_S_3DNR:
		viv_check_retval(copy_from_user
				 (&dev->dnr3, args, sizeof(dev->dnr3)));
		ret = isp_s_3dnr(dev);
		break;
	case ISPIOC_S_SIMP:
		viv_check_retval(copy_from_user
				 (&dev->simp, args, sizeof(dev->simp)));
		ret = isp_s_simp(dev);
		break;
	case ISPIOC_S_COMP:
		viv_check_retval(copy_from_user
				 (&dev->comp, args, sizeof(dev->comp)));
		ret = isp_s_comp(dev);
		break;
	case ISPIOC_S_CPROC:
		viv_check_retval(copy_from_user
				 (&dev->cproc, args, sizeof(dev->cproc)));
		ret = isp_cproc_control(dev);
		break;
	case ISPIOC_S_XTALK:
		viv_check_retval(copy_from_user
				 (&dev->xtalk, args, sizeof(dev->xtalk)));
		ret = isp_s_xtalk(dev);
		break;
	case ISPIOC_S_ELAWB:
		viv_check_retval(copy_from_user
				 (&dev->elawb, args, sizeof(dev->elawb)));
		ret = isp_s_elawb(dev);
		break;
	case ISPIOC_S_INPUT:
		viv_check_retval(copy_from_user
				 (&dev->ctx, args, sizeof(dev->ctx)));
		ret = isp_s_input(dev);
		break;
	case ISPIOC_S_DEMOSAIC:
		viv_check_retval(copy_from_user
				 (&dev->ctx, args, sizeof(dev->ctx)));
		ret = isp_s_demosaic(dev);
		break;
	case ISPIOC_MI_START:
		viv_check_retval(copy_from_user
				 (&dev->mi, args, sizeof(dev->mi)));
		ret = isp_mi_start(dev);
		break;
	case ISPIOC_S_HDR_WB:
		viv_check_retval(copy_from_user
				 (&dev->hdr, args, sizeof(dev->hdr)));
		ret = isp_s_hdr_wb(dev);
		break;
	case ISPIOC_S_HDR_BLS:
		viv_check_retval(copy_from_user
				 (&dev->hdr, args, sizeof(dev->hdr)));
		ret = isp_s_hdr_bls(dev);
		break;
	case ISPIOC_S_HDR_DIGITAL_GAIN:
		viv_check_retval(copy_from_user
				 (&dev->hdr, args, sizeof(dev->hdr)));
		break;
	case ISPIOC_S_GAMMA_OUT:{
			viv_check_retval(copy_from_user
					 (&dev->gamma_out, args,
					  sizeof(dev->gamma_out)));
			ret = isp_s_gamma_out_ctrl(dev);
			break;
		}
	case ISPIOC_SET_BUFFER:{
			struct isp_buffer_context buf;
			viv_check_retval(copy_from_user
					 (&buf, args, sizeof(buf)));
			if (dev->alloc)
				ret = dev->alloc(dev, &buf);
			break;
		}
	case ISPIOC_SET_BP_BUFFER:{
			struct isp_bp_buffer_context buf;
			viv_check_retval(copy_from_user
					 (&buf, args, sizeof(buf)));
			ret = isp_set_bp_buffer(dev, &buf);
			break;
		}
	case ISPIOC_START_CAPTURE:{
			u32 num;
			int i;
			viv_check_retval(copy_from_user
					 (&num, args, sizeof(num)));
			ret = isp_start_stream(dev, num);
			if(!ret) {
				dev->streaming = true;
				for (i = 0; i < MI_PATH_NUM; i++) {
					dev->frame_cnt[i] = 0;
					dev->last_ns[i] = 0;
				}
			}
			break;
		}
	case ISPIOC_U_3DNR:{
			struct isp_3dnr_update param;
			viv_check_retval(copy_from_user
					 (&param, args, sizeof(param)));
			ret = isp_u_3dnr(dev, &param);
			break;
		}

		case ISPIOC_S_3DNR_CMP: {
			viv_check_retval(
				copy_from_user(&dev->dnr3.compress, args, sizeof(dev->dnr3.compress)));
					ret = isp_s_3dnr_cmp(dev);
			break;
		}

	case ISPIOC_G_AWBMEAN:{
			struct isp_awb_mean mean;
			ret = isp_g_awbmean(dev, &mean);
			viv_check_retval(copy_to_user
					 (args, &mean, sizeof(mean)));
			break;
		}
	case ISPIOC_G_EXPMEAN:{
			u8 mean[25];
			ret = isp_g_expmean(dev, mean);
			viv_check_retval(copy_to_user
					 (args, mean, sizeof(mean)));
			break;
		}
	case ISPIOC_G_HISTMEAN:{
			u32 mean[HIST_BIN_TOTAL];
			ret = isp_g_histmean(dev, mean);
			viv_check_retval(copy_to_user
					 (args, mean, sizeof(mean)));
			break;
		}
	case ISPIOC_G_VSM:{
			struct isp_vsm_result vsm;
			ret = isp_g_vsm(dev, &vsm);
			viv_check_retval(copy_to_user(args, &vsm, sizeof(vsm)));
			break;
		}
	case ISPIOC_G_AFM:{
			struct isp_afm_result afm;
			ret = isp_g_afm(dev, &afm);
			viv_check_retval(copy_to_user(args, &afm, sizeof(afm)));
			break;
		}
	case ISPIOC_G_3DNR:{
			u32 avg;
			ret = isp_g_3dnr(dev, &avg);
			viv_check_retval(copy_to_user(args, &avg, sizeof(avg)));
			break;
		}
	case ISPIOC_G_STATUS:
		ret = isp_ioc_g_status(dev, args);
		break;
	case ISPIOC_G_FEATURE:
		ret = isp_ioc_g_feature(dev, args);
		break;
	case ISPIOC_G_FEATURE_VERSION:
		ret = isp_ioc_g_feature_veresion(dev, args);
		break;
	case ISPIOC_WDR_CONFIG:
		viv_check_retval(copy_from_user
				 (&dev->wdr, args, sizeof(dev->wdr)));
		ret = isp_s_wdr_ctrl(dev);
		break;
	case ISPIOC_S_WDR_CURVE:
		viv_check_retval(copy_from_user
				 (&dev->wdr, args, sizeof(dev->wdr)));
		ret = isp_s_wdr_curve(dev);
		break;
	case ISPIOC_ENABLE_GCMONO:
		viv_check_retval(copy_from_user
				 (&dev->gcmono.mode, args, sizeof(u32)));
		ret = isp_enable_gcmono(dev);
		break;
	case ISPIOC_DISABLE_GCMONO:
		ret = isp_disable_gcmono(dev);
		break;
	case ISPIOC_S_GCMONO:{
			struct isp_gcmono_data *data;
			data = (struct isp_gcmono_data *)
				kmalloc(sizeof(struct isp_gcmono_data), GFP_KERNEL);

			if (data == NULL) {
				isp_err("malloc mem for rgb gamma failed.");
				ret = -1;
			} else {
				viv_check_retval(copy_from_user
						 (data, args,
						  sizeof(struct
							 isp_gcmono_data)));
				ret = isp_s_gcmono(dev, data);
				kfree(data);
			}
			break;
		}
	case ISPIOC_ENABLE_RGBGAMMA:
		ret = isp_enable_rgbgamma(dev);
		break;
	case ISPIOC_DISABLE_RGBGAMMA:
		ret = isp_disable_rgbgamma(dev);
		break;
	case ISPIOC_S_RGBGAMMA:{
			struct isp_rgbgamma_data *data;
			data = (struct isp_rgbgamma_data *)
				kmalloc(sizeof(struct isp_rgbgamma_data),
					GFP_KERNEL);
			if (data == NULL) {
				isp_err("malloc mem for rgb gamma failed.");
				ret = -1;
			} else {
				viv_check_retval(copy_from_user
						 (data, args,
						  sizeof(struct
							 isp_rgbgamma_data)));
				ret = isp_s_rgbgamma(dev, data);
				kfree(data);
			}
			break;
		}
	case ISPIOC_S_GREENEQUILIBRATE:
		viv_check_retval(copy_from_user
				 (&dev->ge, args, sizeof(dev->ge)));
		ret = isp_s_ge(dev);
		break;
	case ISPIOC_S_COLOR_ADJUST:
		viv_check_retval(copy_from_user
				 (&dev->ca, args, sizeof(dev->ca)));
		ret = isp_s_ca(dev);
		break;
	case VIDIOC_QUERYCAP:
		ret = isp_ioc_qcap(dev, args);
		break;
	case ISPIOC_G_QUERY_EXTMEM:
		ret = isp_get_extmem(dev, args);
		break;
	default:
		isp_err("unsupported command %d", cmd);
		ret = -EINVAL;
		break;
	}

	return ret;
}
