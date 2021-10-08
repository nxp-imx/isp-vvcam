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
#ifdef __KERNEL__
# include "dwe_driver.h"
# include "video/vvbuf.h"
#endif
#include "dwe_ioctl.h"
#include "dwe_regs.h"

#if defined(__KERNEL__) && defined(ENABLE_IRQ)

static int update_dma_buffer(struct dwe_ic_dev *dev)
{
	int which;
	u32 dewarp_ctrl;

	if (dev->hardware_status == HARDWARE_BUSY) {
		return 0;
	}

	if (dev->src) {
		vvbuf_ready(dev->sink_bctx, dev->src->pad, dev->src);
		dev->src = NULL;
	}

	if (dev->dst) {
		vvbuf_push_buf(dev->src_bctx[dev->index], dev->dst);
		dev->dst = NULL;
	}
	if (dwe_read_reg(dev, INTERRUPT_STATUS) & INT_FRAME_BUSY)
		return -1;

	dwe_enable_bus(dev, 0);

	do {
		dev->src = vvbuf_pull_buf(dev->sink_bctx);
		if (dev->src == NULL){
			dev->error = BUF_ERR_UNDERFLOW;
			return dev->error;
		}
		dev->index = dev->get_index(dev, dev->src);
		if ((dev->index < 0) || (dev->index > MAX_DWE_NUM)) {
			vvbuf_ready(dev->sink_bctx, dev->src->pad, dev->src);
			dev->src = NULL;
			dev->error = BUF_ERR_WRONGSTATE;
			continue;
		}

		if ((*dev->state[dev->index] & STATE_DRIVER_STARTED) !=
		STATE_DRIVER_STARTED) {
			vvbuf_ready(dev->sink_bctx, dev->src->pad, dev->src);
			dev->src = NULL;
			dev->error = BUF_ERR_WRONGSTATE;
			continue;
		}

		if (dev->dist_map[dev->index] == NULL) {
			vvbuf_ready(dev->sink_bctx, dev->src->pad, dev->src);
			dev->src = NULL;
			dev->error = BUF_ERR_WRONGSTATE;
			continue;

		}

		dev->dst = vvbuf_pull_buf(dev->src_bctx[dev->index]);
		if (!dev->dst){
			vvbuf_ready(dev->sink_bctx, dev->src->pad, dev->src);
			dev->src = NULL;
			dev->error = (BUF_ERR_OVERFLOW0 << dev->index);
			continue;
		}
	} while(dev->dst == NULL);

	which = dev->which[dev->index];
	dwe_s_params(dev, &dev->info[dev->index][which]);
	dwe_set_buffer(dev, &dev->info[dev->index][which], dev->dst->dma);
	dwe_set_lut(dev, dev->dist_map[dev->index][which]);
	dwe_start_dma_read(dev, &dev->info[dev->index][which], dev->src->dma);
	dewarp_ctrl = dwe_read_reg(dev, DEWARP_CTRL);
	dwe_write_reg(dev, DEWARP_CTRL, dewarp_ctrl | 2);
	dwe_write_reg(dev, DEWARP_CTRL, dewarp_ctrl);
	dwe_write_reg(dev, INTERRUPT_STATUS, INT_MSK_STATUS_MASK);
	dwe_enable_bus(dev, 1);
	dev->hardware_status = HARDWARE_BUSY;
	return 0;
}

int dwe_on_buf_update(struct dwe_ic_dev *dev)
{
	int rc = 0;
	unsigned long flags;
	spin_lock_irqsave(&dev->irqlock, flags);
	rc = update_dma_buffer(dev);
	spin_unlock_irqrestore(&dev->irqlock, flags);
	return rc;
}

void dwe_clear_interrupts(struct dwe_ic_dev *dev)
{
	u32 status;
	u32 clr;
	status = dwe_read_reg(dev, INTERRUPT_STATUS);
	clr = (status & 0xFF) << 24;
	dwe_write_reg(dev, INTERRUPT_STATUS, clr);
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
	clr = (status & 0xFF) << 24;
	dwe_write_reg(dev, INTERRUPT_STATUS, clr);

	if (dev->src) {
		vvbuf_ready(dev->sink_bctx, dev->src->pad, dev->src);
		dev->src = NULL;
	}

	if (dev->dst) {
		vvbuf_ready(dev->src_bctx[dev->index], dev->dst->pad, dev->dst);
		dev->dst = NULL;
	}

	spin_lock_irqsave(&dev->irqlock, flags);
		dev->hardware_status = HARDWARE_IDLE;
		update_dma_buffer(dev);
	spin_unlock_irqrestore(&dev->irqlock, flags);

	return IRQ_HANDLED;
}

#endif
