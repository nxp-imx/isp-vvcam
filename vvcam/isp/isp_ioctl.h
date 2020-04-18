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

enum {
	ISPIOC_RESET = 0,
	ISPIOC_WRITE_REG = 1,
	ISPIOC_READ_REG = 4,	/* can't pass id 2, skip it. */
	ISPIOC_S_INPUT = 8,
	ISPIOC_ENABLE = 16,
	ISPIOC_DISABLE,
	ISPIOC_ISP_STOP,
	ISPIOC_START_CAPTURE,
	ISPIOC_DISABLE_ISP_OFF,
	ISPIOC_SET_BUFFER,
	ISPIOC_SET_BP_BUFFER,
	ISPIOC_START_DMA_READ,
	ISPIOC_MI_START,
	ISPIOC_MI_STOP,
	ISPIOC_ENABLE_TPG,
	ISPIOC_DISABLE_TPG,
	ISPIOC_S_TPG,
	ISPIOC_ENABLE_BLS,
	ISPIOC_DISABLE_BLS,
	ISPIOC_S_BLS,
	ISPIOC_S_MUX,
	ISPIOC_ENABLE_AWB,
	ISPIOC_DISABLE_AWB,
	ISPIOC_S_AWB,
	ISPIOC_G_AWBMEAN,
	ISPIOC_S_IS,
	ISPIOC_S_RAW_IS,
	ISPIOC_S_CNR,
	ISPIOC_S_CC,
	ISPIOC_S_XTALK,
	ISPIOC_S_GAMMA_OUT,
	ISPIOC_ENABLE_LSC,
	ISPIOC_DISABLE_LSC,
	ISPIOC_S_LSC,
	ISPIOC_S_DPF,
	ISPIOC_S_EE,
	ISPIOC_S_EXP,
	ISPIOC_G_EXPMEAN,
	ISPIOC_S_HIST,
	ISPIOC_G_HISTMEAN,
	ISPIOC_S_DPCC,
	ISPIOC_S_FLT,
	ISPIOC_S_CAC,
	ISPIOC_S_DEG,
	ISPIOC_S_AFM,
	ISPIOC_G_AFM,
	ISPIOC_S_VSM,
	ISPIOC_G_VSM,
	ISPIOC_S_IE,
	ISPIOC_S_WDR3,
	ISPIOC_S_EXP2,
	ISPIOC_S_2DNR,
	ISPIOC_S_3DNR,
	ISPIOC_G_3DNR,		/* get last avg */
	ISPIOC_U_3DNR,		/* update */
	ISPIOC_R_3DNR,		/* read back 3dnr reference image. */
	ISPIOC_S_HDR,
	ISPIOC_S_COMP,
	ISPIOC_S_CPROC,
	ISPIOC_S_SIMP,
	ISPIOC_S_ELAWB,
	ISPIOC_S_HDR_WB,
	ISPIOC_S_HDR_BLS,
	ISPIOC_ENABLE_WB,
	ISPIOC_DISABLE_WB,
	ISPIOC_DISABLE_HDR,
	ISPIOC_ENABLE_HDR,
	ISPIOC_ENABLE_GAMMA_OUT,
	ISPIOC_DISABLE_GAMMA_OUT,
	ISPIOC_G_STATUS,
	ISPIOC_G_FEATURE,
	ISPIOC_G_FEATURE_VERSION,
	ISPIOC_ENABLE_GCMONO,
	ISPIOC_DISABLE_GCMONO,
	ISPIOC_S_GCMONO,
	ISPIOC_ENABLE_RGBGAMMA,
	ISPIOC_DISABLE_RGBGAMMA,
	ISPIOC_S_RGBGAMMA,
	ISPIOC_S_DEMOSAIC,
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
int isp_enable_lsc(struct isp_ic_dev *dev);
int isp_disable_lsc(struct isp_ic_dev *dev);
int isp_s_input(struct isp_ic_dev *dev);
int isp_s_demosaic(struct isp_ic_dev *dev);
int isp_s_tpg(struct isp_ic_dev *dev);
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
int isp_s_lsc(struct isp_ic_dev *dev);
int isp_s_dpf(struct isp_ic_dev *dev);
int isp_s_ee(struct isp_ic_dev *dev);
int isp_s_exp(struct isp_ic_dev *dev);
int isp_g_expmean(struct isp_ic_dev *dev, u8 * mean);
int isp_s_hist(struct isp_ic_dev *dev);
int isp_g_histmean(struct isp_ic_dev *dev, u32 * mean);
int isp_s_dpcc(struct isp_ic_dev *dev);
int isp_s_flt(struct isp_ic_dev *dev);
int isp_s_cac(struct isp_ic_dev *dev);
int isp_s_deg(struct isp_ic_dev *dev);
int isp_s_ie(struct isp_ic_dev *dev);
int isp_s_vsm(struct isp_ic_dev *dev);
int isp_g_vsm(struct isp_ic_dev *dev, struct isp_vsm_result *vsm);
int isp_s_afm(struct isp_ic_dev *dev);
int isp_g_afm(struct isp_ic_dev *dev, struct isp_afm_result *afm);
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

#endif /* _ISP_IOC_H_ */
