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
#if defined(__KERNEL__) && defined(ENABLE_IRQ)

#include "dwe_ioctl.h"
#include "dwe_regs.h"
#include "video/video.h"

static DEFINE_SPINLOCK(irqlock);

static int update_dma_buffer(struct dwe_ic_dev *dev)
{
#ifdef CONFIG_VIDEOBUF2_DMA_CONTIG
	int index;
	u32 no_dist_map_err, overflow_err;
	u32 dewarp_ctrl;

	dwe_enable_bus(dev, 0);
	if (dev->dst)
		return -EBUSY;
	if (dev->src) {
		viv_loop_source(RESV_STREAMID_DWE, dev->src);
		dev->src = NULL;
	}
	dev->src = viv_get_source(RESV_STREAMID_DWE);
	if (!dev->src) {
		dev->error |= BUF_ERR_UNDERFLOW;
		return -ENOMEM;
	}
	dev->error &= ~BUF_ERR_UNDERFLOW;
	if (dev->src->group == RESV_STREAMID_ISP0) {
		index = 0;
		no_dist_map_err = BUF_ERR_NO_DIST_MAP0;
		overflow_err = BUF_ERR_OVERFLOW0;
	} else if (dev->src->group == RESV_STREAMID_ISP1) {
		index = 1;
		no_dist_map_err = BUF_ERR_NO_DIST_MAP1;
		overflow_err = BUF_ERR_OVERFLOW1;
	} else {
		dev->src = NULL;
		return -EINVAL;
	}
	if (!dev->dist_map[index]) {
		dev->error |= no_dist_map_err;
		dev->src = NULL;
		return -ENOMEM;
	}
	dev->error &= ~no_dist_map_err;
	dev->dst = viv_dequeue_sg_buffer(RESV_STREAMID_DWE, dev->src);
	if (!dev->dst)
		dev->dst = viv_dequeue_buffer(dev->src->group);
	if (!dev->dst) {
		dev->error |= overflow_err;
		dev->src = NULL;
		return -ENOMEM;
	}
	dev->error &= ~overflow_err;
	dwe_set_buffer(dev, dev->dst->dma);
	dwe_set_lut(dev, dev->dist_map[index]);
	dwe_start_dma_read(dev, dev->src->dma);
	dewarp_ctrl = dwe_read_reg(dev, DEWARP_CTRL);
	dwe_write_reg(dev, DEWARP_CTRL, dewarp_ctrl | 2);
	dwe_write_reg(dev, DEWARP_CTRL, dewarp_ctrl);
	dwe_write_reg(dev, INTERRUPT_STATUS, INT_MSK_STATUS_MASK);
	dwe_enable_bus(dev, 1);
#endif
	return 0;
}

int dwe_on_src_notify(int streamid, void *data)
{
	struct dwe_ic_dev *dev = (struct dwe_ic_dev *)data;
	int rc = 0;
	unsigned long flags;

	spin_lock_irqsave(&irqlock, flags);
	if (dev && (dev->error & BUF_ERR_UNDERFLOW))
		rc = update_dma_buffer(dev);
	spin_unlock_irqrestore(&irqlock, flags);
	return rc;
}

int dwe_on_dst_notify(int streamid, void *data)
{
	struct dwe_ic_dev *dev = (struct dwe_ic_dev *)data;
	int rc = 0;
	u32 overflow_err;
	unsigned long flags;

	if (streamid == RESV_STREAMID_ISP0)
		overflow_err = BUF_ERR_OVERFLOW0;
	else if (streamid == RESV_STREAMID_ISP1)
		overflow_err = BUF_ERR_OVERFLOW1;
	else
		return rc;

	spin_lock_irqsave(&irqlock, flags);
	if (dev && (dev->error & overflow_err))
		rc = update_dma_buffer(dev);
	spin_unlock_irqrestore(&irqlock, flags);
	return rc;
}

irqreturn_t dwe_hw_isr(int irq, void *data)
{
	struct dwe_ic_dev *dev = (struct dwe_ic_dev *)data;
	u32 status;
	u32 clr;
	unsigned long flags;

	if (!dev)
		return IRQ_HANDLED;

	status = dwe_read_reg(dev, INTERRUPT_STATUS);
	if (status & INT_FRAME_DONE) {
		clr = (status & 0xFF) << 24;
		dwe_write_reg(dev, INTERRUPT_STATUS, clr);
		spin_lock_irqsave(&irqlock, flags);
		if (dev->dst) {
			viv_buffer_ready(RESV_STREAMID_DWE, dev->dst);
			dev->dst = NULL;
		}
		update_dma_buffer(dev);
		spin_unlock_irqrestore(&irqlock, flags);
	}
	return IRQ_HANDLED;
}

#endif
