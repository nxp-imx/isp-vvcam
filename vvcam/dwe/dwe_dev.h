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
#ifndef _DWE_DEV_H
#define _DWE_DEV_H

#include "vvdefs.h"

#define MAX_DWE_NUM (2)
#define MAX_CFG_NUM (2)

struct dwe_hw_info {
	u32 split_line;
	u32 scale_factor;
	u32 in_format;
	u32 out_format;
	u32 hand_shake;
	u32 roi_x, roi_y;
	u32 boundary_y, boundary_u, boundary_v;
	u32 map_w, map_h;
	u32 src_auto_shadow, dst_auto_shadow;
	u32 src_w, src_stride, src_h;
	u32 dst_w, dst_stride, dst_h, dst_size_uv;
	u32 split_h, split_v1, split_v2;
};

enum BUF_ERR_TYPE {
	BUF_ERR_UNDERFLOW = 1,
	BUF_ERR_OVERFLOW0 = 1 << 1,
	BUF_ERR_OVERFLOW1 = 1 << 2,
	BUF_ERR_NO_DIST_MAP0 = 1 << 2,
	BUF_ERR_NO_DIST_MAP1 = 1 << 3,
	BUF_ERR_WRONGSTATE = 1 << 4,
};

enum HARDWARE_STATUS {
	HARDWARE_IDLE = 0,
	HARDWARE_BUSY,
};

struct dwe_ic_dev {
	struct dwe_hw_info info[MAX_DWE_NUM][MAX_CFG_NUM];
	int which[MAX_DWE_NUM];
	void __iomem *base;
	void __iomem *reset;
#if defined(__KERNEL__) && defined(ENABLE_IRQ)
	struct vvbuf_ctx *sink_bctx;
	struct vvbuf_ctx *src_bctx[MAX_DWE_NUM];
	dma_addr_t dist_map[MAX_DWE_NUM][MAX_CFG_NUM];
	dma_addr_t curmap[MAX_DWE_NUM][MAX_CFG_NUM];
	int hardware_status;
	int *state[MAX_DWE_NUM];
	int index;
	struct vb2_dc_buf *src;
	struct vb2_dc_buf *dst;
	spinlock_t irqlock;
	u32 error;
	int (*get_index)(struct dwe_ic_dev *dev, struct vb2_dc_buf *buf);
	struct tasklet_struct tasklet;
#endif

};

void dwe_write_reg(struct dwe_ic_dev *dev, u32 offset, u32 val);
u32 dwe_read_reg(struct dwe_ic_dev *dev, u32 offset);

#endif /* _DWE_DEV_H */
