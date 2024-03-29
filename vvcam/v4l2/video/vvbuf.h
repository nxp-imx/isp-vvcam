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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
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
#ifndef _VVBUF_H_
#define _VVBUF_H_

#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/dma-buf.h>
#include <media/media-entity.h>
#include <media/videobuf2-v4l2.h>

#if !IS_ENABLED(CONFIG_VIDEOBUF2_DMA_CONTIG)
#error "CONFIG_VIDEOBUF2_DMA_CONTIG is not enable"
#endif

struct vb2_dc_buf {
	struct vb2_v4l2_buffer vb;
	struct media_pad *pad;
	struct list_head irqlist;
	dma_addr_t dma;
	uint64_t timestamp;
	int flags;
};

struct vvbuf_ctx;

struct vvbuf_ops {
	void (*notify)(struct vvbuf_ctx *ctx, struct vb2_dc_buf *buf);
};

struct vvbuf_ctx {
	spinlock_t irqlock;
	struct list_head dmaqueue;
	const struct vvbuf_ops *ops;
};

void vvbuf_ctx_init(struct vvbuf_ctx *ctx);
void vvbuf_ctx_deinit(struct vvbuf_ctx *ctx);
struct vb2_dc_buf *vvbuf_pull_buf(struct vvbuf_ctx *ctx);
int vvbuf_push_buf(struct vvbuf_ctx *ctx, struct vb2_dc_buf *buf);

struct vb2_dc_buf *vvbuf_try_dqbuf(struct vvbuf_ctx *ctx);
void vvbuf_try_dqbuf_done(struct vvbuf_ctx *ctx, struct vb2_dc_buf *buf);
void vvbuf_ready(struct vvbuf_ctx *ctx, struct media_pad *pad,
				struct vb2_dc_buf *buf);

#endif /* _VVBUF_H_ */
