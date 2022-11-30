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
#ifndef _ISP_DEV_H_
#define _ISP_DEV_H_

#include "isp_version.h"
#include "vvdefs.h"

#define REG_ADDR(x)  ((uint32_t)(uintptr_t)&all_regs->x)

#ifdef ISP_MIV1
#define ISP_BUF_GAP             (1024)
#elif defined ISP_MIV2
#define ISP_BUF_GAP             (0)
#endif

#ifdef ISP_MI_BP
# define MI_PATH_NUM            (3)
#else
# define MI_PATH_NUM            (2)
#endif

struct isp_reg_t {
	u32 offset;
	u32 val;
};

struct ic_window {
	u16 x;
	u16 y;
	u16 width;
	u16 height;
};

struct isp_context {
	u32 mode;
	u32 sample_edge;
	bool hSyncLowPolarity, vSyncLowPolarity;
	u32 bayer_pattern;
	u32 sub_sampling;
	u32 seq_ccir;
	u32 field_selection;
	u32 input_selection;
	u32 latency_fifo;
	struct ic_window acqWindow;  /**< acquisition window */
	struct ic_window ofWindow;   /**< output formatter window */
	struct ic_window isWindow;   /**< image stabilization output window */
	u32 bypass_mode;
	u8 demosaic_threshold;
	u32 stitching_mode;
};

struct isp_digital_gain_cxt {
	/* data */
	bool enable;
	u16 gain_r;
	u16 gain_b;
	u16 gain_gr;
	u16 gain_gb;
};

struct isp_mi_data_path_context {
	bool enable;
	u32 out_mode;	   /**< output format */
	u32 in_mode;		/**< input format */
	u32 data_layout;	/**< layout of data */
	u32 data_alignMode;	/**< align mode of data */
	u32 in_width;
	u32 in_height;
	u32 out_width;
	u32 out_height;
	bool hscale;
	bool vscale;
	int pixelformat;
};

struct isp_dummy_hblank_cxt {
	u8 bp, fp, w, in_hsize;
};
struct isp_mi_context {
	struct isp_mi_data_path_context path[3];
	u32 burst_len;
};

struct isp_bls_context {
	bool enabled;
	u32 mode;
	u16 a, b, c, d;
};

struct isp_tpg_userdefine_mode {
	u16 total, fp, sync, bp, act;
};

struct isp_tpg_context {
	bool enabled;
	u32 image_type;
	u32 bayer_pattern;
	u32 color_depth;
	u32 resolution;
	u16 pixleGap;
	u16 lineGap;
	u16 gapStandard;
	u32 randomSeed;
	struct isp_tpg_userdefine_mode user_mode_h, user_mode_v;
};

struct isp_mcm_context {
	bool bypass_enable;
	u32 vsync_blank;
	u32 vsync_duration;
	u32 hsync_blank;
	u32 hsync_preample;
};

struct isp_mux_context {
	u32 mp_mux;             /**< main path muxer (vi_mp_mux) */
	u32 sp_mux;             /**< self path muxer (vi_dma_spmux) */
	u32 chan_mode;          /**< 1-mp, 2-sp, 4-sp2 */
	u32 ie_mux;             /**< image effects muxer (vi_dma_iemux) */
	u32 dma_read_switch;    /**< dma read switch (vi_dma_switch) */
	u32 if_select;          /**< interface selector (if_select) */
};

struct isp_awb_context {
	bool enable;
	u8 mode;
	u16 gain_r, gain_gr, gain_gb, gain_b;
	struct ic_window window;
	u16 refcb_max_b;
	u16 refcr_max_r;
	u16 max_y;
	u16 max_c_sum;
	u16 min_y_max_g;
	u16 min_c;
	bool changed;
};

struct isp_awb_mean {
	u32 r, g, b;
	u32 no_white_count;
};

struct isp_cnr_context {
	bool enable;
	u32 line_width;
	u32 threshold_1;
	u32 threshold_2;
};

struct isp_cc_context {
	u32 lCoeff[9];
	bool update_curve;
	bool conv_range_y_full, conv_range_c_full;
};

struct isp_xtalk_context {
	u32 lCoeff[9];
	u32 r, g, b;
};

struct isp_gamma_out_context {
	bool enableWB, enableGamma;
	bool changed;
	bool enable_changed;
	u32 mode;
	u32 curve[17];
};

#define CAEMRIC_GRAD_TBL_SIZE 8
#define CAMERIC_DATA_TBL_SIZE 289
#define CAMERIC_MAX_LSC_SECTORS 16
#define CA_CURVE_DATA_TABLE_LEN 65

struct isp_lsc_context {
	/**< correction values of R color part */
	u16 r[CAMERIC_DATA_TBL_SIZE];
	/**< correction values of G (red lines) color part */
	u16 gr[CAMERIC_DATA_TBL_SIZE];
	/**< correction values of G (blue lines) color part */
	u16 gb[CAMERIC_DATA_TBL_SIZE];
	/**< correction values of B color part */
	u16 b[CAMERIC_DATA_TBL_SIZE];
	/**< multiplication factors of x direction */
	u16 x_grad[CAEMRIC_GRAD_TBL_SIZE];
	/**< multiplication factors of y direction */
	u16 y_grad[CAEMRIC_GRAD_TBL_SIZE];
	/**< sector sizes of x direction */
	u16 x_size[CAEMRIC_GRAD_TBL_SIZE];
	/**< sector sizes of y direction */
	u16 y_size[CAEMRIC_GRAD_TBL_SIZE];
};

struct isp_dmoi_context {
	bool enable;
	u8 demoire_area_thr;
	u8 demoire_sat_shrink;
	u16 demoire_r2;
	u16 demoire_r1;
	u8 demoire_t2_shift;
	u8 demoire_t1;

	u16 demoire_edge_r2;
	u16 demoire_edge_r1;

	u8 demoire_edge_t2_shift;
	u16 demoire_edge_t1;
};

struct isp_shap_context {
	bool enabled;
	u8 sharpen_size;
	u16 sharpen_factor_black;
	u16 sharpen_factor_white;
	u16 sharpen_clip_black;
	u16 sharpen_clip_white;
	u16 sharpen_t4_shift;
	u16 sharpen_t3;

	u8 sharpen_t2_shift;
	u16 sharpen_t1;
	u16 sharpen_r3;
	u16 sharpen_r2;
	u16 sharpen_r1;

	u8 dmsc_sharpen_line_shift2;
	u8 dmsc_sharpen_line_shift1;
	u16 dmsc_sharpen_line_t1;
	u16 sharpen_line_strength;
	u16 sharpen_line_r2;
	u16 sharpen_line_r1;
	u8 hf_filt_00;
	u8 hf_filt_01;
	u8 hf_filt_02;
	u8 hf_filt_10;
	u8 hf_filt_11;
	u8 hf_filt_12;
	u8 hf_filt_20;
	u8 hf_filt_21;
	u8 hf_filt_22;
};

struct isp_skin_context {
	bool enable;
	u8 cbcr_mode;
	u8 depurple_sat_shrink;
	u8 depurple_thr;
	u16 cb_thr_max_2047;
	u16 cb_thr_min_2047;
	u16 cr_thr_max_2047;
	u16 cr_thr_min_2047;
	u16 y_thr_max_2047;
	u16 y_thr_min_2047;
};

struct isp_interplaion_thr_cxt {
	u16 intp_dir_thr_min;
	u16 intp_dir_thr_max;
};

struct isp_dmsc_context {
	bool enable;
	u8 demosaic_thr;
	u8 denoise_stren;
	struct isp_interplaion_thr_cxt intp_cxt;
	struct isp_dmoi_context dmoi_cxt;
	struct isp_shap_context shap_cxt;
	struct isp_skin_context skin_cxt;
};

struct isp_ge_context {
	bool enable;
	u16 threshold;
	u16 h_dummy;
};

struct isp_ca_context {
	bool enable;
	u8 mode;
	u16 lut_x[CA_CURVE_DATA_TABLE_LEN];
	u16 lut_luma[CA_CURVE_DATA_TABLE_LEN];
	u16 lut_chroma[CA_CURVE_DATA_TABLE_LEN];
	u16 lut_shift[CA_CURVE_DATA_TABLE_LEN];
};

struct isp_buffer_context {
	u32 type;
	u32 path;
	u32 addr_y, addr_cb, addr_cr;
	u32 size_y, size_cb, size_cr;
};

struct isp_bp_buffer_context {
	u32 addr_r;
	u32 addr_gr;
	u32 addr_gb;
	u32 addr_b;
};

struct isp_dma_context {
	u32 type;
	u32 base;
	u32 width;
	u32 height;
	u32 burst_y;
	u32 burst_c;
};

struct isp_dpf_context {
	bool enable;
	u32 filter_type;
	u32 gain_usage;
	u32 strength_r;
	u32 strength_g;
	u32 strength_b;
	u8 weight_g[6];
	u8 weight_rb[6];
	u16 denoise_talbe[17];
	u32 x_scale;
	u32 nf_gain_r;
	u32 nf_gain_gr;
	u32 nf_gain_gb;
	u32 nf_gain_b;
	bool filter_r_off;
	bool filter_gr_off;
	bool filter_gb_off;
	bool filter_b_off;
};

struct isp_is_context {
	bool enable;
	bool update;
	struct ic_window window;
	u32 recenter;
	u32 max_dx, max_dy;
	u32 displace_x, displace_y;
};

struct isp_ee_context {
	bool enable;
	u8 src_strength;
	u8 strength;
	u8 input_sel;
	u32 y_gain, uv_gain, edge_gain;
};

struct isp_exp_context {
	bool enable;
	u32 mode;
	struct ic_window window;
};

struct isp_hist_context {
	bool enable;
	u32 mode;
	u32 step_size;
	struct ic_window window;
	u8 weight[25];
};

struct isp_dpcc_params {
	u32 line_thresh;
	u32 line_mad_fac;
	u32 pg_fac;
	u32 rnd_thresh;
	u32 rg_fac;
};

struct isp_dpcc_context {
	bool enable;
	u32 mode;
	u32 outmode;
	u32 set_use;
	u32 methods_set[3];
	struct isp_dpcc_params params[3];
	u32 ro_limits;
	u32 rnd_offs;
};

struct isp_flt_context {
	bool enable;
	bool changed;
	u32 denoise;
	u32 sharpen;
	u32 chrV;
	u32 chrH;
};

struct isp_cac_context {
	bool enable;
	u32 hmode, vmode;
	u32 ab, ar, bb, br, cb, cr;
	u32 xns, xnf, yns, ynf;
	u32 hstart, vstart;
};

/* degamma */
struct isp_deg_context {
	bool enable;
	u8 segment[16];
	u16 r[17];
	u16 g[17];
	u16 b[17];
};

struct isp_ie_context {
	bool enable;
	u32 mode;
	u32 color_sel;
	u32 color_thresh;
	u32 sharpen_factor;
	u32 sharpen_thresh;
	int32_t m[9];
	u32 tint_cr;
	u32 tint_cb;
	bool full_range;
};

struct isp_afm_result {
	u32 sum_a, sum_b, sum_c;
	u32 lum_a, lum_b, lum_c;
};

struct isp_afm_context {
	bool enable;
	u32 thresh;
	struct ic_window window[3];
	u32 pixCnt[3];
	bool enableWinId[3];
	u32 lum_shift;
	u32 afm_shift;
	u32 max_pix_cnt;
};

struct isp_vsm_result {
	u32 x, y;
};

struct isp_vsm_context {
	bool enable;
	struct ic_window window;
	u32 h_seg, v_seg;
};

#ifndef WDR3_BIN
#define WDR3_BIN 14
#endif
struct isp_wdr3_context {
	bool enable;
	bool changed;
	bool inited;
	u32 strength;
	u32 max_gain;
	u32 global_strength;
	u32 histogram[WDR3_BIN];
	u32 shift[WDR3_BIN];
	u32 invert_linear[WDR3_BIN];
	u32 invert_curve[WDR3_BIN];
	u32 gamma_pre[WDR3_BIN];
	u32 gamma_up[WDR3_BIN];
	u32 gamma_down[WDR3_BIN];
	u32 entropy[WDR3_BIN];
	u32 distance_weight[WDR3_BIN];
	u32 difference_weight[WDR3_BIN];
};

#define AEV2_DMA_SIZE 4096
struct isp_exp2_context {
	bool enable;
	struct ic_window window;
	/* weight; */
	u8 r, gr, gb, b;
	/* write 4096 EXPV2 mean value to dma by MI MP-JDP path. */
	/* physical address, alloacte by user */
	u64 pa;
};

#define ISP_2DNR_SIGMA_BIN 60
struct isp_2dnr_context {
	bool enable;
	u32 pre_gamma;
	u32 strength;
	u16 sigma[ISP_2DNR_SIGMA_BIN];
#ifdef ISP_2DNR_V2
	u32 sigma_sqr;
	u32 weight;
#endif
};

struct isp_3dnr_compress_context {
	u8 weight_up_y[2];
	u8 weight_down[4];
	u8 weight_up[8];
};
struct isp_3dnr_context {
	bool enable;
	bool update_bin;
	bool enable_h, enable_v;
	bool enable_temperal;
	bool enable_dilate;
	bool init;
	u32 spacial_curve[17];
	u32 temperal_curve[17];
	u32 strength;
	u16 motion_factor;
	u16 delta_factor;
	/* write full denoise3d reference raw image to dma by MI SP2. */
	/* physical address, alloacte by user */
	u64 pa;
	u32 size;
	struct isp_3dnr_compress_context compress;
};

struct isp_3dnr_update {
	u32 thr_edge_h_inv;
	u32 thr_edge_v_inv;
	u32 thr_motion_inv;
	u32 thr_range_s_inv;
	u32 range_t_h;
	u32 range_t_v;
	u32 range_d;
	u32 thr_range_t_inv;
	u32 thr_delta_h_inv;
	u32 thr_delta_v_inv;
	u32 thr_delta_t_inv;
};

struct isp_hdr_context {
	bool enable;
	/* hdr bls */
	u16 width;
	u16 height;
	u8 l_bit_dep, s_bit_dep, vs_bit_dep, ls_bit_dep;
	u8 weight0, weight1, weight2;
	u16 start_linear, norm_factor_mul_linear;
	u16 start_nonlinear, norm_factor_mul_nonlinear;
	u16 dummy_hblank, out_hblank;
	u16 out_vblank;
	u16 long_exp, short_exp, very_short_exp;
	u16 bls[4];
	u16 digal_gain[4];
	u32 reg;
	/* hdr awb */
	u32 r, gr, gb, b;
	/* hdr exp */
	u32 compress_lut[15];
	/* long short, very short */
	u32 ls0, ls1, vs0, vs1;
	u32 ext_bit;
	u32 valid_thresh;
	u32 offset_val;
	u32 sat_thresh;
	u32 combine_weight;
};

struct isp_simp_context {
	bool enable;
	u32 x, y;
	u32 r, g, b;
	u32 transparency_mode;
	u32 ref_image;
};

struct isp_compand_curve_context {
	bool     enable;
	bool     update_curve;
	uint8_t  in_bit;
	uint8_t  out_bit;
	uint8_t  px[64];
	uint32_t x_data[65];
	uint32_t y_data[65];
};

struct isp_compand_bls_context {
	bool     enable;
	uint8_t  bit_width;
	uint32_t a;
	uint32_t b;
	uint32_t c;
	uint32_t d;
};

/* COMPAND */
struct isp_comp_context {
	bool enable;
	struct isp_compand_curve_context expand;
	struct isp_compand_bls_context   bls;
	struct isp_compand_curve_context compress;
};

struct isp_cproc_context {
	bool enable;
	bool changed;
	u32 contrast;
	u32 brightness;
	u32 saturation;
	u32 hue;
	bool y_out_full;
	bool c_out_full;
	bool y_in_full;
};

struct elawb_ellipse_info {
	u32 x, y;		/* ellipse center */
	u32 a1, a2, a3, a4;	/* ellipse axis */
	u32 r_max_sqr;
};

struct isp_elawb_context {
	bool enable;
	u32 id;			/* ellipse id,  1-8,  0 means update all. */
	struct elawb_ellipse_info info[8];
	struct ic_window window;
	u32 r, gr, gb, b;	/* gain */
};

struct isp_gcmono_data {
	u8 basePara[1024];
	u32 px[64];
	u32 dataX[63];
	u32 dataY[64];
};

struct isp_gcmono_context {
	u32 enable;
	u32 mode;
};

struct isp_rgbgamma_data {
	u32 rgbgc_r_px[64];
	u32 rgbgc_r_datax[63];
	u32 rgbgc_r_datay[64];
	u32 rgbgc_g_px[64];
	u32 rgbgc_g_datax[63];
	u32 rgbgc_g_datay[64];
	u32 rgbgc_b_px[64];
	u32 rgbgc_b_datax[63];
	u32 rgbgc_b_datay[64];
};

struct isp_rgbgamma_context {
	bool enable;
};

struct isp_irq_data {
	uint32_t addr;
	uint32_t val;
	uint32_t nop[14];
};

typedef struct isp_wdr_context
{
	bool	enabled;
	bool	changed;	/* the wdr ctrl && reb shift does not have shandow
						egister,need to change after frame end irq. */
	u16 	LumOffset;
	u16 	RgbOffset;
	u16 	Ym[33];
	u8		dY[33];
} isp_wdr_context_t;

struct isp_ic_dev {
	void __iomem *base;
	void __iomem *reset;
	void *rmem;
	int id;
#ifdef ISP8000NANO_V1802
	struct regmap *mix_gpr;
#endif
#if defined(__KERNEL__) && defined(ENABLE_IRQ)
	struct vvbuf_ctx *bctx;
	struct vb2_dc_buf *mi_buf[MI_PATH_NUM];
	struct vb2_dc_buf *mi_buf_shd[MI_PATH_NUM];
	int (*alloc)(struct isp_ic_dev *dev, struct isp_buffer_context *buf);
	int (*free)(struct isp_ic_dev *dev, struct vb2_dc_buf *buf);
	int *state;
	struct tasklet_struct tasklet;
	spinlock_t lock;
#endif

#ifdef __KERNEL__
	spinlock_t irqlock;
#endif

	void (*post_event)(struct isp_ic_dev *dev, void *data, size_t size);

	struct isp_context ctx;
	struct isp_digital_gain_cxt dgain;
	struct isp_bls_context bls;
	struct isp_tpg_context tpg;
	struct isp_mcm_context mcm;
	struct isp_mux_context mux;
	struct isp_awb_context awb;
	struct isp_lsc_context lsc;
	struct isp_gamma_out_context gamma_out;
	struct isp_xtalk_context xtalk;
	struct isp_cc_context cc;
	struct isp_cnr_context cnr;
	struct isp_is_context is;
	struct isp_is_context rawis;
	struct isp_mi_context mi;
	struct isp_dpf_context dpf;
	struct isp_ee_context ee;
	struct isp_exp_context exp;
	struct isp_hist_context hist;
	struct isp_dpcc_context dpcc;
	struct isp_flt_context flt;
	struct isp_cac_context cac;
	struct isp_deg_context deg;
	struct isp_ie_context ie;
	struct isp_vsm_context vsm;
	struct isp_afm_context afm;
	struct isp_wdr3_context wdr3;
	struct isp_exp2_context exp2;
	struct isp_hdr_context hdr;
	struct isp_2dnr_context dnr2;
	struct isp_3dnr_context dnr3;
	struct isp_comp_context comp;
	struct isp_simp_context simp;
	struct isp_cproc_context cproc;
	struct isp_elawb_context elawb;
	struct isp_gcmono_context gcmono;
	struct isp_rgbgamma_context rgbgamma;
	struct isp_dmsc_context demosaic;
	struct isp_ge_context ge;
	struct isp_ca_context ca;
	struct isp_dummy_hblank_cxt hblank;
	struct isp_wdr_context wdr;
	bool streaming;
	bool update_lsc_tbl;
	bool update_gamma_en;
};

struct isp_extmem_info {
	u64 addr;
	u64 size;
};

void isp_write_reg(struct isp_ic_dev *dev, u32 offset, u32 val);
u32 isp_read_reg(struct isp_ic_dev *dev, u32 offset);

#endif /* _ISP_DEV_H_ */
