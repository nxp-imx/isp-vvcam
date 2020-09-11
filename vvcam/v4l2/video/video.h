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
#ifndef _ISP_VIDEO_H_
#define _ISP_VIDEO_H_

#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/videodev2.h>
#include <linux/dma-buf.h>
#include <linux/dma-mapping.h>
#include <media/videobuf2-dma-contig.h>
#include <media/videobuf2-memops.h>
#include <media/videobuf2-v4l2.h>
#include <media/videobuf2-core.h>
#include <linux/version.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-fh.h>

#include "viv_video_kevent.h"

struct viv_custom_ctrls {
	struct v4l2_ctrl_handler handler;
	struct v4l2_ctrl *request;
	uint64_t buf_pa;
	void __iomem *buf_va;
	struct completion wait;
};

struct viv_video_fmt {
	int fourcc;
	int depth;
	int bpp;
};

struct viv_video_device {
	struct file *filep;
	struct video_device *video;
	struct v4l2_device *v4l2_dev;
	struct viv_custom_ctrls ctrls;
	struct vvcam_constant_modeinfo modeinfo[20];
	int modeinfocount;
	struct viv_video_fmt formats[20];
	int formatscount;
	int id;
	struct viv_caps_mode_s caps_mode;
	int event_result;
	struct viv_caps_supports caps_supports;

};

typedef int (*on_notify_func)(int streamid, void *data);

struct viv_video_file {
	struct v4l2_fh vfh;
	int streamid;
	/*  0-free
	    1-ready to streaming
	    2-streaming
	    -1-closed
	*/
	int state;
	int sequence;
	bool req;
	bool capsqueried;
	struct vb2_queue queue;
	struct mutex event_mutex;
	struct mutex buffer_mutex;
	struct completion wait;
	struct v4l2_format fmt;
	struct list_head entry;
	struct v4l2_fract timeperframe;
	struct viv_video_device *vdev;
#ifdef ENABLE_IRQ
	int is_endpoint : 1;
	spinlock_t irqlock;
	struct list_head dmaqueueidle;
	struct list_head dmaqueuedone;
	struct {
		spinlock_t lock;
		on_notify_func on_notify;
		void *data;
	} idle, done;
#endif
	struct list_head extdmaqueue;
};

#define priv_to_handle(priv)	container_of(priv, struct viv_video_file, vfh)
#define queue_to_handle(__q)	container_of(__q, struct viv_video_file, queue)

#ifdef CONFIG_VIDEOBUF2_DMA_CONTIG
struct vb2_dc_buf {
	struct vb2_v4l2_buffer vb;
	struct list_head irqlist;
	dma_addr_t dma;
	int user;
	int group;
};

# ifdef ENABLE_IRQ
struct vb2_dc_buf *viv_dequeue_buffer(int streamid);
struct vb2_dc_buf *viv_dequeue_sg_buffer(int streamid, struct vb2_dc_buf *src);
void viv_queue_buffer(int streamid, struct vb2_dc_buf *buf);
void viv_buffer_ready(int streamid, struct vb2_dc_buf *buf);
struct vb2_dc_buf *viv_get_source(int streamid);
void viv_loop_source(int streamid, struct vb2_dc_buf *buf);
void viv_register_src_notifier(int streamid,
				on_notify_func notifier, void *data);
void viv_register_dst_notifier(int streamid,
				on_notify_func notifier, void *data);
void viv_post_irq_event(int streamid, int type, void *data, size_t size);
void viv_init_bufq(int streamid, int *is_endpoint);
void viv_deinit_bufq(int streamid, int *is_endpoint);
# endif
#endif

#endif /* _ISP_VIDEO_H_ */
