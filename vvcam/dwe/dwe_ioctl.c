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
#include "dwe_ioctl.h"
#include "dwe_regs.h"
#ifndef __KERNEL__
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#ifdef USE_V4L2
#include <linux/videodev2.h>
#endif
#endif

#ifndef __KERNEL__
#ifdef HAL_CMODEL
#define DEWARP_REGISTER_OFFSET  0
#else
#define DEWARP_REGISTER_OFFSET  0x380000
#endif

#define DEWARP_REGISTER_CTL	 0x308250

pReadBar g_read_func;
pWriteBar g_write_func;

void dwe_set_func(pReadBar read_func, pWriteBar write_func)
{
	g_read_func = read_func;
	g_write_func = write_func;
}

void dwe_write_reg(struct dwe_ic_dev *dev, u32 offset, u32 val)
{
	g_write_func(DEWARP_REGISTER_OFFSET + offset, val);
}

u32 dwe_read_reg(struct dwe_ic_dev *dev, u32 offset)
{
	u32 data;

	g_read_func(DEWARP_REGISTER_OFFSET + offset, &data);
	return data;
}

void dwe_write_extreg(u32 offset, u32 val)
{
	g_write_func(DEWARP_REGISTER_CTL + offset, val);
}

u32 dwe_read_extreg(u32 offset)
{
	u32 data;

	g_read_func(DEWARP_REGISTER_CTL + offset, &data);
	return data;
}

long dwe_copy_data(void *dst, void *src, int size)
{
	if (dst != src)
		memcpy(dst, src, size);
	return 0;
}
#else
void dwe_write_reg(struct dwe_ic_dev *dev, u32 offset, u32 val)
{
	__raw_writel(val, dev->base + offset);
}

u32 dwe_read_reg(struct dwe_ic_dev *dev, u32 offset)
{
	return __raw_readl(dev->base + offset);
}
#endif

int dwe_reset(struct dwe_ic_dev *dev)
{
	pr_debug("enter %s\n", __func__);
#ifdef DWE_REG_RESET
	__raw_writel(0, dev->reset);
	__raw_writel(1, dev->reset);
#endif
	dwe_write_reg(dev, DEWARP_CTRL, 0x0c);
	return 0;
}

int dwe_s_params(struct dwe_ic_dev *dev, struct dwe_hw_info *info)
{
	u32 reg = 0;
	u32 reg_y_rbuff_size = ALIGN_UP(info->dst_stride * info->dst_h, 16);
	u32 vUp = (info->split_v1 & ~0x0F) | 0x0C;
	u32 vDown = (info->split_v2 & ~0x0F) | 0x0C;
	u32 hLine = (info->split_h & ~0x0F) | 0x0C;

	pr_debug("enter %s\n", __func__);

	dwe_write_reg(dev, MAP_LUT_SIZE,
		      ((info->map_w & 0x7ff) | ((info->map_h & 0x7ff) << 16)));
	dwe_write_reg(dev, SRC_IMG_SIZE,
		      ((info->src_w & 0x1fff) |
		       ((info->src_h & 0x1fff) << 16)));
	dwe_write_reg(dev, SRC_IMG_STRIDE, info->src_stride);

	dwe_write_reg(dev, DST_IMG_SIZE,
		      ((info->dst_w & 0x1FFF) |
		       ((info->dst_h & 0x1FFF) << 16)));
	dwe_write_reg(dev, DST_IMG_STRIDE, info->dst_stride);
	dwe_write_reg(dev, DST_IMG_Y_SIZE1, reg_y_rbuff_size >> 4);
	dwe_write_reg(dev, DST_IMG_UV_SIZE1, info->dst_size_uv >> 4);
	dwe_write_reg(dev, VERTICAL_SPLIT_LINE,
		      (vUp & 0x1fff) | ((vDown & 0x1fff) << 16));
	dwe_write_reg(dev, HORIZON_SPLIT_LINE, (hLine & 0x1fff));

	reg = 0x4C800001;
	reg |= ((info->split_line & 0x1) << 11);
	reg |= ((info->in_format & 0x3) << 4);
	reg |= ((info->out_format & 0x3) << 6);
	reg |=
	    ((info->src_auto_shadow & 0x1) << 8) |
	    ((info->dst_auto_shadow & 0x1) << 10);
	reg |= ((info->hand_shake & 0x1) << 9);
	dwe_write_reg(dev, DEWARP_CTRL, reg);

	dwe_write_reg(dev, BOUNDRY_PIXEL,
		      (((info->boundary_y & 0xff) << 16) |
		       ((info->boundary_u & 0xff)
			<< 8) | (info->boundary_v & 0xff)));
	dwe_write_reg(dev, SCALE_FACTOR, info->scale_factor);
	dwe_write_reg(dev, ROI_START,
		      ((info->roi_x & 0x1fff) |
		       ((info->roi_y & 0x1fff) << 16)));
	return 0;
}

int dwe_enable_bus(struct dwe_ic_dev *dev, bool enable)
{
	u32 reg = dwe_read_reg(dev, BUS_CTRL);

	/* pr_debug("enter %s\n", __func__); */

	if (enable) {
		dwe_write_reg(dev, BUS_CTRL, reg | DEWRAP_BUS_CTRL_ENABLE_MASK);
	} else {
		dwe_write_reg(dev, BUS_CTRL,
			      reg & ~DEWRAP_BUS_CTRL_ENABLE_MASK);
	}

	return 0;
}

int dwe_disable_irq(struct dwe_ic_dev *dev)
{
	/* pr_debug("enter %s\n", __func__); */
	dwe_write_reg(dev, INTERRUPT_STATUS, INT_CLR_MASK);
	return 0;
}

int dwe_clear_irq(struct dwe_ic_dev *dev)
{
	u32 reg_dewarp_ctrl;

	/* pr_debug("enter %s\n", __func__); */
	reg_dewarp_ctrl = dwe_read_reg(dev, DEWARP_CTRL);
	dwe_write_reg(dev, DEWARP_CTRL, reg_dewarp_ctrl | 2);
	dwe_write_reg(dev, DEWARP_CTRL, reg_dewarp_ctrl);
	dwe_write_reg(dev, INTERRUPT_STATUS, INT_CLR_MASK | INT_MSK_STATUS_MASK);

	return 0;
}

int dwe_read_irq(struct dwe_ic_dev *dev, u32 *ret)
{
	u32 irq = 0;

	irq = dwe_read_reg(dev, INTERRUPT_STATUS);
	*ret = irq;

	return 0;
}

int dwe_start(struct dwe_ic_dev *dev)
{
#if defined(__KERNEL__) && defined(ENABLE_IRQ)
	dev->src = NULL;
	dev->dst = NULL;
	dev->error = BUF_ERR_UNDERFLOW;
#endif
	return 0;
}

int dwe_stop(struct dwe_ic_dev *dev)
{
	dwe_enable_bus(dev, 0);
	dwe_disable_irq(dev);
	return 0;
}

int dwe_start_dma_read(struct dwe_ic_dev *dev,
				struct dwe_hw_info *info, u64 addr)
{
#ifdef DWE_REG_RESET
	u32 regStart = 1 << 4;
	u32 reg;
#endif
	u32 reg_dst_y_base = (u32)addr;
	u32 reg_y_rbuff_size = ALIGN_UP(info->src_stride * info->src_h, 16);
	u32 reg_dst_uv_base = reg_dst_y_base + reg_y_rbuff_size;

	/* pr_debug("enter %s\n", __func__); */

	dwe_write_reg(dev, SRC_IMG_Y_BASE, (reg_dst_y_base) >> 4);
	dwe_write_reg(dev, SRC_IMG_UV_BASE, (reg_dst_uv_base) >> 4);

#ifdef DWE_REG_RESET
	reg = __raw_readl(dev->reset);
	__raw_writel(reg | regStart, dev->reset);
	__raw_writel(reg & ~regStart, dev->reset);
#endif

	return 0;
}

int dwe_set_buffer(struct dwe_ic_dev *dev, struct dwe_hw_info *info, u64 addr)
{
	u32 reg_dst_y_base = (u32) addr;
	u32 reg_y_rbuff_size = ALIGN_UP(info->dst_stride * info->dst_h, 16);
	u32 reg_dst_uv_base = reg_dst_y_base + reg_y_rbuff_size;

	/* pr_debug("enter %s\n", __func__); */
	dwe_write_reg(dev, DST_IMG_Y_BASE, (reg_dst_y_base) >> 4);
	dwe_write_reg(dev, DST_IMG_UV_BASE, (reg_dst_uv_base) >> 4);

	return 0;
}

int dwe_set_lut(struct dwe_ic_dev *dev, u64 addr)
{
	dwe_write_reg(dev, MAP_LUT_ADDR, ((u32) addr) >> 4);
	return 0;
}

int dwe_ioc_qcap(struct dwe_ic_dev *dev, void *args)
{
#ifdef __KERNEL__
	struct v4l2_capability *cap = (struct v4l2_capability *)args;

	strcpy((char *)cap->driver, "viv_dewarp100");
#endif
	return 0;
}

long dwe_priv_ioctl(struct dwe_ic_dev *dev, unsigned int cmd, void *args)
{
	int ret = -1;

	switch (cmd) {
	case DWEIOC_RESET:
		ret = dwe_reset(dev);
		break;
	case DWEIOC_S_PARAMS:
		viv_check_retval(copy_from_user
				 (&dev->info[0][0], args, sizeof(dev->info[0][0])));
		ret = dwe_s_params(dev, &dev->info[0][0]);
		break;
#ifndef ENABLE_IRQ
	case DWEIOC_ENABLE_BUS:
		ret = dwe_enable_bus(dev, 1);
		break;
	case DWEIOC_DISABLE_BUS:
		ret = dwe_enable_bus(dev, 0);
		break;
	case DWEIOC_DISABLE_IRQ:
		ret = dwe_disable_irq(dev);
		break;
	case DWEIOC_CLEAR_IRQ:
		ret = dwe_clear_irq(dev);
		break;
#endif
	case DWEIOC_READ_IRQ: {
		u32 irq = 0;
#ifndef ENABLE_IRQ
		ret = dwe_read_irq(dev, &irq);
#endif
		viv_check_retval(copy_to_user(args, &irq, sizeof(irq)));
		break;
	}
	case DWEIOC_START:
		ret = dwe_start(dev);
		break;
	case DWEIOC_STOP:
		ret = dwe_stop(dev);
		break;
	case DWEIOC_START_DMA_READ: {
#ifndef ENABLE_IRQ
		u64 addr;

		viv_check_retval(copy_from_user(&addr, args, sizeof(addr)));
		ret = dwe_start_dma_read(dev, &dev->info[0][0], addr);
#endif
		break;
	}
	case DWEIOC_SET_BUFFER: {
#ifndef ENABLE_IRQ
		u64 addr;

		viv_check_retval(copy_from_user(&addr, args, sizeof(addr)));
		ret = dwe_set_buffer(dev, &dev->info[0][0], addr);
#endif
		break;
	}
	case DWEIOC_SET_LUT: {
		struct lut_info info;

		viv_check_retval(copy_from_user(&info, args, sizeof(info)));
#ifndef ENABLE_IRQ
		ret = dwe_set_lut(dev, info.addr);
#endif
		break;
	}
#ifdef __KERNEL__
	case VIDIOC_QUERYCAP:
		ret = dwe_ioc_qcap(dev, args);
		break;
#endif
	default:
		pr_err("unsupported dwe command %d", cmd);
		break;
	}

	return ret;
}
