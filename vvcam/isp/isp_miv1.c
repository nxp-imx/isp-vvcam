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
#include "mrv_all_bits.h"
#include "isp_ioctl.h"
#include "isp_types.h"

#ifdef ISP_MIV1

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

int isp_ioc_start_dma_read(struct isp_ic_dev *dev, void *args)
{
	struct isp_dma_context dma;
	u32 mi_dma_ctrl = isp_read_reg(dev, REG_ADDR(mi_dma_ctrl));
	u32 llength = 0, mcm_rd_fmt_bit = 0;
	u32 mi_imsc = 0, mcm_fmt = 0;

	pr_info("enter %s\n", __func__);
	viv_check_retval(copy_from_user(&dma, args, sizeof(dma)));

	REG_SET_SLICE(mi_dma_ctrl, MRV_MI_DMA_BURST_LEN_LUM, dma.burst_y);
	REG_SET_SLICE(mi_dma_ctrl, MRV_MI_DMA_BURST_LEN_CHROM, dma.burst_c);

	isp_write_reg(dev, REG_ADDR(mi_dma_y_pic_start_ad),
		      (MRV_MI_DMA_Y_PIC_START_AD_MASK & dma.base));
	getRawBit(dma.type, &mcm_rd_fmt_bit, &llength);

	llength = dma.width * llength / 8;
	REG_SET_SLICE(mcm_fmt, MCM_RD_RAW_BIT, mcm_rd_fmt_bit);
	isp_write_reg(dev, REG_ADDR(mi_dma_y_pic_width),
		      (MRV_MI_DMA_Y_PIC_WIDTH_MASK & dma.width));
	isp_write_reg(dev, REG_ADDR(mi_dma_y_llength),
		      (MRV_MI_DMA_Y_LLENGTH_MASK & llength));
	isp_write_reg(dev, REG_ADDR(mi_dma_y_pic_size),
		      (MRV_MI_DMA_Y_PIC_SIZE_MASK & (llength * dma.height)));
	isp_write_reg(dev, REG_ADDR(mi_dma_cb_pic_start_ad), 0);
	isp_write_reg(dev, REG_ADDR(mi_dma_cr_pic_start_ad), 0);
	isp_write_reg(dev, REG_ADDR(mi_dma_ctrl), mi_dma_ctrl);

	isp_write_reg(dev, REG_ADDR(mi_dma_status), 0);
	isp_write_reg(dev, REG_ADDR(mi_dma_y_raw_fmt), mcm_fmt);
	isp_write_reg(dev, REG_ADDR(mi_dma_y_raw_lval),
		      (MRV_MI_DMA_Y_LLENGTH_MASK & llength));

	mi_imsc = isp_read_reg(dev, REG_ADDR(mi_imsc));
	mi_imsc |= MRV_MI_DMA_READY_MASK;
	isp_write_reg(dev, REG_ADDR(mi_imsc), mi_imsc);
	isp_write_reg(dev, REG_ADDR(mi_dma_start), MRV_MI_DMA_START_MASK);
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
	} else {
		return -EINVAL;
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
		oh = outputHeight / 2;	/*  scale cbcr */
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

	REG_SET_SLICE(ctrl, MRV_MRSZ_AUTO_UPD, 1);

	if (id == IC_MI_PATH_MAIN) {
		isp_write_reg(dev, REG_ADDR(mrsz_scale_vc), scale_vc);
		isp_write_reg(dev, REG_ADDR(mrsz_scale_vy), scale_vy);
		isp_write_reg(dev, REG_ADDR(mrsz_scale_hcr), scale_hcr);
		isp_write_reg(dev, REG_ADDR(mrsz_scale_hcb), scale_hcb);
		isp_write_reg(dev, REG_ADDR(mrsz_scale_hy), scale_hy);
		isp_write_reg(dev, REG_ADDR(mrsz_ctrl),
			      ctrl | MRV_MRSZ_CFG_UPD_MASK);
	} else if (id == IC_MI_PATH_SELF) {
		isp_write_reg(dev, REG_ADDR(srsz_scale_vc), scale_vc);
		isp_write_reg(dev, REG_ADDR(srsz_scale_vy), scale_vy);
		isp_write_reg(dev, REG_ADDR(srsz_scale_hcr), scale_hcr);
		isp_write_reg(dev, REG_ADDR(srsz_scale_hcb), scale_hcb);
		isp_write_reg(dev, REG_ADDR(srsz_scale_hy), scale_hy);
		isp_write_reg(dev, REG_ADDR(srsz_ctrl),
			      ctrl | MRV_MRSZ_CFG_UPD_MASK);
	}

	return 0;
}

#ifdef ISP_MI_BP
int isp_bppath_start(struct isp_ic_dev *dev)
{
	struct isp_mi_context mi = *(&dev->mi);
	u32 bp_ctrl = 0, lval = 0;
	struct isp_mi_data_path_context *path = &mi.path[2];
	u32 mi_imsc = isp_read_reg(dev, REG_ADDR(mi_imsc));
	int i;

	pr_info("enter %s\n", __func__);
	bp_ctrl = 0;
	lval = path->out_width;

	if (mi.path[2].enable) {
		bp_ctrl &= ~MRV_MI_BP_WRITE_RAWBIT_MASK;

		if (path->data_alignMode == ISP_MI_DATA_ALIGN_16BIT_MODE) {
			if ((path->out_mode == IC_MI_DATAMODE_RAW10) ||
			    (path->out_mode == IC_MI_DATAMODE_RAW12) ||
			    (path->out_mode == IC_MI_DATAMODE_RAW14)) {
				lval = (path->out_width + 3) / 4;
			}
		} else if (path->data_alignMode ==
			   ISP_MI_DATA_ALIGN_128BIT_MODE) {
			if ((path->out_mode == IC_MI_DATAMODE_RAW10)
			    || (path->out_mode == IC_MI_DATAMODE_RAW12)
			    || (path->out_mode == IC_MI_DATAMODE_RAW14)) {
				lval = (path->out_width * 2 + 126) / 128;
			}
		} else {
			if (path->out_mode == IC_MI_DATAMODE_RAW10) {
				lval = (path->out_width * 10 + 63) / 64;
			} else if (path->out_mode == IC_MI_DATAMODE_RAW12) {
				lval = (path->out_width * 12 + 63) / 64;
			} else if (path->out_mode == IC_MI_DATAMODE_RAW14) {
				lval = (path->out_width * 14 + 63) / 64;
			} else if (path->out_mode == IC_MI_DATAMODE_RAW16) {
				lval = (path->out_width * 16 + 63) / 64;
			} else {
				lval = (path->out_width * 8 + 63) / 64;
			}
		}
		lval <<= 3;
		REG_SET_SLICE(bp_ctrl, BP_WR_RAW_ALIGNED, path->data_alignMode);
		switch (mi.path[2].out_mode) {
		case (IC_MI_DATAMODE_RAW8):
			REG_SET_SLICE(bp_ctrl, MRV_MI_BP_WRITE_RAWBIT,
				      MRV_MI_BP_WRITE_RAWBIT_RAW_8);
			REG_SET_SLICE(bp_ctrl, MRV_MI_BP_WRITE_FORMAT,
				      MRV_MI_BP_WRITE_INTERLEAVE_FORMAT);
			break;
		case (IC_MI_DATAMODE_RAW12):
			REG_SET_SLICE(bp_ctrl, BP_WR_BYTE_SWAP, 1);
			REG_SET_SLICE(bp_ctrl, MRV_MI_BP_WRITE_RAWBIT,
				      MRV_MI_BP_WRITE_RAWBIT_RAW_12);
			REG_SET_SLICE(bp_ctrl, MRV_MI_BP_WRITE_FORMAT,
				      MRV_MI_BP_WRITE_INTERLEAVE_FORMAT);
			break;
		case (IC_MI_DATAMODE_RAW10):
			REG_SET_SLICE(bp_ctrl, BP_WR_BYTE_SWAP, 1);
			REG_SET_SLICE(bp_ctrl, MRV_MI_BP_WRITE_RAWBIT,
				      MRV_MI_BP_WRITE_RAWBIT_RAW_10);
			REG_SET_SLICE(bp_ctrl, MRV_MI_BP_WRITE_FORMAT,
				      MRV_MI_BP_WRITE_INTERLEAVE_FORMAT);
			break;
		default:
			break;
		}
		isp_write_reg(dev, REG_ADDR(mi_bp_wr_size_init),
			      lval * mi.path[2].out_height);
		isp_write_reg(dev, REG_ADDR(mi_bp_pic_width),
			      mi.path[2].out_width);
		isp_write_reg(dev, REG_ADDR(mi_bp_wr_llength), lval);
		isp_write_reg(dev, REG_ADDR(mi_bp_pic_height),
			      mi.path[2].out_height);
		isp_write_reg(dev, REG_ADDR(mi_bp_pic_size),
			      lval * mi.path[2].out_height);
		/* enable frame end irq for  bp path */
		mi_imsc |=
		    MRV_MI_BP_FRAME_END_MASK | MRV_MI_BP_WRAP_R_MASK |
		    MRV_MI_BP_WRAP_GR_MASK | MRV_MI_BP_WRAP_GB_MASK |
		    MRV_MI_BP_WRAP_B_MASK;
	}
	if (!dev->rawis.enable) {
		isp_write_reg(dev, REG_ADDR(isp_raw_is_h_size),
			      mi.path[2].out_width);
		isp_write_reg(dev, REG_ADDR(isp_raw_is_v_size),
			      mi.path[2].out_height);
		isp_write_reg(dev, REG_ADDR(isp_raw_is_ctrl), 0);
	}
	bp_ctrl |= MRV_MI_BP_PATH_ENABLE_MASK;
	isp_write_reg(dev, REG_ADDR(mi_bp_ctrl), bp_ctrl);
	isp_write_reg(dev, REG_ADDR(mi_imsc), mi_imsc);
	return 0;
}
#endif
int isp_mi_start(struct isp_ic_dev *dev)
{
	struct isp_mi_context mi = *(&dev->mi);
	u32 mi_init, mi_ctrl, mi_imsc;
	u32 out_stride;
	int i;
	u8 retry = 3;

	pr_info("enter %s\n", __func__);

	isp_write_reg(dev, REG_ADDR(mrsz_ctrl), 0);
	isp_write_reg(dev, REG_ADDR(mrsz_ctrl_shd), 0);

	for (i = 0; i < 2; i++) {
		if (mi.path[i].hscale || mi.path[i].vscale) {
			set_scaling(i, dev, dev->is.enable);
		}
	}

	mi_init = 0;
	mi_ctrl = 0;
	mi_imsc = 0;
	if (mi.path[0].enable) {
		/* remove update enable bits for offset and base registers */
		mi_init &= ~MRV_MI_MP_OUTPUT_FORMAT_MASK;
		mi_ctrl &= ~MRV_MI_MP_WRITE_FORMAT_MASK;

		/* config mi_init output format for yuv format */
		if (mi.path[0].out_mode <= IC_MI_DATAMODE_YUV400)
			REG_SET_SLICE(mi_init, MRV_MI_MP_OUTPUT_FORMAT,
				      IC_MI_DATAMODE_YUV400 -
				      mi.path[0].out_mode);
		switch (mi.path[0].out_mode) {
		case (IC_MI_DATAMODE_RAW8):
			REG_SET_SLICE(mi_ctrl, MRV_MI_MP_WRITE_FORMAT,
				      MRV_MI_MP_WRITE_FORMAT_RAW_8);
			REG_SET_SLICE(mi_ctrl, MRV_MI_RAW_ENABLE, 1);
			REG_SET_SLICE(mi_init, MRV_MI_MP_OUTPUT_FORMAT,
				      MRV_MI_MP_OUTPUT_FORMAT_RAW8);
			break;
		case (IC_MI_DATAMODE_RAW12):
			REG_SET_SLICE(mi_ctrl, MRV_MI_MP_WRITE_FORMAT,
				      MRV_MI_MP_WRITE_FORMAT_RAW_12);
			REG_SET_SLICE(mi_ctrl, MRV_MI_RAW_ENABLE, 1);
			REG_SET_SLICE(mi_init, MRV_MI_MP_OUTPUT_FORMAT,
				      MRV_MI_MP_OUTPUT_FORMAT_RAW12);
			isp_write_reg(dev, REG_ADDR(mi_output_align_format), 1);
			break;
		case (IC_MI_DATAMODE_RAW10):
			REG_SET_SLICE(mi_ctrl, MRV_MI_MP_WRITE_FORMAT,
				      MRV_MI_MP_WRITE_FORMAT_RAW_12);
			REG_SET_SLICE(mi_ctrl, MRV_MI_RAW_ENABLE, 1);
			REG_SET_SLICE(mi_init, MRV_MI_MP_OUTPUT_FORMAT,
				      MRV_MI_MP_OUTPUT_FORMAT_RAW10);
			isp_write_reg(dev, REG_ADDR(mi_output_align_format), 1);
			break;
		case (IC_MI_DATAMODE_JPEG):
			REG_SET_SLICE(mi_ctrl, MRV_MI_MP_WRITE_FORMAT,
				      MRV_MI_MP_WRITE_FORMAT_PLANAR);
			REG_SET_SLICE(mi_ctrl, MRV_MI_JPEG_ENABLE, 1);
			break;
		case (IC_MI_DATAMODE_YUV444):
		case (IC_MI_DATAMODE_YUV422):
		case (IC_MI_DATAMODE_YUV420):
		case (IC_MI_DATAMODE_YUV400):
			if (mi.path[0].data_layout == IC_MI_DATASTORAGE_PLANAR) {
				REG_SET_SLICE(mi_ctrl, MRV_MI_MP_WRITE_FORMAT,
					      MRV_MI_MP_WRITE_FORMAT_PLANAR);
			} else if (mi.path[0].data_layout ==
				   IC_MI_DATASTORAGE_SEMIPLANAR) {
				REG_SET_SLICE(mi_ctrl, MRV_MI_MP_WRITE_FORMAT,
					      MRV_MI_MP_WRITE_FORMAT_SEMIPLANAR);
			} else if (mi.path[0].data_layout ==
				   IC_MI_DATASTORAGE_INTERLEAVED) {
				REG_SET_SLICE(mi_ctrl, MRV_MI_MP_WRITE_FORMAT,
					      MRV_MI_MP_WRITE_FORMAT_INTERLEAVED);
			} else {
				break;
			}
			REG_SET_SLICE(mi_ctrl, MRV_MI_MP_ENABLE, 1);
			break;
		default:
			break;
		}

		out_stride = mi.path[0].data_layout ==
		    IC_MI_DATASTORAGE_INTERLEAVED ?
		    (mi.path[0].out_width * 2) : (mi.path[0].out_width);
		isp_write_reg(dev, REG_ADDR(mi_mp_y_pic_width), out_stride);
		isp_write_reg(dev, REG_ADDR(mi_mp_y_llength), out_stride);
		isp_write_reg(dev, REG_ADDR(mi_mp_y_pic_height),
			      mi.path[0].out_height);
		isp_write_reg(dev, REG_ADDR(mi_mp_y_pic_size),
			      out_stride * mi.path[0].out_height);

		/* workaround to resolve the problem that the mi_mp_y_pic_width can't be written */
		for(i = 0; i < retry; i++) {
			if(isp_read_reg(dev, REG_ADDR(mi_mp_y_pic_width)) != out_stride) {
				isp_write_reg(dev, REG_ADDR(mi_mp_y_pic_width), out_stride);
			} else {
				break;
			}
		}
		if(retry == i) {
			pr_info("%s: update mi_mp_y_pic_width error!\n", __func__);
		}

		/* enable frame end irq for  main path */
		mi_imsc |=
		    (MRV_MI_MP_FRAME_END_MASK | MRV_MI_WRAP_MP_Y_MASK |
		     MRV_MI_WRAP_MP_CB_MASK | MRV_MI_WRAP_MP_CR_MASK);
	}

	if (mi.path[1].enable) {
		/* setup mi for self-path */
		mi_ctrl &= ~(MRV_MI_SP_WRITE_FORMAT_MASK);
		REG_SET_SLICE(mi_ctrl, MRV_MI_SP_INPUT_FORMAT,
			      mi.path[1].in_mode - 1);
		REG_SET_SLICE(mi_ctrl, MRV_MI_SP_OUTPUT_FORMAT,
			      mi.path[1].out_mode - 1);

		switch (mi.path[1].out_mode) {
		case (IC_MI_DATAMODE_RGB888):
		case (IC_MI_DATAMODE_RGB666):
		case (IC_MI_DATAMODE_RGB565):
			REG_SET_SLICE(mi_ctrl, MRV_MI_SP_WRITE_FORMAT,
				      MRV_MI_SP_WRITE_FORMAT_RGB_INTERLEAVED);
			break;
		case (IC_MI_DATAMODE_YUV444):
		case (IC_MI_DATAMODE_YUV400):
			if (mi.path[1].data_layout == IC_MI_DATASTORAGE_PLANAR) {
				REG_SET_SLICE(mi_ctrl, MRV_MI_SP_WRITE_FORMAT,
					      MRV_MI_SP_WRITE_FORMAT_PLANAR);
			}
			break;
		case (IC_MI_DATAMODE_YUV422):
			switch (mi.path[1].data_layout) {
			case (IC_MI_DATASTORAGE_PLANAR):
				REG_SET_SLICE(mi_ctrl, MRV_MI_SP_WRITE_FORMAT,
					      MRV_MI_SP_WRITE_FORMAT_PLANAR);
				break;
			case (IC_MI_DATASTORAGE_SEMIPLANAR):
				REG_SET_SLICE(mi_ctrl, MRV_MI_SP_WRITE_FORMAT,
					      MRV_MI_SP_WRITE_FORMAT_SEMIPLANAR);
				break;
			case (IC_MI_DATASTORAGE_INTERLEAVED):
				REG_SET_SLICE(mi_ctrl, MRV_MI_SP_WRITE_FORMAT,
					      MRV_MI_SP_WRITE_FORMAT_INTERLEAVED);
				break;
			default:
				break;
			}
			break;
		case (IC_MI_DATAMODE_YUV420):
			switch (mi.path[1].data_layout) {
			case (IC_MI_DATASTORAGE_PLANAR):
				REG_SET_SLICE(mi_ctrl, MRV_MI_SP_WRITE_FORMAT,
					      MRV_MI_SP_WRITE_FORMAT_PLANAR);
				break;
			case (IC_MI_DATASTORAGE_SEMIPLANAR):
				REG_SET_SLICE(mi_ctrl, MRV_MI_SP_WRITE_FORMAT,
					      MRV_MI_SP_WRITE_FORMAT_SEMIPLANAR);
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}

		out_stride = mi.path[1].data_layout ==
		    IC_MI_DATASTORAGE_INTERLEAVED ?
		    mi.path[1].out_width * 2 : mi.path[1].out_width;
		REG_SET_SLICE(mi_ctrl, MRV_MI_SP_ENABLE, 1);
		isp_write_reg(dev, REG_ADDR(mi_sp_y_pic_width), out_stride);
		isp_write_reg(dev, REG_ADDR(mi_sp_y_llength), out_stride);
		isp_write_reg(dev, REG_ADDR(mi_sp_y_pic_height),
			      mi.path[1].out_height);
		isp_write_reg(dev, REG_ADDR(mi_sp_y_pic_size),
			      out_stride * mi.path[1].out_height);
		/* enable frame end interrupt on self path */
		mi_imsc |=
		    (MRV_MI_SP_FRAME_END_MASK | MRV_MI_WRAP_SP_Y_MASK |
		     MRV_MI_WRAP_SP_CB_MASK | MRV_MI_WRAP_SP_CR_MASK);
	}

	*dev->state |= STATE_DRIVER_STARTED;

	mi_ctrl |= (MRV_MI_INIT_BASE_EN_MASK | MRV_MI_INIT_OFFSET_EN_MASK);
	REG_SET_SLICE(mi_ctrl, MRV_MI_BURST_LEN_CHROM, mi.burst_len);
	REG_SET_SLICE(mi_ctrl, MRV_MI_BURST_LEN_LUM, mi.burst_len);
	isp_write_reg(dev, REG_ADDR(mi_ctrl), mi_ctrl | 0x2000);
	REG_SET_SLICE(mi_init, MRV_MI_MI_CFG_UPD, 1);

	isp_write_reg(dev, REG_ADDR(mi_imsc), mi_imsc);
#ifdef ISP_MI_BP
	isp_bppath_start(dev);
#endif

	/*set memory for first frame, need set MRV_MI_MI_CFG_UPD bit to update memory to mi shadow address*/
	update_dma_buffer(dev);

	isp_write_reg(dev, REG_ADDR(mi_init), mi_init);

	/*prepare  memory for next frame */
	update_dma_buffer(dev);
	return 0;
}

int isp_mi_stop(struct isp_ic_dev *dev)
{
	u32 mi_ctrl = 0, mi_init = 0;
	pr_info("enter %s\n", __func__);

	isp_write_reg(dev, REG_ADDR(mi_imsc), 0);

	/* disable mi path */
	mi_ctrl = isp_read_reg(dev, REG_ADDR(mi_ctrl));
	REG_SET_SLICE(mi_ctrl, MRV_MI_MP_ENABLE, 0);
	REG_SET_SLICE(mi_ctrl, MRV_MI_SP_ENABLE, 0);
	REG_SET_SLICE(mi_ctrl, MRV_MI_JPEG_ENABLE, 0);
	REG_SET_SLICE(mi_ctrl, MRV_MI_RAW_ENABLE, 0);
	isp_write_reg(dev, REG_ADDR(mi_ctrl), mi_ctrl);

	mi_init = isp_read_reg(dev, REG_ADDR(mi_init));
	REG_SET_SLICE(mi_init, MRV_MI_MI_CFG_UPD, 1);
	isp_write_reg(dev, REG_ADDR(mi_init), mi_init);

	if (*dev->state & STATE_DRIVER_STARTED) {
		*dev->state &= ~STATE_DRIVER_STARTED;
		clean_dma_buffer(dev);
	}
	return 0;
}

int isp_set_buffer(struct isp_ic_dev *dev, struct isp_buffer_context *buf)
{
	u32 addr;

	if (!dev || !buf) {
		pr_err("NULL pointer %s\n", __func__);
		return -EINVAL;
	}

	addr = buf->path == 0 ?
	    REG_ADDR(mi_mp_y_base_ad_init) : REG_ADDR(mi_sp_y_base_ad_init);
	isp_write_reg(dev, addr, (buf->addr_y & MRV_MI_MP_Y_BASE_AD_INIT_MASK));
	isp_write_reg(dev, addr + 1 * 4,
		      (buf->size_y & MRV_MI_MP_Y_SIZE_INIT_MASK));
	isp_write_reg(dev, addr + 2 * 4, 0);
	isp_write_reg(dev, addr + 5 * 4,
		      (buf->addr_cb & MRV_MI_MP_CB_BASE_AD_INIT_MASK));
	isp_write_reg(dev, addr + 6 * 4,
		      (buf->size_cb & MRV_MI_MP_CB_SIZE_INIT_MASK));
	isp_write_reg(dev, addr + 7 * 4, 0);
	isp_write_reg(dev, addr + 9 * 4,
		      (buf->addr_cr & MRV_MI_MP_CR_BASE_AD_INIT_MASK));
	isp_write_reg(dev, addr + 10 * 4,
		      (buf->size_cr & MRV_MI_MP_CR_SIZE_INIT_MASK));
	isp_write_reg(dev, addr + 11 * 4, 0);

	return 0;
}

int isp_set_bp_buffer(struct isp_ic_dev *dev, struct isp_bp_buffer_context *buf)
{
#ifndef ISP_MI_BP
	pr_err("unsupported function: %s", __func__);
	return -EINVAL;
#else
	isp_write_reg(dev, REG_ADDR(mi_bp_r_base_ad_init),
		      (buf->addr_r & BP_R_BASE_AD_INIT_MASK));
	isp_write_reg(dev, REG_ADDR(mi_bp_gr_base_ad_init),
		      (buf->addr_gr & BP_GR_BASE_AD_INIT_MASK));

	isp_write_reg(dev, REG_ADDR(mi_bp_gb_base_ad_init),
		      (buf->addr_gb & BP_GB_BASE_AD_INIT_MASK));
	isp_write_reg(dev, REG_ADDR(mi_bp_b_base_ad_init),
		      (buf->addr_b & BP_B_BASE_AD_INIT_MASK));
	return 0;
#endif
}

u32 isp_read_mi_irq(struct isp_ic_dev * dev)
{
	return isp_read_reg(dev, REG_ADDR(mi_mis));
}

void isp_reset_mi_irq(struct isp_ic_dev *dev, u32 icr)
{
	isp_write_reg(dev, REG_ADDR(mi_icr), icr);
}

#endif
