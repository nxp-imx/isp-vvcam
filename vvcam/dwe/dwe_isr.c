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

#include "dwe_driver.h"
#include "video/vvbuf.h"
#include "dwe_ioctl.h"
#include "dwe_regs.h"

void dwe_isr_tasklet(unsigned long arg)
{
	int which;
	u32 dewarp_ctrl;
	unsigned long flags;
	unsigned long irq_flags;
	struct dwe_ic_dev *dev = (struct dwe_ic_dev *)(arg);
	struct vb2_dc_buf *sink_buf, *sink_next;

	spin_lock_irqsave(&dev->irqlock, flags);
	dwe_enable_bus(dev, 0);
	do {
		dev->src = vvbuf_pull_buf(dev->sink_bctx);
		if (dev->src == NULL) {
			dev->hardware_status = HARDWARE_IDLE;
			spin_unlock_irqrestore(&dev->irqlock, flags);
			return;
		}

		dev->index = dev->get_index(dev, dev->src);
		if ( *dev->state[dev->index]  != (STATE_DRIVER_STARTED | STATE_STREAM_STARTED) ) {
			vvbuf_ready(dev->sink_bctx, dev->src->pad, dev->src);
			dev->src = NULL;
			spin_lock_irqsave(&dev->sink_bctx->irqlock, irq_flags);
			list_for_each_entry_safe(sink_buf, sink_next, &(dev->sink_bctx->dmaqueue), irqlist) {
				if (dev->get_index(dev, sink_buf) == dev->index) {
					list_del(&sink_buf->irqlist);
					vvbuf_ready(dev->sink_bctx, sink_buf->pad, sink_buf);
				}
			}
			spin_unlock_irqrestore(&dev->sink_bctx->irqlock, irq_flags);
			continue;
		}
		which = dev->which[dev->index];
		if (dev->dist_map[dev->index][which] == (dma_addr_t)NULL) {
			vvbuf_ready(dev->sink_bctx, dev->src->pad, dev->src);
			dev->src = NULL;
			continue;
		}
		dev->dst = vvbuf_pull_buf(dev->src_bctx[dev->index]);
		if (dev->dst == NULL) {
			vvbuf_ready(dev->sink_bctx, dev->src->pad, dev->src);
			dev->src = NULL;
			continue;
		}
	} while (dev->dst == NULL);

	dev->dst->timestamp = dev->src->timestamp;
	dwe_s_params(dev, &dev->info[dev->index][which]);
	dwe_set_buffer(dev, &dev->info[dev->index][which], dev->dst->dma);
	dwe_set_lut(dev, dev->dist_map[dev->index][which]);
	dwe_start_dma_read(dev, &dev->info[dev->index][which], dev->src->dma);
	dewarp_ctrl = dwe_read_reg(dev, DEWARP_CTRL);
	dwe_write_reg(dev, DEWARP_CTRL, dewarp_ctrl | 2);
	dwe_write_reg(dev, DEWARP_CTRL, dewarp_ctrl);
	dwe_write_reg(dev, INTERRUPT_STATUS, INT_MSK_STATUS_MASK);
	dwe_enable_bus(dev, 1);
	dev->curmap[dev->index][which] = dev->dist_map[dev->index][which];
	spin_unlock_irqrestore(&dev->irqlock, flags);
#ifdef ENABLE_LATENCY_STATISTIC
	dev->dwe_frame_cnt++;
	dev->frame_in_timestamp = ktime_get_ns();
#endif
}

void dwe_clear_interrupts(struct dwe_ic_dev *dev)
{
	u32 status;
	u32 clr;
	status = dwe_read_reg(dev, INTERRUPT_STATUS);
	clr = (status & 0xFF) << 24;
	dwe_write_reg(dev, INTERRUPT_STATUS, clr);
}

#ifdef ENABLE_LATENCY_STATISTIC
void dwe_latency_statistic(struct dwe_ic_dev *dev)
{
	int i = 0;
	uint64_t average_latency_ns = 0;

	if (dev->dwe_frame_cnt < LATENCY_START_STATISTIC)
		return;

	if (dev->interleave_frames > 0 &&
		dev->interleave_frames <= LATENCY_INTERLEVE_FRAME) {
		dev->interleave_frames++;
		return;
	}

	dev->interleave_frames = 0;
	dev->dwe_in_timestamp[dev->frame_id_latency]  = dev->frame_in_timestamp;
	dev->dwe_out_timestamp[dev->frame_id_latency] = dev->frame_out_timestamp;
	if (dev->frame_id_latency == (LATENCY_FRAME_NUM - 1)) {
		pr_info("dwe[%d] dwe_in_timestamp(ns) [%lld~%lld]:\n"
			"%lld ,%lld ,%lld ,%lld ,%lld ,%lld ,%lld ,%lld\n"
			"%lld ,%lld ,%lld ,%lld ,%lld ,%lld ,%lld ,%lld\n"
			"%lld ,%lld ,%lld ,%lld ,%lld ,%lld ,%lld ,%lld\n",
			dev->index, dev->dwe_frame_cnt - LATENCY_FRAME_NUM + 1,
			dev->dwe_frame_cnt,
			dev->dwe_in_timestamp[0], dev->dwe_in_timestamp[1],
			dev->dwe_in_timestamp[2], dev->dwe_in_timestamp[3],
			dev->dwe_in_timestamp[4], dev->dwe_in_timestamp[5],
			dev->dwe_in_timestamp[6], dev->dwe_in_timestamp[7],
			dev->dwe_in_timestamp[8], dev->dwe_in_timestamp[9],
			dev->dwe_in_timestamp[10], dev->dwe_in_timestamp[11],
			dev->dwe_in_timestamp[12], dev->dwe_in_timestamp[13],
			dev->dwe_in_timestamp[14], dev->dwe_in_timestamp[15],
			dev->dwe_in_timestamp[16], dev->dwe_in_timestamp[17],
			dev->dwe_in_timestamp[18], dev->dwe_in_timestamp[19],
			dev->dwe_in_timestamp[20], dev->dwe_in_timestamp[21],
			dev->dwe_in_timestamp[22], dev->dwe_in_timestamp[23]
		);

		pr_info("dwe[%d] dwe_out_timestamp(ns) [%lld~%lld]:\n"
			"%lld ,%lld ,%lld ,%lld ,%lld ,%lld ,%lld ,%lld\n"
			"%lld ,%lld ,%lld ,%lld ,%lld ,%lld ,%lld ,%lld\n"
			"%lld ,%lld ,%lld ,%lld ,%lld ,%lld ,%lld ,%lld\n",
			dev->index, dev->dwe_frame_cnt - LATENCY_FRAME_NUM + 1,
			dev->dwe_frame_cnt,
			dev->dwe_out_timestamp[0], dev->dwe_out_timestamp[1],
			dev->dwe_out_timestamp[2], dev->dwe_out_timestamp[3],
			dev->dwe_out_timestamp[4], dev->dwe_out_timestamp[5],
			dev->dwe_out_timestamp[6], dev->dwe_out_timestamp[7],
			dev->dwe_out_timestamp[8], dev->dwe_out_timestamp[9],
			dev->dwe_out_timestamp[10], dev->dwe_out_timestamp[11],
			dev->dwe_out_timestamp[12], dev->dwe_out_timestamp[13],
			dev->dwe_out_timestamp[14], dev->dwe_out_timestamp[15],
			dev->dwe_out_timestamp[16], dev->dwe_out_timestamp[17],
			dev->dwe_out_timestamp[18], dev->dwe_out_timestamp[19],
			dev->dwe_out_timestamp[20], dev->dwe_out_timestamp[21],
			dev->dwe_out_timestamp[22], dev->dwe_out_timestamp[23]
		);

		pr_info("dwe[%d] average latency(ns) [%lld~%lld]:\n",
			dev->index, dev->dwe_frame_cnt - LATENCY_FRAME_NUM + 1,
			dev->dwe_frame_cnt);
		for (i = 0; i < LATENCY_FRAME_NUM; i++) {
			average_latency_ns +=
				(dev->dwe_out_timestamp[i] - dev->dwe_in_timestamp[i]);
		}
		pr_info("%lld\n", average_latency_ns / LATENCY_FRAME_NUM);

		dev->frame_id_latency = 0;
		dev->interleave_frames++;
	} else if (dev->frame_id_latency < (LATENCY_FRAME_NUM - 1)) {
		dev->frame_id_latency++;
	}
}
#endif

irqreturn_t dwe_hw_isr(int irq, void *data)
{
	struct dwe_ic_dev *dev = (struct dwe_ic_dev *)data;
	u32 status;
	u32 clr;
	unsigned long flags;
	int dwe_status_active = (STATE_DRIVER_STARTED | STATE_STREAM_STARTED);

	if (!dev)
		return IRQ_HANDLED;

	status = dwe_read_reg(dev, INTERRUPT_STATUS);
	clr = (status & 0xFF) << 24;
	dwe_write_reg(dev, INTERRUPT_STATUS, clr);

	if ((dev->state[0] && (*dev->state[0] == dwe_status_active)) ||
	    (dev->state[1] && (*dev->state[1] == dwe_status_active))) {
		if (status & INT_FRAME_DONE) {
#ifdef ENABLE_LATENCY_STATISTIC
			dev->frame_out_timestamp = ktime_get_ns();
			dwe_latency_statistic(dev);
#endif
			spin_lock_irqsave(&dev->irqlock, flags);
			if (dev->src) {
				vvbuf_ready(dev->sink_bctx, dev->src->pad, dev->src);
				dev->src = NULL;
			}
			if (dev->dst) {
				vvbuf_ready(dev->src_bctx[dev->index], dev->dst->pad, dev->dst);
				dev->dst = NULL;
			}
			spin_unlock_irqrestore(&dev->irqlock, flags);
			tasklet_schedule(&dev->tasklet);
		} else {
			spin_lock_irqsave(&dev->irqlock, flags);
			if (dev->src) {
				vvbuf_ready(dev->sink_bctx, dev->src->pad, dev->src);
				dev->src = NULL;
			}
			if (dev->dst) {
				vvbuf_push_buf(dev->src_bctx[dev->index], dev->dst);
				dev->dst = NULL;
			}
			dwe_enable_bus(dev, 0);
			dev->hardware_status = HARDWARE_IDLE;
			spin_unlock_irqrestore(&dev->irqlock, flags);
		}
	}
	else {
		dev->hardware_status = HARDWARE_IDLE;
	}

	return IRQ_HANDLED;
}

void dwe_clean_src_memory(struct dwe_ic_dev *dev)
{
	unsigned long flags;
	struct vb2_dc_buf *buf;

	spin_lock_irqsave(&dev->irqlock, flags);
	do {
		buf = vvbuf_pull_buf(dev->sink_bctx);
		if (buf)
			vvbuf_ready(dev->sink_bctx, buf->pad, buf);
	} while(buf);

	if (dev->src) {
		vvbuf_ready(dev->sink_bctx, dev->src->pad, dev->src);
		dev->src = NULL;
	}
	dev->dst = NULL;
	spin_unlock_irqrestore(&dev->irqlock, flags);
}

