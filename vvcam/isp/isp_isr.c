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

#include <linux/version.h>
#include "isp_ioctl.h"
#include "isp_types.h"
#include "mrv_all_bits.h"
#include "video/vvbuf.h"
#include "isp_driver.h"

extern MrvAllRegister_t *all_regs;

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

int update_dma_buffer(struct isp_ic_dev *dev)
{
	int i;
	unsigned long flags;
	struct isp_mi_context *mi = &dev->mi;
	struct vb2_dc_buf *buf = NULL;
	struct isp_buffer_context dmabuf;

	spin_lock_irqsave(&dev->lock, flags);
	for (i = 0; i < MI_PATH_NUM; ++i) {
		if (!mi->path[i].enable)
			continue;

		buf = vvbuf_pull_buf(dev->bctx);
		if (buf == NULL) {
			dev->frame_loss_cnt[i]++;
			continue;
		}
		dmabuf.path = i;
		if (config_dma_buf(&mi->path[i], buf->dma, &dmabuf)){
			vvbuf_push_buf(dev->bctx,buf);
			continue;
		}
		isp_set_buffer(dev, &dmabuf);
		dev->mi_buf_shd[i] = dev->mi_buf[i];
		dev->mi_buf[i] = buf;
	}
	spin_unlock_irqrestore(&dev->lock, flags);

	return 0;
}

static void isp_fps_stat(struct isp_ic_dev *dev, int path)
{
	uint64_t cur_ns = 0, interval = 0;
	cur_ns = ktime_get_ns();
	interval = ktime_us_delta(cur_ns,dev->last_ns[path]);
	if (interval < 3 * USEC_PER_SEC) {
		dev->frame_cnt[path]++;
	} else {
		if (dev->last_ns[path] > 0) {
			dev->fps[path] = (uint32_t)(++dev->frame_cnt[path]
					* USEC_PER_SEC * 100 / interval);
		}
		dev->frame_cnt[path] = 0;
		dev->last_ns[path] = cur_ns;
	}
}
#ifdef ENABLE_LATENCY_STATISTIC
void isp_latency_statistic(struct isp_ic_dev *dev)
{
	int i = 0;
	uint64_t average_latency_ns = 0;

	if (dev->frame_in_cnt < LATENCY_START_STATISTIC)
		return;

	if (dev->interleave_frames > 0 &&
		dev->interleave_frames < LATENCY_INTERLEVE_FRAME) {
		dev->interleave_frames++;
		return;
	}

	dev->isp_in_timestamp[dev->frame_id_latency]  = dev->frame_in_timestamp;
	dev->isp_out_timestamp[dev->frame_id_latency] = dev->frame_out_timestamp;
	if (dev->frame_id_latency == (LATENCY_FRAME_NUM - 1)) {
		pr_info("isp[%d] isp_in_timestamp(ns) [%lld~%lld]:\n"
			"%lld ,%lld ,%lld ,%lld ,%lld ,%lld ,%lld ,%lld\n"
			"%lld ,%lld ,%lld ,%lld ,%lld ,%lld ,%lld ,%lld\n"
			"%lld ,%lld ,%lld ,%lld ,%lld ,%lld ,%lld ,%lld\n",
			dev->id, dev->frame_in_cnt - LATENCY_FRAME_NUM + 1,
			dev->frame_in_cnt,
			dev->isp_in_timestamp[0], dev->isp_in_timestamp[1],
			dev->isp_in_timestamp[2], dev->isp_in_timestamp[3],
			dev->isp_in_timestamp[4], dev->isp_in_timestamp[5],
			dev->isp_in_timestamp[6], dev->isp_in_timestamp[7],
			dev->isp_in_timestamp[8], dev->isp_in_timestamp[9],
			dev->isp_in_timestamp[10], dev->isp_in_timestamp[11],
			dev->isp_in_timestamp[12], dev->isp_in_timestamp[13],
			dev->isp_in_timestamp[14], dev->isp_in_timestamp[15],
			dev->isp_in_timestamp[16], dev->isp_in_timestamp[17],
			dev->isp_in_timestamp[18], dev->isp_in_timestamp[19],
			dev->isp_in_timestamp[20], dev->isp_in_timestamp[21],
			dev->isp_in_timestamp[22], dev->isp_in_timestamp[23]
		);

		pr_info("isp[%d] isp_out_timestamp(ns)  [%lld~%lld]:\n"
			"%lld ,%lld ,%lld ,%lld ,%lld ,%lld ,%lld ,%lld\n"
			"%lld ,%lld ,%lld ,%lld ,%lld ,%lld ,%lld ,%lld\n"
			"%lld ,%lld ,%lld ,%lld ,%lld ,%lld ,%lld ,%lld\n",
			dev->id, dev->frame_in_cnt - LATENCY_FRAME_NUM + 1,
			dev->frame_in_cnt,
			dev->isp_out_timestamp[0], dev->isp_out_timestamp[1],
			dev->isp_out_timestamp[2], dev->isp_out_timestamp[3],
			dev->isp_out_timestamp[4], dev->isp_out_timestamp[5],
			dev->isp_out_timestamp[6], dev->isp_out_timestamp[7],
			dev->isp_out_timestamp[8], dev->isp_out_timestamp[9],
			dev->isp_out_timestamp[10], dev->isp_out_timestamp[11],
			dev->isp_out_timestamp[12], dev->isp_out_timestamp[13],
			dev->isp_out_timestamp[14], dev->isp_out_timestamp[15],
			dev->isp_out_timestamp[16], dev->isp_out_timestamp[17],
			dev->isp_out_timestamp[18], dev->isp_out_timestamp[19],
			dev->isp_out_timestamp[20], dev->isp_out_timestamp[21],
			dev->isp_out_timestamp[22], dev->isp_out_timestamp[23]
		);

		pr_info("isp[%d] average latency(ns) [%lld~%lld]:\n",
			dev->id, dev->frame_in_cnt - LATENCY_FRAME_NUM + 1,
			dev->frame_in_cnt);
		for (i = 0; i < LATENCY_FRAME_NUM; i++) {
			average_latency_ns +=
				(dev->isp_out_timestamp[i] - dev->isp_in_timestamp[i]);
		}
		pr_info("%lld\n", average_latency_ns / LATENCY_FRAME_NUM);

		dev->frame_id_latency = 0;
		dev->interleave_frames++;
	} else if (dev->frame_id_latency < (LATENCY_FRAME_NUM - 1)) {
		dev->frame_id_latency++;
	}
}
#endif

static void isr_process_frame(struct isp_ic_dev *dev)
{
	int i;
	unsigned long flags;
	struct isp_mi_context *mi = &dev->mi;

	spin_lock_irqsave(&dev->lock, flags);
	for (i = 0; i < MI_PATH_NUM; ++i) {
		if (!mi->path[i].enable)
			continue;

		if (dev->mi_buf_shd[i]) {
			dev->mi_buf_shd[i]->timestamp = dev->frame_in_timestamp;
			vvbuf_ready(dev->bctx, dev->mi_buf_shd[i]->pad, dev->mi_buf_shd[i]);
			dev->mi_buf_shd[i] = NULL;
			isp_fps_stat(dev, i);
#ifdef ENABLE_LATENCY_STATISTIC
			if (i == 0)
				isp_latency_statistic(dev);
#endif
		}
	}
	spin_unlock_irqrestore(&dev->lock, flags);
	tasklet_schedule(&dev->tasklet);
	return;
}

void isp_isr_tasklet(unsigned long arg)
{
	struct isp_ic_dev *dev = (struct isp_ic_dev *)arg;
	update_dma_buffer(dev);
	return;
}

int clean_dma_buffer(struct isp_ic_dev *dev)
{
	int i;
	struct vb2_dc_buf *buf = NULL;
	struct isp_device *isp_dev;
	struct media_pad *remote_pad;
	unsigned long flags;

	if (!dev->free)
		return 0;

	isp_dev = container_of(dev, struct isp_device, ic_dev);

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 19, 0)
	remote_pad = media_entity_remote_pad(&isp_dev->pads[ISP_PAD_SOURCE]);
#else
	remote_pad = media_pad_remote_pad_first(&isp_dev->pads[ISP_PAD_SOURCE]);
#endif
	if (remote_pad && is_media_entity_v4l2_video_device(remote_pad->entity)) {
		/*if isp connect to video, the buf free by video,isp maybe not access by isp,so just empty queue*/
		for (i = 0; i < MI_PATH_NUM; ++i) {
			dev->mi_buf[i]     = NULL;
			dev->mi_buf_shd[i] = NULL;
		}

		spin_lock_irqsave(&dev->bctx->irqlock, flags);
		if (!list_empty(&dev->bctx->dmaqueue))
			list_del_init(&dev->bctx->dmaqueue);
		spin_unlock_irqrestore(&dev->bctx->irqlock, flags);

		return 0;
	} else {
		/*if isp connect to subdev, isp buf alloc by isp driver,need free memory*/
		spin_lock_irqsave(&dev->lock, flags);
		do {
			buf = vvbuf_pull_buf(dev->bctx);
			dev->free(dev, buf);
		} while(buf);

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
		spin_unlock_irqrestore(&dev->lock, flags);
	}

	return 0;
}

void isp_clear_interrupts(struct isp_ic_dev *dev)
{
	u32 isp_mis, mi_mis;

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
}

irqreturn_t isp_hw_isr(int irq, void *data)
{
	unsigned long flags;
	struct isp_ic_dev *dev = (struct isp_ic_dev *)data;
	static const u32 frameendmask = MRV_MI_MP_FRAME_END_MASK |
			MRV_MI_SP_FRAME_END_MASK;
	static const u32 errormask = MRV_MI_WRAP_MP_Y_MASK |
			MRV_MI_WRAP_MP_CB_MASK |
			MRV_MI_WRAP_MP_CR_MASK |
			MRV_MI_WRAP_SP_Y_MASK  |
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
	if ((isp_mis == 0) && (mi_mis == 0)) {
		return IRQ_NONE;
	}

	mi_status = isp_read_reg(dev, REG_ADDR(mi_status));
	if (mi_status & fifofullmask) {
		isp_write_reg(dev, REG_ADDR(mi_status), mi_status);
		pr_debug("MI FIFO full: 0x%x\n", mi_status);
	}

	if (isp_mis & MRV_ISP_MIS_FRAME_IN_MASK) {
		dev->frame_in_cnt++;
		dev->frame_in_timestamp = ktime_get_ns();
	}

	if (isp_mis & MRV_ISP_MIS_FRAME_MASK) {
		spin_lock_irqsave(&dev->irqlock, flags);

		if (dev->cproc.changed) {
			isp_s_cproc(dev);
		}

		if (dev->cc.changed) {
			isp_s_cc(dev);
		}

		if (dev->awb.changed) {
			isp_s_awb(dev);
		}

		if (dev->flt.changed) {
			isp_s_flt(dev);
		}

		if (dev->wdr.changed) {
			isp_s_wdr(dev);
		}

		if (dev->gamma_out.changed) {
			isp_s_gamma_out(dev);
		} else if (dev->gamma_out.enable_changed) {
			isp_enable_gamma_out(dev, dev->gamma_out.enableGamma);
		}

		if (dev->exp.changed) {
			isp_s_exp(dev);
		}

		if (dev->hist.changed) {
			isp_s_hist(dev);
		}

		spin_unlock_irqrestore(&dev->irqlock, flags);
	}

	if (mi_mis & errormask) {
		isp_write_reg(dev, REG_ADDR(mi_ctrl), dev->mi.mi_ctrl);
	}

	if (mi_mis & frameendmask) {
		if (*dev->state == (STATE_DRIVER_STARTED | STATE_STREAM_STARTED)) {
#ifdef ENABLE_LATENCY_STATISTIC
			dev->frame_out_timestamp = ktime_get_ns();
#endif
			isr_process_frame(dev);
		}
	}

	if (isp_mis) {
		memset(&irq_data, 0, sizeof(irq_data));
		irq_data.val = isp_mis;
		if (dev->post_event)
			dev->post_event(dev, &irq_data, sizeof(irq_data));
	}

	return IRQ_HANDLED;
}

