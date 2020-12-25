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
#ifdef ENABLE_IRQ

#include "isp_ioctl.h"
#include "isp_types.h"
#include "mrv_all_bits.h"
#include "video/vvbuf.h"

extern MrvAllRegister_t *all_regs;

#ifdef CONFIG_VIDEOBUF2_DMA_CONTIG
static int config_dma_buf(struct isp_mi_data_path_context *path,
		dma_addr_t dma, struct isp_buffer_context *buf)
{
	u32 size = path->out_width * path->out_height;

	buf->addr_y = dma;
	switch (path->out_mode) {
	case IC_MI_DATAMODE_YUV444:
	case IC_MI_DATAMODE_YUV422:
	case IC_MI_DATAMODE_YUV420:
		if (path->data_layout == IC_MI_DATASTORAGE_PLANAR) {
			buf->size_y = size + ISP_BUF_GAP;
			buf->addr_cb = buf->addr_y + size;
			buf->size_cb = size + ISP_BUF_GAP;
			buf->addr_cr = buf->addr_cb + size;
			buf->size_cr = size + ISP_BUF_GAP;
		} else if (path->data_layout ==
				IC_MI_DATASTORAGE_SEMIPLANAR) {
			buf->size_y = size + ISP_BUF_GAP;
			buf->addr_cb = buf->addr_y + size;
			if (path->out_mode == IC_MI_DATAMODE_YUV420)
				buf->size_cb = (size >> 1) + ISP_BUF_GAP;
			else
				buf->size_cb = size + ISP_BUF_GAP;
		} else if (path->data_layout ==
				IC_MI_DATASTORAGE_INTERLEAVED) {
			buf->size_y = (size << 1) + ISP_BUF_GAP;
		} else
			return -1;
		break;
	case IC_MI_DATAMODE_RAW8:
		buf->size_y = size + ISP_BUF_GAP;
		break;
	case IC_MI_DATAMODE_RAW10:
	case IC_MI_DATAMODE_RAW12:
		buf->size_y = (size << 1) + ISP_BUF_GAP;
		break;
	default:
		pr_err("unsupported out mode:%d\n", path->out_mode);
		return -1;
	}
#ifdef ISP_MP_34BIT
	buf->addr_y  >>= 2;
	buf->addr_cb >>= 2;
	buf->addr_cr >>= 2;
#endif
	return 0;
}
#endif

int update_dma_buffer(struct isp_ic_dev *dev)
{
#ifdef CONFIG_VIDEOBUF2_DMA_CONTIG
	struct isp_mi_context *mi = &dev->mi;
	struct vb2_dc_buf *buf = NULL;
	struct isp_buffer_context dmabuf;
	int i, dequeued;

	for (i = 0; i < MI_PATH_NUM; ++i) {
		if (!mi->path[i].enable)
			continue;
		if (dev->mi_buf[i]) {
			vvbuf_ready(dev->bctx, dev->mi_buf[i]->pad,
					dev->mi_buf[i]);
			dev->mi_buf[i] = NULL;
		}
		dequeued = 1;
		buf = vvbuf_try_dqbuf(dev->bctx);
		if (!buf) {
			buf = dev->mi_buf_shd[i];
			if (!buf)
				return -ENOMEM;
			dev->mi_buf_shd[i] = NULL;
			dequeued = 0;
		} else if (dev->mi_buf_shd[i]) {
			dev->mi_buf[i] = dev->mi_buf_shd[i];
			dev->mi_buf_shd[i] = NULL;
		}

		memset(&dmabuf, 0, sizeof(dmabuf));
		dmabuf.path = i;
		if (config_dma_buf(&mi->path[i], buf->dma, &dmabuf))
			continue;
		isp_set_buffer(dev, &dmabuf);
		dev->mi_buf_shd[i] = buf;
		if (dequeued)
			vvbuf_try_dqbuf_done(dev->bctx, buf);
	}
#endif
	return 0;
}

int clean_dma_buffer(struct isp_ic_dev *dev)
{
#ifdef CONFIG_VIDEOBUF2_DMA_CONTIG
	int i;

	if (!dev->free)
		return 0;

	for (i = 0; i < MI_PATH_NUM; ++i) {
		if (dev->mi_buf[i]) {
			dev->free(dev, dev->mi_buf[i]);
			dev->mi_buf[i] = NULL;
		}
		if (dev->mi_buf_shd[i]) {
			dev->free(dev, dev->mi_buf_shd[i]);
			dev->mi_buf_shd[i] = NULL;
		}
	}
#endif
	return 0;
}

irqreturn_t isp_hw_isr(int irq, void *data)
{
	struct isp_ic_dev *dev = (struct isp_ic_dev *)data;
	static const u32 frameendmask = MRV_MI_MP_FRAME_END_MASK |
#ifdef ISP_MI_BP
			MRV_MI_BP_FRAME_END_MASK |
#endif
			MRV_MI_SP_FRAME_END_MASK;
	static const u32 errormask = MRV_MI_WRAP_MP_Y_MASK |
			MRV_MI_WRAP_MP_CB_MASK |
			MRV_MI_WRAP_MP_CR_MASK |
#ifdef ISP_MI_BP
			MRV_MI_BP_WRAP_R_MASK |
			MRV_MI_BP_WRAP_GR_MASK |
			MRV_MI_BP_WRAP_GB_MASK |
			MRV_MI_BP_WRAP_B_MASK |
#endif
			MRV_MI_WRAP_SP_Y_MASK |
			MRV_MI_WRAP_SP_CB_MASK |
			MRV_MI_WRAP_SP_CR_MASK |
			MRV_MI_FILL_MP_Y_MASK;
	static const u32 fifofullmask = MRV_MI_MP_Y_FIFO_FULL_MASK |
			MRV_MI_MP_CB_FIFO_FULL_MASK |
			MRV_MI_MP_CR_FIFO_FULL_MASK |
			MRV_MI_SP_Y_FIFO_FULL_MASK |
			MRV_MI_SP_CB_FIFO_FULL_MASK |
			MRV_MI_SP_CR_FIFO_FULL_MASK;
	u32 isp_mis, mi_mis, mi_status;
	struct isp_irq_data irq_data;
	int rc = 0;

	if (!dev)
		return IRQ_HANDLED;

	isp_mis = isp_read_reg(dev, REG_ADDR(isp_mis));
	isp_write_reg(dev, REG_ADDR(isp_icr), isp_mis);

#ifdef ISP_MIV1
	mi_mis = isp_read_reg(dev, REG_ADDR(mi_mis));
	isp_write_reg(dev, REG_ADDR(mi_icr), mi_mis);
#elif defined(ISP_MIV2)
	mi_mis = isp_read_reg(dev, REG_ADDR(miv2_mis));
	isp_write_reg(dev, REG_ADDR(miv2_icr), mi_mis);
#else
	mi_mis = 0;
#endif

	mi_status = isp_read_reg(dev, REG_ADDR(mi_status));
	if (mi_status & fifofullmask) {
		isp_write_reg(dev, REG_ADDR(mi_status), mi_status);
		pr_debug("MI FIFO full: 0x%x\n", mi_status);
	}

	if (mi_mis & errormask)
		pr_debug("MI mis error: 0x%x\n", mi_mis);

	if (mi_mis & frameendmask)
		rc = update_dma_buffer(dev);

	if (isp_mis) {
		if (isp_mis & MRV_ISP_MIS_FRAME_MASK) {
			if (dev->isp_update_flag & ISP_FLT_UPDATE) {
				isp_s_flt(dev);
				dev->isp_update_flag &= (~ISP_FLT_UPDATE);
			}
		}

		memset(&irq_data, 0, sizeof(irq_data));
		irq_data.val = isp_mis;
		if (dev->post_event)
			dev->post_event(dev, &irq_data, sizeof(irq_data));
	}
	return IRQ_HANDLED;
}

#endif
