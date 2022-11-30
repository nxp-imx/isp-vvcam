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

#ifdef ISP_MIV2

extern MrvAllRegister_t *all_regs;

int getRawBit(u32 type, u32 *bit, u32 *len)
{
	*len = 16;
	switch (type) {
	case ISP_PICBUF_TYPE_RAW8:
		*bit = 0;
		*len = 8;
		break;
	case ISP_PICBUF_TYPE_RAW10:
	case ISP_PICBUF_TYPE_RAW12:
	case ISP_PICBUF_TYPE_RAW14:
	case ISP_PICBUF_TYPE_RAW16:
		*bit = 4;
		break;
	default:
		pr_err("unsupport raw formt: %d\n", type);
		return -1;
	}
	return 0;
}

static bool isYuv(int type)
{
	return (type == ISP_PICBUF_TYPE_YCbCr444) ||
	    (type == ISP_PICBUF_TYPE_YCbCr422) ||
	    (type == ISP_PICBUF_TYPE_YCbCr420) ||
	    (type == ISP_PICBUF_TYPE_YCbCr400);
}

static bool isRaw(u32 type)
{
	return (type == ISP_PICBUF_TYPE_RAW8) ||
	    (type == ISP_PICBUF_TYPE_RAW10) ||
	    (type == ISP_PICBUF_TYPE_RAW12) ||
	    (type == ISP_PICBUF_TYPE_RAW14) || (type == ISP_PICBUF_TYPE_RAW16);

}

void set_rgb_buffer(struct isp_ic_dev *dev, struct isp_buffer_context *buf)
{
	u32 addr = buf->path == 0 ? REG_ADDR(miv2_mp_y_base_ad_init) :
	    (buf->path ==
	     1) ? REG_ADDR(miv2_sp1_y_base_ad_init) :
	    REG_ADDR(miv2_sp2_y_base_ad_init);
	if (buf->type == ISP_PICBUF_TYPE_RGB888) {
		isp_write_reg(dev, addr, (buf->addr_y & MP_Y_BASE_AD_MASK));
		isp_write_reg(dev, addr + 1 * 4,
			      (buf->size_y & MP_Y_SIZE_MASK));
		isp_write_reg(dev, addr + 2 * 4, 0);
		isp_write_reg(dev, addr + 7 * 4,
			      (buf->addr_cb & MP_CB_BASE_AD_MASK));
		isp_write_reg(dev, addr + 8 * 4,
			      (buf->size_cb & MP_CB_SIZE_MASK));
		isp_write_reg(dev, addr + 9 * 4, 0);
		isp_write_reg(dev, addr + 10 * 4,
			      (buf->addr_cr & MP_CR_BASE_AD_MASK));
		isp_write_reg(dev, addr + 11 * 4,
			      (buf->size_cr & MP_CR_SIZE_MASK));
		isp_write_reg(dev, addr + 12 * 4, 0);
	}
}

void set_yuv_buffer(struct isp_ic_dev *dev, struct isp_buffer_context *buf)
{

	u32 addr = buf->path == 0 ? REG_ADDR(miv2_mp_y_base_ad_init) :
	    (buf->path ==
	     1) ? REG_ADDR(miv2_sp1_y_base_ad_init) :
	    REG_ADDR(miv2_sp2_y_base_ad_init);

	if (isYuv(buf->type)) {
		isp_write_reg(dev, addr, (buf->addr_y & MP_Y_BASE_AD_MASK));
		isp_write_reg(dev, addr + 1 * 4,
			      (buf->size_y & MP_Y_SIZE_MASK));
		isp_write_reg(dev, addr + 2 * 4, 0);
		isp_write_reg(dev, addr + 7 * 4,
			      (buf->addr_cb & MP_CB_BASE_AD_MASK));
		isp_write_reg(dev, addr + 8 * 4,
			      (buf->size_cb & MP_CB_SIZE_MASK));
		isp_write_reg(dev, addr + 9 * 4, 0);
		isp_write_reg(dev, addr + 10 * 4,
			      (buf->addr_cr & MP_CR_BASE_AD_MASK));
		isp_write_reg(dev, addr + 11 * 4,
			      (buf->size_cr & MP_CR_SIZE_MASK));
		isp_write_reg(dev, addr + 12 * 4, 0);
	}
}

void set_raw_buffer(struct isp_ic_dev *dev, struct isp_buffer_context *buf)
{
	if (isRaw(buf->type)) {
		isp_write_reg(dev, REG_ADDR(miv2_mp_raw_base_ad_init),
			      (buf->addr_y & MP_RAW_BASE_AD_MASK));
		isp_write_reg(dev, REG_ADDR(miv2_mp_raw_size_init),
			      (buf->size_y & MP_RAW_SIZE_MASK));
		isp_write_reg(dev, REG_ADDR(miv2_mp_raw_offs_cnt_init), 0);

		isp_write_reg(dev, REG_ADDR(miv2_sp2_raw_base_ad_init),
			      (buf->addr_y & MP_RAW_BASE_AD_MASK));
		isp_write_reg(dev, REG_ADDR(miv2_sp2_raw_size_init),
			      (buf->size_y & MP_RAW_SIZE_MASK));
		isp_write_reg(dev, REG_ADDR(miv2_sp2_raw_offs_cnt_init), 0);
	}
}

int isp_set_buffer(struct isp_ic_dev *dev, struct isp_buffer_context *buf)
{
	if (!dev || !buf) {
		pr_err("NULL pointer %s\n", __func__);
		return -1;
	}

	set_yuv_buffer(dev, buf);
	set_raw_buffer(dev, buf);
	set_rgb_buffer(dev, buf);
	return 0;
}

/* only support read raw */
int isp_ioc_start_dma_read(struct isp_ic_dev *dev, void *args)
{
	struct isp_dma_context dma;
	u32 llength, miv2_imsc, miv2_ctrl, mcm_ctrl, mcm_fmt;
	u32 mcm_bus_cfg;
	u32 mcm_rd_fmt_bit = 0;

	pr_info("enter %s\n", __func__);
	viv_check_retval(copy_from_user(&dma, args, sizeof(dma)));

	miv2_imsc = isp_read_reg(dev, REG_ADDR(miv2_imsc));
	miv2_ctrl = isp_read_reg(dev, REG_ADDR(miv2_ctrl));
	mcm_ctrl = isp_read_reg(dev, REG_ADDR(miv2_mcm_ctrl));
	mcm_fmt = isp_read_reg(dev, REG_ADDR(miv2_mcm_fmt));
	mcm_bus_cfg = isp_read_reg(dev, REG_ADDR(miv2_mcm_bus_cfg));

	getRawBit(dma.type, &mcm_rd_fmt_bit, &llength);

	llength = dma.width * llength / 8;
	REG_SET_SLICE(mcm_fmt, MCM_RD_RAW_BIT, mcm_rd_fmt_bit);
	REG_SET_SLICE(mcm_ctrl, MCM_RD_CFG_UPD, 1);
	REG_SET_SLICE(mcm_ctrl, MCM_RD_AUTO_UPDATE, 1);
	isp_write_reg(dev, REG_ADDR(miv2_mcm_dma_raw_pic_start_ad),
		      (MCM_DMA_RAW_PIC_START_AD_MASK & dma.base));
	isp_write_reg(dev, REG_ADDR(miv2_mcm_dma_raw_pic_width),
		      (MCM_DMA_RAW_PIC_WIDTH_MASK & dma.width));
	isp_write_reg(dev, REG_ADDR(miv2_mcm_dma_raw_pic_llength),
		      (MCM_DMA_RAW_PIC_LLENGTH_MASK & llength));
	isp_write_reg(dev, REG_ADDR(miv2_mcm_dma_raw_pic_size),
		      (MCM_DMA_RAW_PIC_SIZE_MASK & (llength * dma.height)));
	isp_write_reg(dev, REG_ADDR(miv2_mcm_dma_raw_pic_lval),
		      (MCM_DMA_RAW_PIC_WIDTH_MASK & llength));
	isp_write_reg(dev, REG_ADDR(miv2_mcm_fmt), mcm_fmt);
	isp_write_reg(dev, REG_ADDR(miv2_mcm_bus_cfg), mcm_bus_cfg);
	isp_write_reg(dev, REG_ADDR(miv2_imsc), miv2_imsc | 0x01800025);	/* enabled jdp, sp2_raw, mp_raw, mcm */

	miv2_ctrl |= (MCM_RAW_RDMA_START_MASK | MCM_RAW_RDMA_PATH_ENABLE_MASK);
	isp_write_reg(dev, REG_ADDR(miv2_mcm_ctrl), mcm_ctrl);
	isp_write_reg(dev, REG_ADDR(miv2_ctrl), miv2_ctrl);
	return 0;
}

u32 getScaleFactor(u32 src, u32 dst)
{
	if (dst > src) {
		return ((65536 * (src - 1)) / (dst - 1));
	} else if (dst < src) {
		return ((65536 * (dst - 1)) / (src - 1)) + 1;
	}
	return 65536;
}

int set_scaling(int id, struct isp_ic_dev *dev, bool stabilization)
{
	u32 addr, ctrl;
	u32 iw, ih, ow, oh;
	u32 inputWidth, inputHeight, outputWidth, outputHeight;
	u32 scale_hy, scale_hcb, scale_hcr, scale_vy, scale_vc;
	struct isp_mi_data_path_context *path = &dev->mi.path[id];

	if (id == IC_MI_PATH_MAIN) {	/* mp */
		addr = REG_ADDR(mrsz_ctrl);
	} else if (id == IC_MI_PATH_SELF) {	/* sp */
		addr = REG_ADDR(srsz_ctrl);
	} else if (id == IC_MI_PATH_SELF2) {	/* sp2 */
		addr = REG_ADDR(srsz2_ctrl);
	}

	inputWidth = path->in_width;
	inputHeight = path->in_height;
	outputWidth = path->out_width;
	outputHeight = path->out_height;

	if (stabilization) {	/* enabled image stabilization. */
		inputWidth = isp_read_reg(dev, REG_ADDR(isp_is_h_size));
		inputHeight = isp_read_reg(dev, REG_ADDR(isp_is_v_size));
	}

	ctrl = isp_read_reg(dev, addr);
	iw = inputWidth / 2;
	ih = inputHeight;
	ow = outputWidth / 2;
	oh = outputHeight;

	switch (path->in_mode) {
	case IC_MI_DATAMODE_YUV422:
		oh = outputHeight;
		break;
	case IC_MI_DATAMODE_YUV420:
		oh = outputHeight / 2;	/* scale cbcr */
		break;
	default:
		return -EFAULT;
	}

	REG_SET_SLICE(ctrl, MRV_MRSZ_SCALE_HY_ENABLE,
		      inputWidth != outputWidth);
	REG_SET_SLICE(ctrl, MRV_MRSZ_SCALE_VY_ENABLE,
		      inputHeight != outputHeight);
	REG_SET_SLICE(ctrl, MRV_MRSZ_SCALE_HY_UP, inputWidth < outputWidth);
	REG_SET_SLICE(ctrl, MRV_MRSZ_SCALE_VY_UP, inputHeight < outputHeight);
	scale_hy = getScaleFactor(inputWidth, outputWidth);
	scale_vy = getScaleFactor(inputHeight, outputHeight);
	REG_SET_SLICE(ctrl, MRV_MRSZ_SCALE_HC_ENABLE, iw != ow);
	REG_SET_SLICE(ctrl, MRV_MRSZ_SCALE_VC_ENABLE, ih != oh);
	REG_SET_SLICE(ctrl, MRV_MRSZ_SCALE_HC_UP, iw < ow);
	REG_SET_SLICE(ctrl, MRV_MRSZ_SCALE_VC_UP, ih < oh);
	scale_hcr = getScaleFactor(iw, ow);
	scale_hcb = getScaleFactor(iw, ow);
	scale_vc = getScaleFactor(ih, oh);

	/*Need to update immediately*/
	REG_SET_SLICE(ctrl, MRV_MRSZ_CFG_UPD, 1);

	if (id == IC_MI_PATH_MAIN) {
		isp_write_reg(dev, REG_ADDR(mrsz_scale_vc), scale_vc);
		isp_write_reg(dev, REG_ADDR(mrsz_scale_vy), scale_vy);
		isp_write_reg(dev, REG_ADDR(mrsz_scale_hcr), scale_hcr);
		isp_write_reg(dev, REG_ADDR(mrsz_scale_hcb), scale_hcb);
		isp_write_reg(dev, REG_ADDR(mrsz_scale_hy), scale_hy);
		isp_write_reg(dev, REG_ADDR(mrsz_ctrl), ctrl);
	} else if (id == IC_MI_PATH_SELF) {
		isp_write_reg(dev, REG_ADDR(srsz_scale_vc), scale_vc);
		isp_write_reg(dev, REG_ADDR(srsz_scale_vy), scale_vy);
		isp_write_reg(dev, REG_ADDR(srsz_scale_hcr), scale_hcr);
		isp_write_reg(dev, REG_ADDR(srsz_scale_hcb), scale_hcb);
		isp_write_reg(dev, REG_ADDR(srsz_scale_hy), scale_hy);
		isp_write_reg(dev, REG_ADDR(srsz_ctrl), ctrl);
	} else if (id == IC_MI_PATH_SELF2) {
		isp_write_reg(dev, REG_ADDR(srsz2_scale_vc), scale_vc);
		isp_write_reg(dev, REG_ADDR(srsz2_scale_vy), scale_vy);
		isp_write_reg(dev, REG_ADDR(srsz2_scale_hcr), scale_hcr);
		isp_write_reg(dev, REG_ADDR(srsz2_scale_hcb), scale_hcb);
		isp_write_reg(dev, REG_ADDR(srsz2_scale_hy), scale_hy);
		isp_write_reg(dev, REG_ADDR(srsz2_ctrl), ctrl);
	}

	return 0;
}

int calc_raw_lval(int width, int out_mode, int align_mode)
{
	u32 lval;

	if (align_mode == ISP_MI_DATA_ALIGN_16BIT_MODE) {
		if ((out_mode == IC_MI_DATAMODE_RAW10) ||
		    (out_mode == IC_MI_DATAMODE_RAW12) ||
		    (out_mode == IC_MI_DATAMODE_RAW14)) {
			lval = (width + 7) / 8;
		}
	} else if (align_mode == ISP_MI_DATA_ALIGN_128BIT_MODE) {
		if ((out_mode == IC_MI_DATAMODE_RAW10)
		    || (out_mode == IC_MI_DATAMODE_RAW12)
		    || (out_mode == IC_MI_DATAMODE_RAW14)) {
			lval = (width + 127) / 128;
		}
	} else {
		if (out_mode == IC_MI_DATAMODE_RAW10) {
			lval = (width * 10 + 127) / 128;
		} else if (out_mode == IC_MI_DATAMODE_RAW12) {
			lval = (width * 12 + 127) / 128;
		} else if (out_mode == IC_MI_DATAMODE_RAW14) {
			lval = (width * 14 + 127) / 128;
		} else if (out_mode == IC_MI_DATAMODE_RAW16) {
			lval = (width * 16 + 127) / 128;
		} else {
			lval = (width * 8 + 127) / 128;
		}
	}

	return lval;
}

#define PATHNUM 3  /* hw related */

/* only config write bits,  SP2 read bit at 3dnr.c
 read defined is same as write */
struct miv2_format_bit {
	u32 nyv, nv12;
	u32 raw_aligned, yuv_aligned;
	u32 raw_bit, yuv_str;
	u32 yuv_fmt, yuv_bit, jdp_fmt;
};

static struct miv2_format_bit fmt_bit[PATHNUM] = {
	{
		.nyv = 3 << 13, .nv12 = 1 << 12,
		.raw_aligned = 3 << 10,
		.yuv_aligned = 1 << 9,
		.raw_bit = 7 << 6,
		.yuv_str = 3 << 4,
		.yuv_fmt = 3 << 2,
		.yuv_bit = 1 << 1,
		.jdp_fmt = 1,
	},
	{
		.nyv = 3 << 7,
		.nv12 =  1 << 6,
		.yuv_aligned = 1 << 5,
		.yuv_str = 3 << 3,
		.yuv_fmt = 3 << 1,
		.yuv_bit = 1,
	},
	{
		.nyv = 3 << 12,
		.nv12 =  1 << 11,
		.raw_aligned = 3 << 9,
		.yuv_aligned = 1 << 8,
		.raw_bit = 7 << 5,
		.yuv_str = 3 << 3,
		.yuv_fmt = 3 << 1,
		.yuv_bit = 1,
	},
};

u32 bit_shift(u32 i) {
	u32 shift = 0;
	while(!(i&1)) {
		shift++;
		i >>= 1;
	}
	return shift;
}

void mi_set_slice(u32* val, u32 mask, u32 slice)
{
	/* mp, sp1, sp2 have different masks. */
	if (mask) {
		*val &= ~mask;
		*val |= (slice << bit_shift(mask));
	}
}

struct miv2_path_address {
	u32 bus_cfg_addr;
	u32 bus_id_addr;
	u32 path_ctrl_addr;
	u32 format_addr;
	u32 y_length_addr;
	u32 raw_llength;
	u32 raw_pic_width;
	u32 raw_pic_height;
	u32 raw_pic_size;
	u32 ycbcr_enable_bit;
	u32 format_conv_ctrl;
};

void set_data_path(int id, struct isp_mi_data_path_context *path,
		   struct isp_ic_dev *dev)
{
	u32 bus_cfg, bus_id;
	u32 format;
	u32 miv2_ctrl;
	u32 path_ctrl;
	u32 lval;
	u32 acq_proc;
	u32 mcm_bus_cfg = isp_read_reg(dev, REG_ADDR(miv2_mcm_bus_cfg));
	u32 conv_format_ctrl;
	u32 y_length_addr;

	/* please take care the register order */
	struct miv2_path_address  path_list[PATHNUM] = {
		{
			REG_ADDR(miv2_mp_bus_cfg), REG_ADDR(miv2_mp_bus_id), REG_ADDR(miv2_mp_ctrl),
			REG_ADDR(miv2_mp_fmt), REG_ADDR(miv2_mp_y_llength), REG_ADDR(miv2_mp_raw_llength),
			REG_ADDR(miv2_mp_raw_pic_width), REG_ADDR(miv2_mp_raw_pic_height), REG_ADDR(miv2_mp_raw_pic_size),
			MP_YCBCR_PATH_ENABLE_MASK, REG_ADDR(mrsz_format_conv_ctrl),
		},
		{
			REG_ADDR(miv2_sp1_bus_cfg), REG_ADDR(miv2_sp1_bus_id), REG_ADDR(miv2_sp1_ctrl),
			REG_ADDR(miv2_sp1_fmt), REG_ADDR(miv2_sp1_y_llength), 0,
			0, 0, 0,
			SP1_YCBCR_PATH_ENABLE_MASK, REG_ADDR(srsz_phase_format_conv_ctr),
		},
		{
			REG_ADDR(miv2_sp2_bus_cfg), REG_ADDR(miv2_sp2_bus_id), REG_ADDR(miv2_sp2_ctrl),
			REG_ADDR(miv2_sp2_fmt), REG_ADDR(miv2_sp2_y_llength), REG_ADDR(miv2_sp2_raw_llength),
			REG_ADDR(miv2_sp2_raw_pic_width), REG_ADDR(miv2_sp2_raw_pic_height), REG_ADDR(miv2_sp2_raw_pic_size),
			SP2_YCBCR_PATH_ENABLE_MASK, REG_ADDR(srsz2_phase_format_conv_ctr),
		},
	};

	if (!path->enable)
		return;

	if (path->hscale || path->vscale || dev->is.enable) {
		set_scaling(id, dev, dev->is.enable);
	}

	miv2_ctrl = isp_read_reg(dev, REG_ADDR(miv2_ctrl));
	bus_cfg = isp_read_reg(dev, path_list[id].bus_id_addr);
	format = isp_read_reg(dev, path_list[id].format_addr);
	conv_format_ctrl = isp_read_reg(dev, path_list[id].format_conv_ctrl);
	y_length_addr = path_list[id].y_length_addr;

	switch (path->out_mode) {
	case IC_MI_DATAMODE_YUV444:
		mi_set_slice(&format, fmt_bit[id].yuv_fmt, 2);
		miv2_ctrl |= path_list[id].ycbcr_enable_bit;
		REG_SET_SLICE(conv_format_ctrl, MRV_MRSZ_COVERT_OUTPUT, 3);
		REG_SET_SLICE(conv_format_ctrl, MRV_MRSZ_COVERT_INPUT, 2);
		break;
	case IC_MI_DATAMODE_YUV422:
		mi_set_slice(&format, fmt_bit[id].yuv_fmt, 1);
		miv2_ctrl |= path_list[id].ycbcr_enable_bit;
		break;
	case IC_MI_DATAMODE_YUV420:
		mi_set_slice(&format, fmt_bit[id].yuv_fmt, 0);
		miv2_ctrl |= path_list[id].ycbcr_enable_bit;
		break;
	case IC_MI_DATAMODE_YUV400:
	case IC_MI_DATAMODE_JPEG:
		mi_set_slice(&format, fmt_bit[id].jdp_fmt, 1);
		REG_SET_SLICE(miv2_ctrl, MP_JDP_PATH_ENABLE, 1);
		break;
	case IC_MI_DATAMODE_RAW8:
		mi_set_slice(&format, fmt_bit[id].raw_bit, 0);
		REG_SET_SLICE(miv2_ctrl, MP_RAW_PATH_ENABLE, 1);
		REG_SET_SLICE(miv2_ctrl, SP2_RAW_PATH_ENABLE, 1);
		break;
	case IC_MI_DATAMODE_RAW10:
		mi_set_slice(&format, fmt_bit[id].raw_bit, 1);
		REG_SET_SLICE(miv2_ctrl, MP_RAW_PATH_ENABLE, 1);
		REG_SET_SLICE(miv2_ctrl, SP2_RAW_PATH_ENABLE, 1);
		REG_SET_SLICE(bus_cfg, MP_WR_SWAP_RAW, 1);
		break;
	case IC_MI_DATAMODE_RAW12:
		mi_set_slice(&format, fmt_bit[id].raw_bit, 2);
		REG_SET_SLICE(miv2_ctrl, MP_RAW_PATH_ENABLE, 1);
		REG_SET_SLICE(miv2_ctrl, SP2_RAW_PATH_ENABLE, 1);
		REG_SET_SLICE(bus_cfg, MP_WR_SWAP_RAW, 1);
		break;
	case IC_MI_DATAMODE_RAW14:
		mi_set_slice(&format, fmt_bit[id].raw_bit, 3);
		REG_SET_SLICE(miv2_ctrl, MP_RAW_PATH_ENABLE, 1);
		REG_SET_SLICE(miv2_ctrl, SP2_RAW_PATH_ENABLE, 1);
		REG_SET_SLICE(bus_cfg, MP_WR_SWAP_RAW, 1);
		break;
	case IC_MI_DATAMODE_RAW16:
		mi_set_slice(&format, fmt_bit[id].raw_bit, 4);
		REG_SET_SLICE(miv2_ctrl, MP_RAW_PATH_ENABLE, 1);
		REG_SET_SLICE(miv2_ctrl, SP2_RAW_PATH_ENABLE, 1);
		REG_SET_SLICE(bus_cfg, MP_WR_SWAP_RAW, 1);
		break;
	case IC_MI_DATAMODE_RGB888:
		mi_set_slice(&format, fmt_bit[id].yuv_fmt, 2);
		REG_SET_SLICE(conv_format_ctrl, MRV_MRSZ_COVERT_OUTPUT, 6);
		REG_SET_SLICE(conv_format_ctrl, MRV_MRSZ_COVERT_INPUT, 2);
		miv2_ctrl |= path_list[id].ycbcr_enable_bit;
		break;
	default:
		pr_err("mi %s unsupport format: %d", __func__, path->out_mode);
		return;
	}

	switch (path->data_layout) {
	case IC_MI_DATASTORAGE_PLANAR:
		mi_set_slice(&format, fmt_bit[id].yuv_str, 2);
		break;
	case IC_MI_DATASTORAGE_SEMIPLANAR:
		mi_set_slice(&format, fmt_bit[id].yuv_str, 0);
		break;
	case IC_MI_DATASTORAGE_INTERLEAVED:
		mi_set_slice(&format, fmt_bit[id].yuv_str, 1);
		break;
	default:
		break;
	}

	mi_set_slice(&format, fmt_bit[id].yuv_bit, 0);
	mi_set_slice(&format, fmt_bit[id].raw_aligned, path->data_alignMode);
	REG_SET_SLICE(bus_cfg, MP_WR_BURST_LEN, dev->mi.burst_len);
	REG_SET_SLICE(mcm_bus_cfg, MCM_WR_BURST_LEN, dev->mi.burst_len);
	isp_write_reg(dev, y_length_addr, path->out_width);
	isp_write_reg(dev, y_length_addr + 4, path->out_width);
	isp_write_reg(dev, y_length_addr + 8, path->out_height);
	isp_write_reg(dev, y_length_addr + 12,
		      path->out_width * path->out_height);

	if ((id == 0 && (miv2_ctrl & MP_RAW_PATH_ENABLE_MASK))
	    || (id == 2 && (miv2_ctrl & SP2_RAW_PATH_ENABLE_MASK))) {
		lval =
		    calc_raw_lval(path->out_width, path->out_mode,
				  path->data_alignMode);
		lval <<= 4;
		isp_write_reg(dev, path_list[id].raw_llength, lval);
		isp_write_reg(dev, path_list[id].raw_pic_width, path->out_width);
		isp_write_reg(dev, path_list[id].raw_pic_height, path->out_height);
		isp_write_reg(dev, path_list[id].raw_pic_size, path->out_height * lval);
	}

	/* aev2, 3dnr */
	if (id == 0) {
		if (dev->exp2.enable) {
			REG_SET_SLICE(miv2_ctrl, MP_JDP_PATH_ENABLE, 1);
		} else {
			REG_SET_SLICE(miv2_ctrl, MP_JDP_PATH_ENABLE, 0);
		}

		if (isp_read_reg(dev, REG_ADDR(isp_denoise3d_ctrl)) & 0x01) {
			REG_SET_SLICE(miv2_ctrl, SP2_RAW_PATH_ENABLE, 1);
			REG_SET_SLICE(miv2_ctrl, SP2_RAW_RDMA_PATH_ENABLE, 1);
		} else {
			REG_SET_SLICE(miv2_ctrl, SP2_RAW_PATH_ENABLE, 0);
			REG_SET_SLICE(miv2_ctrl, SP2_RAW_RDMA_PATH_ENABLE, 0);
		}

	}

	path_ctrl = isp_read_reg(dev, path_list[id].path_ctrl_addr);
	REG_SET_SLICE(path_ctrl, MP_MI_CFG_UPD, 1);
	REG_SET_SLICE(path_ctrl, MP_AUTO_UPDATE, 1);
	path_ctrl |= (MP_INIT_BASE_EN_MASK | MP_INIT_OFFSET_EN_MASK);
	acq_proc = isp_read_reg(dev, REG_ADDR(isp_acq_prop));
	isp_write_reg(dev, REG_ADDR(isp_acq_prop),
		      acq_proc & ~MRV_ISP_LATENCY_FIFO_SELECTION_MASK);
	bus_id = isp_read_reg(dev, path_list[id].bus_id_addr);
	if (id == 1) {
		bus_id <<= 4;
	}
	bus_id |= MP_WR_ID_EN_MASK;
	if (id == 2) {
		bus_id |= SP2_BUS_SW_EN_MASK;
		REG_SET_SLICE(bus_cfg, SP2_WR_SWAP_Y, 1);
	} else {
		bus_id |= MP_BUS_SW_EN_MASK;
	}
	isp_write_reg(dev, path_list[id].bus_id_addr, bus_id);
	isp_write_reg(dev, path_list[id].bus_cfg_addr, bus_cfg);
	isp_write_reg(dev, REG_ADDR(miv2_mcm_bus_cfg), mcm_bus_cfg);
	isp_write_reg(dev, path_list[id].format_addr, format);
	isp_write_reg(dev, REG_ADDR(miv2_ctrl), miv2_ctrl);
	isp_write_reg(dev, path_list[id].path_ctrl_addr, path_ctrl);
	isp_write_reg(dev, path_list[id].format_conv_ctrl, conv_format_ctrl);

}

int isp_mi_start(struct isp_ic_dev *dev)
{
	int i;
	struct isp_mi_context mi = *(&dev->mi);
	u32 imsc, miv2_mcm_bus_id;
	pr_info("enter %s\n", __func__);

	miv2_mcm_bus_id = isp_read_reg(dev, REG_ADDR(miv2_mcm_bus_id));
	miv2_mcm_bus_id |= MCM_BUS_SW_EN_MASK;
	isp_write_reg(dev, REG_ADDR(miv2_mcm_bus_id), miv2_mcm_bus_id);

	if (isp_read_reg(dev, REG_ADDR(isp_denoise3d_ctrl)) & 0x01) {
		isp_write_reg(dev, REG_ADDR(miv2_sp2_bus_id), 0x084fd56c);
		isp_write_reg(dev, REG_ADDR(miv2_sp2_fmt), 0x0115144a);
		isp_write_reg(dev, REG_ADDR(miv2_sp2_ctrl), 0x238);
	}
	for (i = 0; i < 3; i++) {
		set_data_path(i, &mi.path[i], dev);
	}

	imsc = isp_read_reg(dev, REG_ADDR(miv2_imsc));
	isp_write_reg(dev, REG_ADDR(miv2_imsc),
		      imsc | (MP_YCBCR_FRAME_END_MASK | MP_RAW_FRAME_END_MASK |
			      WRAP_MP_Y_MASK | WRAP_MP_CB_MASK | WRAP_MP_CR_MASK
			      | WRAP_MP_RAW_MASK | WRAP_MP_JDP_MASK |
			      SP1_YCBCR_FRAME_END_MASK | WRAP_SP1_Y_MASK |
			      WRAP_SP1_CB_MASK | WRAP_SP1_CR_MASK |
			      SP2_YCBCR_FRAME_END_MASK | WRAP_SP2_Y_MASK |
			      WRAP_SP2_CB_MASK | WRAP_SP2_CR_MASK |
                  SP2_RAW_FRAME_END_MASK));

	return 0;
}

int isp_mi_stop(struct isp_ic_dev *dev)
{
	pr_info("enter %s\n", __func__);
	isp_write_reg(dev, REG_ADDR(miv2_imsc), 0);

	return 0;
}

u32 isp_read_mi_irq(struct isp_ic_dev *dev)
{
	return isp_read_reg(dev, REG_ADDR(miv2_mis));
}

void isp_reset_mi_irq(struct isp_ic_dev *dev, u32 icr)
{
	isp_write_reg(dev, REG_ADDR(miv2_icr), icr);
}

int isp_set_bp_buffer(struct isp_ic_dev *dev, struct isp_bp_buffer_context *buf)
{
	return 0;
}

#endif
