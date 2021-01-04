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
	int no_dist_map_err, overflow_err;
	int which;
	u32 dewarp_ctrl;

	dwe_enable_bus(dev, 0);
	if (dev->dst)
		return -EBUSY;
	if (dev->src) {
		vvbuf_ready(dev->sink_bctx, dev->src->pad, dev->src);
		dev->src = NULL;
	}
	dev->src = vvbuf_try_dqbuf(dev->sink_bctx);
	if (!dev->src) {
		dev->error |= BUF_ERR_UNDERFLOW;
		return -ENOMEM;
	}
	dev->error &= ~BUF_ERR_UNDERFLOW;
	if (!dev->get_index) {
		dev->error |= BUF_ERR_WRONGSTATE;
		dev->src = NULL;
		return -ENXIO;
	}
	dev->index = dev->get_index(dev, dev->src);
	if (dev->index < 0 || dev->index >= MAX_DWE_NUM) {
		dev->error |= BUF_ERR_WRONGSTATE;
		dev->src = NULL;
		return -ENXIO;
	}
	dev->error &= ~BUF_ERR_WRONGSTATE;
	if (!(*dev->state[dev->index] & STATE_DRIVER_STARTED)) {
		vvbuf_try_dqbuf_done(dev->sink_bctx, dev->src);
		vvbuf_ready(dev->sink_bctx, dev->src->pad, dev->src);
		dev->error |= BUF_ERR_UNDERFLOW;
		dev->src = NULL;
		return 0;
	}
	no_dist_map_err = BUF_ERR_NO_DIST_MAP0 << dev->index;
	if (!dev->dist_map[dev->index]) {
		dev->error |= no_dist_map_err;
		dev->src = NULL;
		return -ENOMEM;
	}
	dev->error &= ~no_dist_map_err;
	overflow_err = BUF_ERR_OVERFLOW0 << dev->index;
	dev->dst = vvbuf_try_dqbuf(dev->src_bctx[dev->index]);
	if (!dev->dst) {
		dev->error |= overflow_err;
		dev->src = NULL;
		return -ENOMEM;
	}
	dev->error &= ~overflow_err;
	vvbuf_try_dqbuf_done(dev->sink_bctx, dev->src);
	vvbuf_try_dqbuf_done(dev->src_bctx[dev->index], dev->dst);
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
	return 0;
}

int dwe_on_buf_update(struct dwe_ic_dev *dev)
{
	int rc = 0;
	unsigned long flags;

	if (dev) {
		spin_lock_irqsave(&dev->irqlock, flags);
		if (dev->error)
			rc = update_dma_buffer(dev);
		spin_unlock_irqrestore(&dev->irqlock, flags);
	}
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
		spin_lock_irqsave(&dev->irqlock, flags);
		if (dev->dst) {
			vvbuf_ready(dev->src_bctx[dev->index],
					dev->dst->pad, dev->dst);
			dev->dst = NULL;
		}
		update_dma_buffer(dev);
		spin_unlock_irqrestore(&dev->irqlock, flags);
	}
	return IRQ_HANDLED;
}

#endif
