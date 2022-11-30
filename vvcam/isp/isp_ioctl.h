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
#ifndef _ISP_IOC_H_
#define _ISP_IOC_H_
#include "ic_dev.h"
#include <linux/ioctl.h>
enum {
	ISPIOC_RESET				= 0x100,
	ISPIOC_WRITE_REG			= 0x101,
	ISPIOC_READ_REG 			= 0x102,
	ISPIOC_S_INPUT				= 0x103,
	ISPIOC_ENABLE				= 0x104,
	ISPIOC_DISABLE				= 0x105,
	ISPIOC_ISP_STATUS			= 0x106,
	ISPIOC_ISP_STOP 			= 0x107,
	ISPIOC_START_CAPTURE		= 0x108,
	ISPIOC_DISABLE_ISP_OFF		= 0x109,
	ISPIOC_SET_BUFFER			= 0x10A,
	ISPIOC_SET_BP_BUFFER		= 0x10B,
	ISPIOC_START_DMA_READ		= 0x10C,
	ISPIOC_MI_START 			= 0x10D,
	ISPIOC_MI_STOP				= 0x10E,
	ISPIOC_ENABLE_TPG			= 0x10F,
	ISPIOC_DISABLE_TPG			= 0x110,
	ISPIOC_S_TPG				= 0x111,
	ISPIOC_S_MCM				= 0x112,
	ISPIOC_ENABLE_BLS			= 0x113,
	ISPIOC_DISABLE_BLS			= 0x114,
	ISPIOC_S_BLS				= 0x115,
	ISPIOC_S_MUX				= 0x116,
	ISPIOC_ENABLE_AWB			= 0x117,
	ISPIOC_DISABLE_AWB			= 0x118,
	ISPIOC_S_AWB				= 0x119,
	ISPIOC_G_AWBMEAN			= 0x11A,
	ISPIOC_S_IS 				= 0x11B,
	ISPIOC_S_RAW_IS 			= 0x11C,
	ISPIOC_S_CNR				= 0x11D,
	ISPIOC_S_CC 				= 0x11E,
	ISPIOC_S_XTALK				= 0x11F,
	ISPIOC_S_GAMMA_OUT			= 0x120,
	ISPIOC_ENABLE_LSC			= 0x121,
	ISPIOC_DISABLE_LSC			= 0x122,
	ISPIOC_S_LSC_TBL			= 0x123,
	ISPIOC_S_LSC_SEC			= 0x124,
	ISPIOC_S_DPF				= 0x125,
	ISPIOC_S_EE 				= 0x126,
	ISPIOC_S_EXP				= 0x127,
	ISPIOC_G_EXPMEAN			= 0x128,
	ISPIOC_S_HIST				= 0x129,
	ISPIOC_G_HISTMEAN			= 0x12A,
	ISPIOC_S_DPCC				= 0x12B,
	ISPIOC_S_FLT				= 0x12C,
	ISPIOC_S_CAC				= 0x12D,
	ISPIOC_S_DEG				= 0x12E,
	ISPIOC_S_AFM				= 0x12F,
	ISPIOC_G_AFM				= 0x130,
	ISPIOC_S_VSM				= 0x131,
	ISPIOC_G_VSM				= 0x132,
	ISPIOC_S_IE 				= 0x133,
	ISPIOC_ENABLE_WDR3			= 0x134,
	ISPIOC_DISABLE_WDR3 		= 0x135,
	ISPIOC_U_WDR3				= 0x136,
	ISPIOC_S_WDR3				= 0x137,
	ISPIOC_S_EXP2				= 0x138,
	ISPIOC_S_2DNR				= 0x139,
	ISPIOC_S_3DNR				= 0x13A,
	ISPIOC_G_3DNR				= 0x13B, /* get last avg */
	ISPIOC_U_3DNR				= 0x13C, /* update */
	ISPIOC_R_3DNR				= 0x13D, /* read back 3dnr reference image. */
	ISPIOC_S_3DNR_CMP			= 0x13E, /*config 3dnr compress */
	ISPIOC_S_HDR				= 0x13F,
	ISPIOC_S_COMP				= 0x140,
	ISPIOC_S_CPROC				= 0x141,
	ISPIOC_S_SIMP				= 0x142,
	ISPIOC_S_ELAWB				= 0x143,
	ISPIOC_S_HDR_WB 			= 0x144,
	ISPIOC_S_HDR_BLS			= 0x145,
	ISPIOC_S_HDR_DIGITAL_GAIN	= 0x146,
	ISPIOC_ENABLE_WB			= 0x147,
	ISPIOC_DISABLE_WB			= 0x148,
	ISPIOC_DISABLE_HDR			= 0x149,
	ISPIOC_ENABLE_HDR			= 0x14A,
	ISPIOC_ENABLE_GAMMA_OUT 	= 0x14B,
	ISPIOC_DISABLE_GAMMA_OUT	= 0x14C,
	ISPIOC_G_STATUS 			= 0x14D,
	ISPIOC_G_FEATURE			= 0x14E,
	ISPIOC_G_FEATURE_VERSION	= 0x14F,
	ISPIOC_ENABLE_GCMONO		= 0x150,
	ISPIOC_DISABLE_GCMONO		= 0x151,
	ISPIOC_S_GCMONO 			= 0x152,
	ISPIOC_ENABLE_RGBGAMMA		= 0x153,
	ISPIOC_DISABLE_RGBGAMMA 	= 0x154,
	ISPIOC_S_RGBGAMMA			= 0x155,
	ISPIOC_S_DEMOSAIC			= 0x156,
	ISPIOC_S_DMSC_INTP			= 0x157,
	ISPIOC_S_DMSC_DMOI			= 0x158,
	ISPIOC_S_DMSC_SKIN			= 0x159,
	ISPIOC_S_DMSC_CAC			= 0x15A,
	ISPIOC_S_DMSC_SHAP			= 0x15B,
	ISPIOC_S_DMSC				= 0x15C,
	ISPIOC_S_GREENEQUILIBRATE	= 0x15D,
	ISPIOC_S_COLOR_ADJUST		= 0x15E,
	ISPIOC_S_DIGITAL_GAIN		= 0x15F,
	ISPIOC_G_QUERY_EXTMEM		= 0x160,

	ISPIOC_WDR_CONFIG			= 0x16C,
	ISPIOC_S_WDR_CURVE			= 0x16D,
};

long isp_priv_ioctl(struct isp_ic_dev *dev, unsigned int cmd, void *args);
long isp_copy_data(void *dst, void *src, int size);

/* internal functions, can called by v4l2 video device and ioctl */
int isp_reset(struct isp_ic_dev *dev);
int isp_enable_tpg(struct isp_ic_dev *dev);
int isp_disable_tpg(struct isp_ic_dev *dev);
int isp_enable_bls(struct isp_ic_dev *dev);
int isp_disable_bls(struct isp_ic_dev *dev);
int isp_enable(struct isp_ic_dev *dev);
int isp_disable(struct isp_ic_dev *dev);
bool is_isp_enable(struct isp_ic_dev *dev);
int isp_enable_lsc(struct isp_ic_dev *dev);
int isp_disable_lsc(struct isp_ic_dev *dev);
int isp_s_input(struct isp_ic_dev *dev);
int isp_s_demosaic(struct isp_ic_dev *dev);
int isp_s_tpg(struct isp_ic_dev *dev);
int isp_s_mcm(struct isp_ic_dev *dev);
int isp_s_mux(struct isp_ic_dev *dev);
int isp_s_bls(struct isp_ic_dev *dev);
int isp_enable_awb(struct isp_ic_dev *dev);
int isp_disable_awb(struct isp_ic_dev *dev);
int isp_s_awb(struct isp_ic_dev *dev);
int isp_g_awbmean(struct isp_ic_dev *dev, struct isp_awb_mean *mean);
int isp_s_is(struct isp_ic_dev *dev);
int isp_s_raw_is(struct isp_ic_dev *dev);
int isp_s_cnr(struct isp_ic_dev *dev);
int isp_start_stream(struct isp_ic_dev *dev, u32 framenum);
int isp_stop_stream(struct isp_ic_dev *dev);
int isp_s_cc(struct isp_ic_dev *dev);
int isp_s_xtalk(struct isp_ic_dev *dev);
int isp_enable_wb(struct isp_ic_dev *dev, bool bEnable);
int isp_enable_gamma_out(struct isp_ic_dev *dev, bool bEnable);
int isp_s_gamma_out(struct isp_ic_dev *dev);
int isp_s_lsc_sec(struct isp_ic_dev *dev);
int isp_s_lsc_tbl(struct isp_ic_dev *dev);
int isp_s_dpf(struct isp_ic_dev *dev);
int isp_s_ee(struct isp_ic_dev *dev);
int isp_s_exp(struct isp_ic_dev *dev);
int isp_g_expmean(struct isp_ic_dev *dev, u8 *mean);
int isp_s_hist(struct isp_ic_dev *dev);
int isp_g_histmean(struct isp_ic_dev *dev, u32 *mean);
int isp_s_dpcc(struct isp_ic_dev *dev);
int isp_s_flt(struct isp_ic_dev *dev);
int isp_s_cac(struct isp_ic_dev *dev);
int isp_s_deg(struct isp_ic_dev *dev);
int isp_s_ie(struct isp_ic_dev *dev);
int isp_s_vsm(struct isp_ic_dev *dev);
int isp_g_vsm(struct isp_ic_dev *dev, struct isp_vsm_result *vsm);
int isp_s_afm(struct isp_ic_dev *dev);
int isp_g_afm(struct isp_ic_dev *dev, struct isp_afm_result *afm);
int isp_enable_wdr3(struct isp_ic_dev *dev);
int isp_disable_wdr3(struct isp_ic_dev *dev);
int isp_u_wdr3(struct isp_ic_dev *dev);
int isp_s_wdr3(struct isp_ic_dev *dev);
int isp_s_exp2(struct isp_ic_dev *dev);
int isp_s_hdr(struct isp_ic_dev *dev);
int isp_s_hdr_wb(struct isp_ic_dev *dev);
int isp_s_hdr_bls(struct isp_ic_dev *dev);
int isp_enable_hdr(struct isp_ic_dev *dev);
int isp_disable_hdr(struct isp_ic_dev *dev);
int isp_s_2dnr(struct isp_ic_dev *dev);
int isp_s_3dnr(struct isp_ic_dev *dev);
int isp_g_3dnr(struct isp_ic_dev *dev, u32 * avg);
int isp_u_3dnr(struct isp_ic_dev *dev, struct isp_3dnr_update *dnr3_update);
int isp_r_3dnr(struct isp_ic_dev *dev);
int isp_s_3dnr_cmp(struct isp_ic_dev *dev);
int isp_s_comp(struct isp_ic_dev *dev);
int isp_s_simp(struct isp_ic_dev *dev);
int isp_s_cproc(struct isp_ic_dev *dev);
int isp_s_elawb(struct isp_ic_dev *dev);

int isp_enable_gcmono(struct isp_ic_dev *dev);
int isp_disable_gcmono(struct isp_ic_dev *dev);
int isp_s_gcmono(struct isp_ic_dev *dev, struct isp_gcmono_data *data);	/* set curve */
int isp_enable_rgbgamma(struct isp_ic_dev *dev);
int isp_disable_rgbgamma(struct isp_ic_dev *dev);
int isp_s_rgbgamma(struct isp_ic_dev *dev, struct isp_rgbgamma_data *data);

u32 isp_read_mi_irq(struct isp_ic_dev *dev);
void isp_reset_mi_irq(struct isp_ic_dev *dev, u32 icr);

int isp_ioc_start_dma_read(struct isp_ic_dev *dev, void *args);
int isp_mi_start(struct isp_ic_dev *dev);
int isp_mi_stop(struct isp_ic_dev *dev);
int isp_set_buffer(struct isp_ic_dev *dev, struct isp_buffer_context *buf);
int isp_set_bp_buffer(struct isp_ic_dev *dev,
		      struct isp_bp_buffer_context *buf);

int isp_enable_dmsc(struct isp_ic_dev *dev);
int isp_disable_dmsc(struct isp_ic_dev *dev);
int isp_set_dmsc_intp(struct isp_ic_dev *dev);
int isp_set_dmsc_skin(struct isp_ic_dev *dev);
int isp_set_dmsc_cac(struct isp_ic_dev *dev);
int isp_set_dmsc_sharpen(struct isp_ic_dev *dev);
int isp_set_dmsc_dmoi(struct isp_ic_dev *dev);
int isp_s_dmsc(struct isp_ic_dev *dev);
int isp_s_ge(struct isp_ic_dev *dev);
int isp_s_color_adjust(struct isp_ic_dev *dev);
int isp_config_dummy_hblank(struct isp_ic_dev *dev);
int isp_s_wdr(struct isp_ic_dev *dev);

#ifdef __KERNEL__
int clean_dma_buffer(struct isp_ic_dev *dev);
irqreturn_t isp_hw_isr(int irq, void *data);
void isp_clear_interrupts(struct isp_ic_dev *dev);
int update_dma_buffer(struct isp_ic_dev *dev);
void isp_isr_tasklet(unsigned long arg);
#endif
#endif /* _ISP_IOC_H_ */
