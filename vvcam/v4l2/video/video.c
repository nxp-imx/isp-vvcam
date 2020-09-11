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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
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
#ifdef CONFIG_VIDEOBUF2_DMA_CONTIG
#include <linux/dma-direct.h>
#endif
#include <linux/module.h>
#include <linux/slab.h>
#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-ioctl.h>

#include <linux/platform_device.h>

#include <linux/kthread.h>
#include <linux/freezer.h>
#include <asm/uaccess.h>

#include "video.h"
#include "vvctrl.h"
#include "vvdefs.h"
#include "vvsensor.h"

#define DEF_PLANE_NO (0)
#define VIDEO_NODE_NUM 2

static struct viv_video_device *vvdev[VIDEO_NODE_NUM];
LIST_HEAD(file_list_head);
static DEFINE_SPINLOCK(file_list_lock);

#ifdef CONFIG_VIDEOBUF2_DMA_CONTIG
struct ext_dma_buf {
	dma_addr_t addr;
	void *vaddr;
	size_t size;
	struct list_head entry;
};
#endif

static struct viv_video_fmt formats[] = {
	{
	 .fourcc = V4L2_PIX_FMT_YUYV,
	 .depth = 16,
	 .bpp = 2,
	 },
	{
	 .fourcc = V4L2_PIX_FMT_NV12,
	 .depth = 12,
	 .bpp = 1,
	 },
	{
	 .fourcc = V4L2_PIX_FMT_NV16,
	 .depth = 16,
	 .bpp = 1,
	 },
};

static int bayer_pattern_to_format(unsigned int bayer_pattern,
		unsigned int bit_width, struct viv_video_fmt *fmt)
{
	int ret = 0;
	if (bayer_pattern == BAYER_BGGR)
		if (bit_width == 8) {
			fmt->fourcc = V4L2_PIX_FMT_SBGGR8;
			fmt->depth = 8;
			fmt->bpp = 1;
		} else if (bit_width == 10) {
			fmt->fourcc = V4L2_PIX_FMT_SBGGR10;
			fmt->depth = 16;
			fmt->bpp = 2;
		} else if (bit_width == 12) {
			fmt->fourcc = V4L2_PIX_FMT_SBGGR12;
			fmt->depth = 16;
			fmt->bpp = 2;
		} else
			ret = -EPERM;
	else if (bayer_pattern == BAYER_GBRG)
		if (bit_width == 8) {
			fmt->fourcc = V4L2_PIX_FMT_SGBRG8;
			fmt->depth = 8;
			fmt->bpp = 1;
		} else if (bit_width == 10) {
			fmt->fourcc = V4L2_PIX_FMT_SGBRG10;
			fmt->depth = 16;
			fmt->bpp = 2;
		} else if (bit_width == 12) {
			fmt->fourcc = V4L2_PIX_FMT_SGBRG12;
			fmt->depth = 16;
			fmt->bpp = 2;
		} else
			ret = -EPERM;
	else if (bayer_pattern == BAYER_GRBG)
		if (bit_width == 8) {
			fmt->fourcc = V4L2_PIX_FMT_SGRBG8;
			fmt->depth = 8;
			fmt->bpp = 1;
		} else if (bit_width == 10) {
			fmt->fourcc = V4L2_PIX_FMT_SGRBG10;
			fmt->depth = 16;
			fmt->bpp = 2;
		} else if (bit_width == 12) {
			fmt->fourcc = V4L2_PIX_FMT_SGRBG12;
			fmt->depth = 16;
			fmt->bpp = 2;
		} else
			ret = -EPERM;
	else if (bayer_pattern == BAYER_RGGB)
		if (bit_width == 8) {
			fmt->fourcc = V4L2_PIX_FMT_SRGGB8;
			fmt->depth = 8;
			fmt->bpp = 1;
		} else if (bit_width == 10) {
			fmt->fourcc = V4L2_PIX_FMT_SRGGB10;
			fmt->depth = 16;
			fmt->bpp = 2;
		} else if (bit_width == 12) {
			fmt->fourcc = V4L2_PIX_FMT_SRGGB12;
			fmt->depth = 16;
			fmt->bpp = 2;
		} else
			ret = -EPERM;
	else
		ret = -EPERM;
	return ret;
}

static int viv_post_event(struct v4l2_event *event, void *fh, bool sync)
{
	struct viv_video_file *handle = priv_to_handle(fh);

	if (sync)
		reinit_completion(&handle->wait);

	mutex_lock(&handle->event_mutex);
	v4l2_event_queue(handle->vdev->video, event);
	mutex_unlock(&handle->event_mutex);

	if (sync) {
		if (wait_for_completion_timeout(&handle->wait, msecs_to_jiffies(
				VIV_VIDEO_EVENT_TIMOUT_MS)) == 0)
			return -ETIMEDOUT;
	}
	return 0;
}

static int viv_post_simple_event(int id, int streamid, void *fh, bool sync)
{
	struct v4l2_event event;
	struct viv_video_event *v_event;

	v_event = (struct viv_video_event *)&event.u.data[0];
	v_event->stream_id = streamid;
	v_event->file = fh;
	v_event->sync = sync;
	event.type = VIV_VIDEO_EVENT_TYPE;
	event.id = id;
	return viv_post_event(&event, fh, sync);
}

static int viv_post_control_event(int streamid, void *fh,
				  struct viv_control_event *control_event)
{
	struct v4l2_event event;
	struct viv_video_event *v_event;

	v_event = (struct viv_video_event *)&event.u.data[0];
	v_event->stream_id = streamid;
	v_event->file = fh;
	v_event->sync = true;
	v_event->addr = control_event->request;
	v_event->response = control_event->response;
	v_event->buf_index = control_event->id;
	event.type = VIV_VIDEO_EVENT_TYPE;
	event.id = VIV_VIDEO_EVENT_PASS_JSON;
	return viv_post_event(&event, fh, true);
}

#ifdef ENABLE_IRQ
static inline struct viv_video_file *get_fh(int streamid)
{
	struct viv_video_file *ph, *file = NULL;
	unsigned long flags;

	spin_lock_irqsave(&file_list_lock, flags);
	list_for_each_entry(ph, &file_list_head, entry) {
		if (ph->streamid == streamid) {
			file = ph;
			break;
		}
	}
	spin_unlock_irqrestore(&file_list_lock, flags);
	return file;
}

struct vb2_dc_buf *viv_dequeue_buffer(int streamid)
{
	struct viv_video_file *file = get_fh(streamid);
	struct vb2_dc_buf *buf;
	unsigned long flags;

	if (!file || file->state != 2)
		return NULL;

	spin_lock_irqsave(&file->irqlock, flags);
	if (list_empty(&file->dmaqueueidle)) {
		spin_unlock_irqrestore(&file->irqlock, flags);
		return NULL;
	}

	buf = list_first_entry(&file->dmaqueueidle, struct vb2_dc_buf, irqlist);
	list_del(&buf->irqlist);
	spin_unlock_irqrestore(&file->irqlock, flags);
	return buf;
}
EXPORT_SYMBOL(viv_dequeue_buffer);

void viv_init_bufq(int streamid, int *is_endpoint)
{
	struct viv_video_file *ph, *file = get_fh(streamid), *req_fh = NULL;
	struct vb2_dc_buf *buf;
	struct vb2_v4l2_buffer *vbuf;
	struct vb2_buffer *vb;
	unsigned long flags;

	if (!file)
		return;

	if (is_endpoint)
		*is_endpoint = file->is_endpoint & 1;

	spin_lock_irqsave(&file_list_lock, flags);
	list_for_each_entry(ph, &file_list_head, entry) {
		if (ph->req) {
			req_fh = ph;
			break;
		}
	}
	spin_unlock_irqrestore(&file_list_lock, flags);

	if (!req_fh)
		return;

	spin_lock_irqsave(&file->irqlock, flags);
	if ((file->is_endpoint & 1) && list_empty(&file->dmaqueueidle)) {
		spin_unlock_irqrestore(&file->irqlock, flags);

		mutex_lock(&req_fh->buffer_mutex);
		list_for_each_entry(vb, &req_fh->queue.queued_list, queued_entry) {
			if (!vb)
				continue;
			vbuf = container_of(vb, struct vb2_v4l2_buffer, vb2_buf);
			buf = container_of(vbuf, struct vb2_dc_buf, vb);
			viv_queue_buffer(streamid, buf);
		}
		mutex_unlock(&req_fh->buffer_mutex);
	} else
		spin_unlock_irqrestore(&file->irqlock, flags);
}
EXPORT_SYMBOL(viv_init_bufq);

void viv_deinit_bufq(int streamid, int *is_endpoint)
{
	struct viv_video_file *file = get_fh(streamid);

	if (!file)
		return;

	if (is_endpoint)
		*is_endpoint = file->is_endpoint & 1;
}
EXPORT_SYMBOL(viv_deinit_bufq);

struct vb2_dc_buf *viv_dequeue_sg_buffer(int streamid, struct vb2_dc_buf *src)
{
	struct viv_video_file *file = get_fh(streamid);
	struct vb2_dc_buf *buf;
	unsigned long flags;

	if (!src || !file || file->state != 2)
		return NULL;

	spin_lock_irqsave(&file->irqlock, flags);
	if (list_empty(&file->dmaqueueidle)) {
		spin_unlock_irqrestore(&file->irqlock, flags);
		return NULL;
	}
	buf = list_first_entry(&file->dmaqueueidle, struct vb2_dc_buf, irqlist);
	list_del(&buf->irqlist);
	spin_unlock_irqrestore(&file->irqlock, flags);
	return buf;
}
EXPORT_SYMBOL(viv_dequeue_sg_buffer);

void viv_queue_buffer(int streamid, struct vb2_dc_buf *buf)
{
	struct viv_video_file *file = get_fh(streamid);
	unsigned long flags;

	if (!file || file->state <= 0)
		return;

	spin_lock_irqsave(&file->irqlock, flags);
	list_add_tail(&buf->irqlist, &file->dmaqueueidle);
	spin_unlock_irqrestore(&file->irqlock, flags);
}
EXPORT_SYMBOL(viv_queue_buffer);

void viv_buffer_ready(int streamid, struct vb2_dc_buf *buf)
{
	struct viv_video_file *file, *fh;
	int targetid;
	unsigned long flags;
	u64 cur_ts, interval;
	u32 fps;
	static u64 frame_count, duration, last_ts;

	switch (streamid) {
	case RESV_STREAMID_ISP0:
	case RESV_STREAMID_ISP1:
		targetid = RESV_STREAMID_ISP0;
		break;
	case RESV_STREAMID_DWE:
		targetid = RESV_STREAMID_DWE;
		break;
	default:
		pr_err("invalid streamid (%d) received.\n", streamid);
		return;
	}

	file = get_fh(targetid);
	if (!file || file->state <= 0)
		return;

	if (!file->is_endpoint) {
		spin_lock_irqsave(&file->irqlock, flags);
		list_add_tail(&buf->irqlist, &file->dmaqueuedone);
		spin_unlock_irqrestore(&file->irqlock, flags);

		spin_lock_irqsave(&file->done.lock, flags);
		if (file->done.on_notify)
			file->done.on_notify(streamid, file->done.data);
		spin_unlock_irqrestore(&file->done.lock, flags);
	} else {
		fh = container_of(buf->vb.vb2_buf.vb2_queue,
				struct viv_video_file, queue);
		buf->vb.vb2_buf.planes[DEF_PLANE_NO].bytesused =
				fh->fmt.fmt.pix.sizeimage;
		cur_ts = ktime_get_ns();
#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 0, 0)
		buf->vb.vb2_buf.timestamp = cur_ts;
#endif
		vb2_buffer_done(&buf->vb.vb2_buf, VB2_BUF_STATE_DONE);

		/* print fps info for debugging purpose */
		interval = (cur_ts - last_ts) / 1000000; /*ms*/
		if (interval > 1000/*ms*/ || duration > 5 * 60 * 1000/*ms*/) {
			frame_count = 0;
			duration = 0;
		} else if (interval > 0) {
			frame_count++;
			duration += interval;
			fps = frame_count * 100000 / duration;
			if (frame_count % (fps / 20)/*frames*/ == 0) {
				pr_info("###### %d.%02d fps ######\n",
						fps / 100, fps % 100);
			}
		}
		last_ts = cur_ts;
	}
}
EXPORT_SYMBOL(viv_buffer_ready);

struct vb2_dc_buf *viv_get_source(int streamid)
{
	struct viv_video_file *file;
	struct vb2_dc_buf *buf;
	int targetid;
	unsigned long flags;

	switch (streamid) {
	case RESV_STREAMID_DWE:
		targetid = RESV_STREAMID_ISP0;
		break;
	default:
		pr_err("invalid streamid (%d) received.\n", streamid);
		return NULL;
	}

	file = get_fh(targetid);
	if (!file || file->state != 2)
		return NULL;

	spin_lock_irqsave(&file->irqlock, flags);
	if (list_empty(&file->dmaqueuedone)) {
		spin_unlock_irqrestore(&file->irqlock, flags);
		return NULL;
	}

	buf = list_first_entry(&file->dmaqueuedone, struct vb2_dc_buf, irqlist);
	spin_unlock_irqrestore(&file->irqlock, flags);
	return buf;
}
EXPORT_SYMBOL(viv_get_source);

void viv_loop_source(int streamid, struct vb2_dc_buf *buf)
{
	struct viv_video_file *file;
	struct vb2_dc_buf *b;
	int targetid;
	unsigned long flags;

	switch (streamid) {
	case RESV_STREAMID_DWE:
		targetid = RESV_STREAMID_ISP0;
		break;
	default:
		pr_err("invalid streamid (%d) received.\n", streamid);
		return;
	}

	file = get_fh(targetid);
	if (!file || file->state <= 0)
		return;

	spin_lock_irqsave(&file->irqlock, flags);
	if (list_empty(&file->dmaqueuedone)) {
		spin_unlock_irqrestore(&file->irqlock, flags);
		return;
	}

	b = list_first_entry(&file->dmaqueuedone, struct vb2_dc_buf, irqlist);
	if (unlikely(b != buf)) {
		spin_unlock_irqrestore(&file->irqlock, flags);
		return;
	}
	list_del(&buf->irqlist);
	list_add_tail(&buf->irqlist, &file->dmaqueueidle);
	spin_unlock_irqrestore(&file->irqlock, flags);
}
EXPORT_SYMBOL(viv_loop_source);

static inline void sync_state(int state)
{
	static int streamids[] = {
		RESV_STREAMID_ISP0,
		RESV_STREAMID_ISP1,
		RESV_STREAMID_DWE
	};
	int i = 0;
	struct viv_video_file *file;
	unsigned long flags;

	while (i < sizeof(streamids)/sizeof(streamids[0])) {
		file = get_fh(streamids[i]);
		if (file) {
			spin_lock_irqsave(&file->irqlock, flags);
			file->state = state;
			spin_unlock_irqrestore(&file->irqlock, flags);
		}
		i++;
	}
}

static void clear_queue(struct viv_video_file *file)
{
	struct vb2_dc_buf *buf;
	unsigned long flags;

	if (!file)
		return;

	spin_lock_irqsave(&file->irqlock, flags);
	while (!list_empty(&file->dmaqueueidle)) {
		buf = list_first_entry(&file->dmaqueueidle,
				struct vb2_dc_buf, irqlist);
		if (buf) {
			list_del(&buf->irqlist);
		}
	}
	while (!list_empty(&file->dmaqueuedone)) {
		buf = list_first_entry(&file->dmaqueuedone,
				struct vb2_dc_buf, irqlist);
		if (buf) {
			list_del(&buf->irqlist);
		}
	}
	spin_unlock_irqrestore(&file->irqlock, flags);
}

static inline void clear_buffer_queue(void)
{
	static int streamids[] = {
		RESV_STREAMID_ISP0,
		RESV_STREAMID_ISP1,
		RESV_STREAMID_DWE
	};
	int i = 0;

	while (i < sizeof(streamids)/sizeof(streamids[0])) {
		clear_queue(get_fh(streamids[i]));
		i++;
	}
}

void viv_register_src_notifier(int streamid,
				on_notify_func notifier, void *data)
{
	struct viv_video_file *file;
	int targetid;
	unsigned long flags;

	switch (streamid) {
	case RESV_STREAMID_DWE:
		targetid = RESV_STREAMID_ISP0;
		break;
	default:
		pr_err("invalid streamid (%d) received.\n", streamid);
		return;
	}

	file = get_fh(targetid);
	if (!file)
		return;

	spin_lock_irqsave(&file->done.lock, flags);
	file->done.on_notify = notifier;
	file->done.data = data;
	spin_unlock_irqrestore(&file->done.lock, flags);
}
EXPORT_SYMBOL(viv_register_src_notifier);

void viv_register_dst_notifier(int streamid,
				on_notify_func notifier, void *data)
{
	struct viv_video_file *file = get_fh(streamid);
	unsigned long flags;

	if (!file)
		return;

	spin_lock_irqsave(&file->idle.lock, flags);
	file->idle.on_notify = notifier;
	file->idle.data = data;
	spin_unlock_irqrestore(&file->idle.lock, flags);
}
EXPORT_SYMBOL(viv_register_dst_notifier);

void viv_post_irq_event(int streamid, int type, void *data, size_t size)
{
	struct viv_video_file *ph, *file = NULL;
	struct v4l2_event event;
	unsigned long flags;

	spin_lock_irqsave(&file_list_lock, flags);
	list_for_each_entry(ph, &file_list_head, entry) {
		if (ph->streamid == streamid) {
			file = ph;
			break;
		}
	}
	spin_unlock_irqrestore(&file_list_lock, flags);

	if (!file || !data)
		return;

	memset(&event, 0, sizeof(event));
	memcpy(event.u.data, data, min_t(size_t, size, 64));
	event.type = type;
	viv_post_event(&event, &file->vfh, false);
}
EXPORT_SYMBOL(viv_post_irq_event);
#endif

static int start_streaming(struct vb2_queue *vq, unsigned int count)
{
	struct viv_video_file *handle = queue_to_handle(vq);
	struct v4l2_fh *fh = &handle->vfh;

	pr_debug("enter %s\n", __func__);
	if (handle->streamid >= 0 && handle->state != 2) {
		handle->state = 2;
#ifdef ENABLE_IRQ
		sync_state(handle->state);
#endif
		viv_post_simple_event(VIV_VIDEO_EVENT_START_STREAM,
				      handle->streamid, fh, true);
	} else {
		pr_err("can't start streaming, device busy!\n");
		return -EBUSY;
	}
	return 0;
}

static void stop_streaming(struct vb2_queue *vq)
{
	struct viv_video_file *handle = queue_to_handle(vq);
	struct vb2_buffer *vb;

	pr_debug("enter %s\n", __func__);

	if (!handle || handle->streamid < 0 || handle->state != 2 ||
	    list_empty(&handle->entry))
		return;

	handle->state = 1;
#ifdef ENABLE_IRQ
	sync_state(handle->state);
#endif
	viv_post_simple_event(VIV_VIDEO_EVENT_STOP_STREAM, handle->streamid,
			      &handle->vfh, true);
	handle->sequence = 0;
	list_for_each_entry(vb, &vq->queued_list, queued_entry) {
		if (vb->state == VB2_BUF_STATE_DONE)
			continue;
		vb2_buffer_done(vb, VB2_BUF_STATE_DONE);
	}
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0)
static int queue_setup(struct vb2_queue *vq, const struct v4l2_format *fmt,
		       unsigned int *nbuffers, unsigned int *nplanes,
		       unsigned int sizes[], void *alloc_ctxs[])
{
	struct viv_video_file *handle = queue_to_handle(vq);
	unsigned long size = handle->fmt.fmt.pix.sizeimage;

	pr_debug("enter %s\n", __func__);
	if (0 == *nbuffers)
		*nbuffers = 1;
	while (size * *nbuffers > RESERVED_MEM_SIZE)
		(*nbuffers)--;
	sizes[0] = size;
	return 0;
}
#else
static int queue_setup(struct vb2_queue *q,
		       unsigned int *num_buffers, unsigned int *num_planes,
		       unsigned int sizes[], struct device *alloc_devs[])
{
	struct viv_video_file *handle = queue_to_handle(q);
	unsigned long size = handle->fmt.fmt.pix.sizeimage;

	pr_debug("enter %s\n", __func__);
	if (0 == *num_buffers)
		*num_buffers = 1;
	while (size * *num_buffers > RESERVED_MEM_SIZE)
		(*num_buffers)--;
	*num_planes = 1;
	sizes[0] = size;
	return 0;
}
#endif

static int buffer_init(struct vb2_buffer *vb)
{
	pr_debug("enter %s\n", __func__);
	return 0;
}

static void buffer_queue(struct vb2_buffer *vb)
{
	struct viv_video_file *handle;
#ifdef ENABLE_IRQ
	struct viv_video_file *ph, *file = NULL;
	struct video_device *video;
	struct vb2_v4l2_buffer *vbuf;
	struct vb2_dc_buf *buf;
	unsigned long flags;
#else
	struct v4l2_event event;
	struct viv_video_event *v_event;
# ifndef CONFIG_VIDEOBUF2_DMA_CONTIG
	struct vb2_dc_buf *buf;
# endif
#endif

	if (!vb)
		return;

	handle = queue_to_handle(vb->vb2_queue);
#ifdef ENABLE_IRQ
	vbuf = container_of(vb, struct vb2_v4l2_buffer, vb2_buf);
	buf = container_of(vbuf, struct vb2_dc_buf, vb);
	if (!buf)
		return;
#elif !defined(CONFIG_VIDEOBUF2_DMA_CONTIG)
	buf = vb->planes[DEF_PLANE_NO].mem_priv;
	if (!buf)
		return;
#endif

	if (!handle)
		return;

#ifdef ENABLE_IRQ
	video = handle->vfh.vdev;
	buf->dma = vb2_dma_contig_plane_dma_addr(vb, DEF_PLANE_NO);

	spin_lock_irqsave(&file_list_lock, flags);
	do {
		list_for_each_entry(ph, &file_list_head, entry) {
			if (ph->is_endpoint & 1) {
				file = ph;
				break;
			}
		}
	} while (!file && buf->user != buf->group && (buf->user = buf->group));
	spin_unlock_irqrestore(&file_list_lock, flags);

	if (file) {
		spin_lock_irqsave(&file->irqlock, flags);
		list_add_tail(&buf->irqlist, &file->dmaqueueidle);
		spin_unlock_irqrestore(&file->irqlock, flags);

		spin_lock_irqsave(&file->idle.lock, flags);
		if (file->idle.on_notify)
			file->idle.on_notify(buf->group, file->idle.data);
		spin_unlock_irqrestore(&file->idle.lock, flags);
	}
#endif

	if (handle->streamid < 0)
		return;

#ifndef ENABLE_IRQ
	/* pr_debug("buffer_queue %d", vb->index); */
	v_event = (struct viv_video_event *)&event.u.data[0];
	v_event->stream_id = handle->streamid;
	v_event->file = &handle->vfh;
#ifdef CONFIG_VIDEOBUF2_DMA_CONTIG
	v_event->addr = vb2_dma_contig_plane_dma_addr(vb, DEF_PLANE_NO);
#endif
	v_event->buf_index = vb->index;
	v_event->sync = false;
	event.type = VIV_VIDEO_EVENT_TYPE;
	event.id = VIV_VIDEO_EVENT_QBUF;
	viv_post_event(&event, &handle->vfh, false);
#endif
}

static struct vb2_ops buffer_ops = {
	.queue_setup = queue_setup,
	.buf_init = buffer_init,
	.buf_queue = buffer_queue,
	.start_streaming = start_streaming,
	.stop_streaming = stop_streaming,
};

static int query_session_caps(struct file *file)
{
	struct viv_video_device *dev = video_drvdata(file);
	struct viv_video_file *handle = priv_to_handle(file->private_data);
	struct v4l2_event event;
	struct viv_video_event *v_event;

	dev->modeinfocount = 0;
	memcpy(dev->formats, formats, sizeof(formats));
	dev->formatscount = sizeof(formats)/sizeof(formats[0]);

	v_event = (struct viv_video_event *)&event.u.data[0];
	v_event->stream_id = 0;
	v_event->file = &handle->vfh;
	v_event->sync = true;
	v_event->buf_index = dev->id;
	event.type = VIV_VIDEO_EVENT_TYPE;
	event.id = VIV_VIDEO_EVENT_QUERYCAPS;
	return viv_post_event(&event, &handle->vfh, true);
}

static int video_open(struct file *file)
{
	struct viv_video_device *dev = video_drvdata(file);
	struct viv_video_file *handle;
	unsigned long flags;
	int rc;

	pr_debug("enter %s\n", __func__);
	handle = kzalloc(sizeof(*handle), GFP_KERNEL);
	memset(handle, 0, sizeof(*handle));

	v4l2_fh_init(&handle->vfh, dev->video);
	v4l2_fh_add(&handle->vfh);

	file->private_data = &handle->vfh;
	handle->vdev = dev;
	handle->queue.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	handle->queue.drv_priv = handle;
	handle->queue.ops = &buffer_ops;
#ifdef CONFIG_VIDEOBUF2_DMA_CONTIG
	handle->queue.io_modes = VB2_MMAP | VB2_DMABUF;
	handle->queue.mem_ops = &vb2_dma_contig_memops;
#endif
	handle->queue.buf_struct_size = sizeof(struct vb2_dc_buf);
	handle->queue.timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC;
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 5, 0)
	handle->queue.dev = dev->v4l2_dev->dev;
#endif
	rc = vb2_queue_init(&handle->queue);
	if (rc) {
		pr_err("can't init vb queue\n");
		v4l2_fh_del(&handle->vfh);
		v4l2_fh_exit(&handle->vfh);
		kzfree(handle);
		return rc;
	}
	mutex_init(&handle->event_mutex);
	mutex_init(&handle->buffer_mutex);
	init_completion(&handle->wait);
	/* default id is 0, user can set another value by VIV_VIDIOC_S_STREAMID
	 * it will be pass to mc at new_stream */
	handle->streamid = 0;
	handle->fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	handle->fmt.fmt.pix.field = V4L2_FIELD_NONE;
	handle->fmt.fmt.pix.width = 1920;
	handle->fmt.fmt.pix.height = 1080;
	handle->fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	v4l_bound_align_image(&handle->fmt.fmt.pix.width, 48, 3840, 2,
			      &handle->fmt.fmt.pix.height, 32, 2160, 0, 0);
	handle->fmt.fmt.pix.bytesperline =
	    ALIGN_UP((handle->fmt.fmt.pix.width * 16) / 8, 16);
	handle->fmt.fmt.pix.sizeimage =
	    ALIGN_UP(handle->fmt.fmt.pix.height *
		     handle->fmt.fmt.pix.bytesperline, 4096);
	handle->fmt.fmt.pix.colorspace = V4L2_COLORSPACE_REC709;

#ifdef ENABLE_IRQ
	spin_lock_init(&handle->irqlock);
	INIT_LIST_HEAD(&handle->dmaqueueidle);
	INIT_LIST_HEAD(&handle->dmaqueuedone);
	spin_lock_init(&handle->idle.lock);
	spin_lock_init(&handle->done.lock);
	INIT_LIST_HEAD(&handle->extdmaqueue);
#endif

	spin_lock_irqsave(&file_list_lock, flags);
	list_add_tail(&handle->entry, &file_list_head);
	spin_unlock_irqrestore(&file_list_lock, flags);
	return 0;
}

static int video_close(struct file *file)
{
	struct viv_video_file *handle = priv_to_handle(file->private_data);
	struct vb2_buffer *vb;
	unsigned long flags;

	pr_debug("enter %s\n", __func__);
	if (handle) {
		if (handle->streamid >= 0 && handle->state == 2) {
			viv_post_simple_event(VIV_VIDEO_EVENT_STOP_STREAM,
					      handle->streamid, &handle->vfh,
					      false);
			handle->state = 1;
#ifdef ENABLE_IRQ
			sync_state(handle->state);
#endif
		}
		if (handle->streamid >= 0 && handle->state == 1)
			viv_post_simple_event(VIV_VIDEO_EVENT_DEL_STREAM,
					      handle->streamid, &handle->vfh,
					      false);

#ifdef ENABLE_IRQ
		if (handle->streamid >= 0 && handle->state == 1) {
			sync_state(-1);
			clear_buffer_queue();
		}
#endif
		handle->state = -1;
		handle->streamid = 0;
		spin_lock_irqsave(&file_list_lock, flags);
		list_del(&handle->entry);
		spin_unlock_irqrestore(&file_list_lock, flags);

		v4l2_fh_del(&handle->vfh);
		v4l2_fh_exit(&handle->vfh);

#ifdef ENABLE_IRQ
#ifdef CONFIG_VIDEOBUF2_DMA_CONTIG
		{
			struct ext_dma_buf *edb = NULL;
			while (!list_empty(&handle->extdmaqueue)) {
				edb = list_first_entry(&handle->extdmaqueue, struct ext_dma_buf, entry);
				if(edb) {
					dma_free_attrs(handle->queue.dev, edb->size,
							edb->vaddr, edb->addr,
							DMA_ATTR_WRITE_COMBINE);
					list_del(&edb->entry);
					kfree(edb);
				}
			}
		}
#endif
#endif

		while (!list_empty(&handle->queue.queued_list)) {
			vb = list_first_entry(&handle->queue.queued_list,
					      struct vb2_buffer, queued_entry);
			list_del(&vb->queued_entry);
			if (vb->state == VB2_BUF_STATE_ACTIVE)
				vb2_buffer_done(vb, VB2_BUF_STATE_ERROR);
		}
		vb2_queue_release(&handle->queue);
		mutex_destroy(&handle->event_mutex);
		mutex_destroy(&handle->buffer_mutex);
		kzfree(handle);
	}
	return 0;
}

static int subscribe_event(struct v4l2_fh *fh,
			   const struct v4l2_event_subscription *sub)
{
	struct viv_video_file *handle = priv_to_handle(fh);

	if (!handle || !sub)
		return -EINVAL;
	if (sub->type == VIV_VIDEO_EVENT_TYPE ||
		sub->type == VIV_VIDEO_ISPIRQ_TYPE)
		return v4l2_event_subscribe(fh, sub, 10, 0);
	else
		return v4l2_ctrl_subscribe_event(fh, sub);
}

static int unsubscribe_event(struct v4l2_fh *fh,
				 const struct v4l2_event_subscription *sub)
{
	struct viv_video_file *handle = priv_to_handle(fh);
	struct viv_video_file *ph;

	pr_debug("enter %s\n", __func__);

	if (!handle || !sub || !fh)
		return 0;
	if (sub->type == VIV_VIDEO_EVENT_TYPE ||
		sub->type == VIV_VIDEO_ISPIRQ_TYPE) {
		list_for_each_entry(ph, &file_list_head, entry) {
			if (ph == handle && handle->streamid < 0)
				return v4l2_event_unsubscribe(fh, sub);
		}
	} else {
		return v4l2_event_unsubscribe(fh, sub);
	}
	return 0;
}

static void viv_buffer_done(struct viv_video_file *handle, u64 addr)
{
	struct vb2_buffer *vb;

	mutex_lock(&handle->buffer_mutex);
	list_for_each_entry(vb, &handle->queue.queued_list, queued_entry) {
		if (!vb)
			continue;
#ifdef CONFIG_VIDEOBUF2_DMA_CONTIG
		if (vb2_dma_contig_plane_dma_addr(vb, DEF_PLANE_NO) == addr) {
			vb->planes[DEF_PLANE_NO].bytesused =
					handle->fmt.fmt.pix.sizeimage;
#endif
#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 0, 0)
			vb->timestamp = ktime_get_ns();
#endif
			vb2_buffer_done(vb, VB2_BUF_STATE_DONE);
			mutex_unlock(&handle->buffer_mutex);
			return;
		}
	}
	mutex_unlock(&handle->buffer_mutex);
}

static int set_caps_mode_event(struct file *file)
{
	struct viv_video_device *dev = video_drvdata(file);
	struct viv_video_file *handle = priv_to_handle(file->private_data);
	struct v4l2_event event;
	struct viv_video_event *v_event;

	v_event = (struct viv_video_event *)&event.u.data[0];
	v_event->stream_id = 0;
	v_event->file = &handle->vfh;
	v_event->sync = true;
	v_event->buf_index = dev->id;
	event.type = VIV_VIDEO_EVENT_TYPE;
	event.id = VIV_VIDEO_EVENT_SET_CAPSMODE;
	return viv_post_event(&event, &handle->vfh, true);
}

static int get_caps_suppots_event(struct file *file)
{
	struct viv_video_device *dev = video_drvdata(file);
	struct viv_video_file *handle = priv_to_handle(file->private_data);
	struct v4l2_event event;
	struct viv_video_event *v_event;

	v_event = (struct viv_video_event *)&event.u.data[0];
	v_event->stream_id = 0;
	v_event->file = &handle->vfh;
	v_event->sync = true;
	v_event->buf_index = dev->id;
	event.type = VIV_VIDEO_EVENT_TYPE;
	event.id = VIV_VIDEO_EVENT_GET_CAPS_SUPPORTS;
	return viv_post_event(&event, &handle->vfh, true);
}

static long private_ioctl(struct file *file, void *fh,
			  bool valid_prio, unsigned int cmd, void *arg)
{
	struct viv_video_file *handle;
	struct viv_video_file *ph;
	struct viv_video_event *v_event;
	struct v4l2_user_buffer *user_buffer;
	struct ext_buf_info *ext_buf;
	struct viv_control_event *control_event;
	struct vvcam_constant_modeinfo* modeinfo;
	struct viv_caps_supports *pcaps_supports;
	struct viv_video_fmt fmt;
	struct viv_video_device *dev = video_drvdata(file);
	unsigned long flags;
	int rc = 0, i;

	if (!file || !fh)
		return -EINVAL;

	handle = priv_to_handle(file->private_data);
	if (!handle || handle->state == -1) {
		pr_err("call ioctl after file closed\n");
		return -EINVAL;
	}

	switch (cmd) {
	case VIV_VIDIOC_EVENT_COMPLETE:
		v_event = (struct viv_video_event *)arg;
		if (v_event->file) {
			handle = priv_to_handle(v_event->file);
			spin_lock_irqsave(&file_list_lock, flags);
			list_for_each_entry(ph, &file_list_head, entry) {
				if (ph == handle) {
					complete(&handle->wait);
					break;
				}
			}
			spin_unlock_irqrestore(&file_list_lock, flags);
		} else {
			complete(&handle->vdev->ctrls.wait);
		}
		break;
	case VIV_VIDIOC_BUFDONE:
		/* pr_debug("priv ioctl VIV_VIDIOC_BUFDONE\n"); */
		user_buffer = (struct v4l2_user_buffer *)arg;
		if (user_buffer->file) {
			handle = priv_to_handle(user_buffer->file);
			if (handle && handle->state == 2) {
				/* handle the stream closed unexpected. */
				spin_lock_irqsave(&file_list_lock, flags);
				list_for_each_entry(ph, &file_list_head, entry) {
					if (ph == handle) {
						viv_buffer_done(handle,
								user_buffer->addr);
						break;
					}
				}
				spin_unlock_irqrestore(&file_list_lock, flags);
			}
		}
		break;
	case VIV_VIDIOC_S_STREAMID:
		pr_debug("priv ioctl VIV_VIDIOC_S_STREAMID\n");
		handle->streamid = *((int *)arg);
		break;
	case VIV_VIDIOC_S_ENDPOINT:
#ifdef ENABLE_IRQ
		pr_debug("priv ioctl VIV_VIDIOC_S_ENDPOINT\n");
		spin_lock_irqsave(&file_list_lock, flags);
			list_for_each_entry(ph, &file_list_head, entry) {
				if (ph->streamid == handle->streamid) {
					ph->is_endpoint = *((int *)arg) & 1;
					break;
				}
			}
		spin_unlock_irqrestore(&file_list_lock, flags);
#endif
		break;
	case VIV_VIDIOC_BUFFER_ALLOC: {
		struct ext_buf_info *ext_buf = (struct ext_buf_info *)arg;
#ifdef CONFIG_VIDEOBUF2_DMA_CONTIG
		struct ext_dma_buf *edb = kzalloc(sizeof(*edb), GFP_KERNEL);

		pr_debug("priv ioctl VIV_VIDIOC_BUFFER_ALLOC\n");
		if (!edb) {
			rc = -ENOMEM;
			break;
		}
		edb->vaddr = dma_alloc_attrs(handle->queue.dev,
				ext_buf->size, &ext_buf->addr,
				GFP_KERNEL, DMA_ATTR_WRITE_COMBINE);
		if (!edb->vaddr) {
			pr_err("failed to alloc dma buffer!\n");
			rc = -ENOMEM;
		} else {
			edb->addr = ext_buf->addr;
			edb->size = ext_buf->size;
			list_add_tail(&edb->entry, &handle->extdmaqueue);
		}
#endif
		break;
	}
	case VIV_VIDIOC_BUFFER_FREE: {
		struct ext_buf_info *ext_buf = (struct ext_buf_info *)arg;
#ifdef CONFIG_VIDEOBUF2_DMA_CONTIG
		struct ext_dma_buf *b, *edb = NULL;

		pr_debug("priv ioctl VIV_VIDIOC_BUFFER_FREE\n");
		list_for_each_entry(b, &handle->extdmaqueue, entry) {
			if (b->addr == ext_buf->addr) {
				edb = b;
				break;
			}
		}

		if (edb) {
			dma_free_attrs(handle->queue.dev, edb->size,
					edb->vaddr, edb->addr,
					DMA_ATTR_WRITE_COMBINE);
			list_del(&edb->entry);
			kfree(edb);
		}
#endif
		break;
	}
	case VIV_VIDIOC_CONTROL_EVENT:
		pr_debug("priv ioctl VIV_VIDIOC_CONTROL_EVENT\n");
		control_event = (struct viv_control_event *)arg;
		rc = viv_post_control_event(handle->streamid, &handle->vfh,
					    control_event);
		break;
	case VIV_VIDIOC_QUERY_EXTMEM:
		pr_debug("priv ioctl VIV_VIDIOC_QUERY_EXTMEM\n");
		ext_buf = (struct ext_buf_info *)arg;
		ext_buf->addr = RESERVED_MEM_BASE;
		ext_buf->size = RESERVED_MEM_SIZE;
		break;
	case VIV_VIDIOC_S_MODEINFO:
		if (!dev)
			break;
		modeinfo = (struct vvcam_constant_modeinfo *)arg;
		for (i = 0; i < dev->modeinfocount; ++i) {
			if (dev->modeinfo[i].index == modeinfo->index) {
				pr_info("sensor mode info already configured!\n");
				break;
			}
		}
		if (i < dev->modeinfocount || modeinfo->w == 0 ||
			modeinfo->h == 0 || modeinfo->fps == 0) {
			rc = -EINVAL;
			break;
		}
		if (dev->modeinfocount < ARRAY_SIZE(dev->modeinfo)) {
			memcpy(&dev->modeinfo[dev->modeinfocount],
					modeinfo, sizeof(*modeinfo));
			dev->modeinfocount++;
		}

		if (!bayer_pattern_to_format(modeinfo->brpat,
				modeinfo->bitw, &fmt) &&
				dev->formatscount < ARRAY_SIZE(dev->formats)) {
			for (i = 0; i < dev->formatscount; ++i)
				if (dev->formats[i].fourcc == fmt.fourcc)
					break;
			if (i == dev->formatscount) {
				memcpy(&dev->formats[dev->formatscount],
						&fmt, sizeof(fmt));
				dev->formatscount++;
			}
		}
		break;

	case VIV_VIDIOC_S_CAPS_MODE:
		memcpy(&(dev->caps_mode),arg,sizeof(dev->caps_mode));
		rc = set_caps_mode_event(file);
		if (rc == 0)
			rc = dev->event_result;
		break;

	case VIV_VIDIOC_G_CAPS_MODE:
		memcpy(arg, &(dev->caps_mode), sizeof(dev->caps_mode));
		break;

	case VIV_VIDIOC_EVENT_RESULT:
		dev->event_result = *(int*)arg;
		break;
	
	case VIV_VIDIOC_GET_CAPS_SUPPORTS:{
		pcaps_supports = (struct viv_caps_supports *)arg;
		rc = get_caps_suppots_event(file);
		memcpy(pcaps_supports,&(dev->caps_supports),sizeof(dev->caps_supports));
		break;
	}
		
	case VIV_VIDIOC_SET_CAPS_SUPPORTS:
		pcaps_supports = (struct viv_caps_supports *)arg;
		memcpy(&(dev->caps_supports),arg,sizeof(dev->caps_supports));
		break;
	}
	return rc;
}

static int video_querycap(struct file *file, void *fh,
			  struct v4l2_capability *cap)
{
	struct viv_video_device *dev = video_drvdata(file);
	pr_debug("enter %s\n", __func__);
	strcpy(cap->driver, "viv_v4l2_device");
	strcpy(cap->card, "VIV");
	cap->bus_info[0] = 0;
	if(dev)
	{
		snprintf((char *)cap->bus_info, sizeof(cap->bus_info), "platform:viv%d", dev->id);
	}
	
	cap->capabilities =
		V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING | V4L2_CAP_DEVICE_CAPS;
	cap->device_caps = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING;
	return 0;
}

static int vidioc_enum_fmt_vid_cap(struct file *file, void *priv,
				   struct v4l2_fmtdesc *f)
{
	struct viv_video_file *handle = priv_to_handle(file->private_data);
	struct viv_video_device *dev = video_drvdata(file);

	if (!handle->capsqueried) {
		handle->capsqueried = true;
		query_session_caps(file);
	}

	if (f->index < dev->formatscount) {
		f->pixelformat = dev->formats[f->index].fourcc;
		return 0;
	}
	return -EINVAL;
}

static int vidioc_g_fmt_vid_cap(struct file *file, void *priv,
				struct v4l2_format *f)
{
	struct viv_video_file *handle = priv_to_handle(file->private_data);

	pr_debug("enter %s\n", __func__);

	if (f->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;
	*f = handle->fmt;
	return 0;
}

static int vidioc_try_fmt_vid_cap(struct file *file, void *priv,
				  struct v4l2_format *f)
{
	struct viv_video_file *handle = priv_to_handle(file->private_data);
	struct viv_video_device *dev = video_drvdata(file);
	struct viv_video_fmt *format = NULL;
	int bytesperline, sizeimage;
	int i;

	pr_debug("enter %s\n", __func__);

	if (f->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	if (!handle->capsqueried) {
		handle->capsqueried = true;
		query_session_caps(file);
	}

	for (i = 0; i < dev->formatscount; ++i) {
		if (dev->formats[i].fourcc == f->fmt.pix.pixelformat) {
			format = &dev->formats[i];
			break;
		}
	}
	if (format == NULL)
		return -EINVAL;

	f->fmt.pix.field = V4L2_FIELD_NONE;
	v4l_bound_align_image(&f->fmt.pix.width, 48, 3840, 2,
			      &f->fmt.pix.height, 32, 2160, 0, 0);
	bytesperline = f->fmt.pix.width * format->bpp;
	bytesperline = ALIGN_UP(bytesperline, 16);
	if (f->fmt.pix.bytesperline < bytesperline)
		f->fmt.pix.bytesperline = bytesperline;
	sizeimage = ALIGN_UP(f->fmt.pix.height * ALIGN_UP((f->fmt.pix.width *
			format->depth) / 8, 16), 4096);
	if (f->fmt.pix.sizeimage < sizeimage)
		f->fmt.pix.sizeimage = sizeimage;
	f->fmt.pix.colorspace = V4L2_COLORSPACE_REC709;
	return 0;
}

static int vidioc_s_fmt_vid_cap(struct file *file, void *priv,
				struct v4l2_format *f)
{
	struct viv_video_file *ph;
	struct viv_video_file *handle = priv_to_handle(file->private_data);
	int ret;

	pr_debug("enter %s\n", __func__);
	if (f->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	ret = vidioc_try_fmt_vid_cap(file, priv, f);

	if (ret < 0)
		return -EINVAL;

	list_for_each_entry(ph, &file_list_head, entry) {
		if (ph->streamid == handle->streamid) {
			memcpy(&ph->fmt, f, sizeof(*f));
		}
	}
	return ret;
}

static int vidioc_reqbufs(struct file *file, void *priv,
			  struct v4l2_requestbuffers *p)
{
	struct viv_video_file *handle = priv_to_handle(file->private_data);
	struct viv_video_file *ph;
	struct v4l2_event event;
	struct viv_video_event *v_event;
	unsigned long flags;
	int ret = 0;

	pr_debug("enter %s %d %d\n", __func__, p->count, p->memory);
	if (p->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	spin_lock_irqsave(&file_list_lock, flags);
	list_for_each_entry(ph, &file_list_head, entry) {
		if (ph->streamid == handle->streamid) {
			if (ph != handle && ph->req) {
				pr_err("stream is busy %d\n", ph->streamid);
				spin_unlock_irqrestore(&file_list_lock, flags);
				return -EBUSY;
			}
		}
	}
	spin_unlock_irqrestore(&file_list_lock, flags);

	if (p->count == 0)
		handle->req = false;
	else
		handle->req = true;

	mutex_lock(&handle->buffer_mutex);
	ret = vb2_reqbufs(&handle->queue, p);
	mutex_unlock(&handle->buffer_mutex);

	if (p->count == 0) {
		memset(p, 0, sizeof(*p));
		return ret;
	}
	if (ret < 0)
		return ret;

#ifdef ENABLE_IRQ
	if (!handle->req)
		clear_buffer_queue();
#endif

	if (handle->streamid < 0 || handle->state > 0)
		return ret;
	handle->state = 1;
#ifdef ENABLE_IRQ
	sync_state(handle->state);
#endif
	ret =
	    viv_post_simple_event(VIV_VIDEO_EVENT_NEW_STREAM, handle->streamid,
				  &handle->vfh, true);

	v_event = (struct viv_video_event *)&event.u.data[0];
	v_event->stream_id = handle->streamid;
	v_event->file = &handle->vfh;
	v_event->addr = handle->fmt.fmt.pix.width;
	v_event->response = handle->fmt.fmt.pix.height;
	v_event->buf_index = handle->fmt.fmt.pix.pixelformat;
	v_event->sync = true;
	event.type = VIV_VIDEO_EVENT_TYPE;
	event.id = VIV_VIDEO_EVENT_SET_FMT;
	return viv_post_event(&event, &handle->vfh, true);
}

static int vidioc_querybuf(struct file *file, void *priv, struct v4l2_buffer *p)
{
	struct viv_video_file *handle = priv_to_handle(file->private_data);
	int rc = 0;

	pr_debug("enter %s\n", __func__);

	if (p->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	mutex_lock(&handle->buffer_mutex);
	rc = vb2_querybuf(&handle->queue, p);
	mutex_unlock(&handle->buffer_mutex);
	return rc;
}

static int vidioc_qbuf(struct file *file, void *priv, struct v4l2_buffer *p)
{
	struct viv_video_file *handle = priv_to_handle(file->private_data);
	int rc = 0;

	if (p->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;
	mutex_lock(&handle->buffer_mutex);
#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 0, 0)
	rc = vb2_qbuf(&handle->queue, NULL, p);
#else
	rc = vb2_qbuf(&handle->queue, p);
#endif
	mutex_unlock(&handle->buffer_mutex);
	return rc;
}

static int vidioc_dqbuf(struct file *file, void *priv, struct v4l2_buffer *p)
{
	struct viv_video_file *handle = priv_to_handle(file->private_data);
	int rc = 0;

	rc = vb2_dqbuf(&handle->queue, p, file->f_flags & O_NONBLOCK);
	p->field = V4L2_FIELD_NONE;
	p->sequence = handle->sequence++;
	return rc;
}

static int vidioc_expbuf(struct file *file, void *priv,
				struct v4l2_exportbuffer *p)
{
	struct viv_video_file *handle = priv_to_handle(file->private_data);

	pr_debug("enter %s\n", __func__);
	return vb2_expbuf(&handle->queue, p);
}

static int vidioc_streamon(struct file *file, void *priv, enum v4l2_buf_type i)
{
	struct viv_video_file *handle = priv_to_handle(file->private_data);

	pr_debug("enter %s\n", __func__);
	handle->capsqueried = false;
	memset(&handle->timeperframe, 0, sizeof(handle->timeperframe));
	return vb2_streamon(&handle->queue, i);
}

static int vidioc_streamoff(struct file *file, void *priv, enum v4l2_buf_type i)
{
	struct viv_video_file *handle = priv_to_handle(file->private_data);
	int rc;

	pr_debug("enter %s\n", __func__);

	mutex_lock(&handle->buffer_mutex);
	rc = vb2_streamoff(&handle->queue, i);
	mutex_unlock(&handle->buffer_mutex);
	return rc;
}

static int vidioc_enum_input(struct file *filep, void *fh,
			     struct v4l2_input *input)
{
	if (input->index > 0)
		return -EINVAL;

	strlcpy(input->name, "camera", sizeof(input->name));
	input->type = V4L2_INPUT_TYPE_CAMERA;

	return 0;
}

static int vidioc_g_input(struct file *filep, void *fh, unsigned int *input)
{
	*input = 0;
	return 0;
}

static int vidioc_s_input(struct file *filep, void *fh, unsigned int input)
{
	return input == 0 ? 0 : -EINVAL;
}

static int vidioc_enum_framesizes(struct file *file, void *priv,
							struct v4l2_frmsizeenum *fsize)
{
	struct viv_video_file *handle = priv_to_handle(file->private_data);
	struct viv_video_device *dev = video_drvdata(file);
	int i;

	if (!handle->capsqueried) {
		handle->capsqueried = true;
		query_session_caps(file);
	}

	if (fsize->index >= dev->modeinfocount)
		return -EINVAL;

	for (i = 0; i < dev->formatscount; ++i)
		if (dev->formats[i].fourcc == fsize->pixel_format)
			break;

	if (i == dev->formatscount)
		return -EINVAL;

	fsize->discrete.width = dev->modeinfo[fsize->index].w;
	fsize->discrete.height = dev->modeinfo[fsize->index].h;
	fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
	return 0;
}

static inline void update_timeperframe(struct file *file)
{
	struct viv_video_file *handle = priv_to_handle(file->private_data);
	struct viv_video_device *dev = video_drvdata(file);
	int i;

	if (!handle->capsqueried) {
		handle->capsqueried = true;
		query_session_caps(file);
	}

	if (handle->timeperframe.numerator == 0 ||
		handle->timeperframe.denominator == 0) {
		handle->timeperframe.numerator = 1;
		handle->timeperframe.denominator = 30;
		for (i = 0; i < dev->modeinfocount; ++i) {
			if (handle->fmt.fmt.pix.width == dev->modeinfo[i].w &&
				handle->fmt.fmt.pix.height == dev->modeinfo[i].h) {
				handle->timeperframe.denominator = dev->modeinfo[i].fps;
				break;
			}
		}
	}
}

static int vidioc_g_parm(struct file *file, void *fh,
			        struct v4l2_streamparm *a)
{
	struct viv_video_file *handle = priv_to_handle(file->private_data);

	if (a->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	update_timeperframe(file);

	memset(&a->parm, 0, sizeof(a->parm));
	a->parm.capture.capability = V4L2_CAP_TIMEPERFRAME;
	a->parm.capture.timeperframe = handle->timeperframe;
	return 0;
}

static int vidioc_s_parm(struct file *file, void *fh,
			        struct v4l2_streamparm *a)
{
	struct viv_video_file *handle = priv_to_handle(file->private_data);

	if (a->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	if (a->parm.capture.timeperframe.numerator == 0 ||
			a->parm.capture.timeperframe.denominator == 0) {
		update_timeperframe(file);
		memset(&a->parm, 0, sizeof(a->parm));
		a->parm.capture.capability = V4L2_CAP_TIMEPERFRAME;
		a->parm.capture.timeperframe = handle->timeperframe;
		return 0;
	}
	handle->timeperframe = a->parm.output.timeperframe;
	return 0;
}

static int vidioc_enum_frameintervals(struct file *filp, void *priv,
			                          struct v4l2_frmivalenum *fival)
{
	struct viv_video_file *handle = priv_to_handle(filp->private_data);
	struct viv_video_device *dev = video_drvdata(filp);
	int i, count = fival->index;

	if (!handle->capsqueried) {
		handle->capsqueried = true;
		query_session_caps(filp);
	}

	if (fival->index >= dev->modeinfocount)
		return -EINVAL;

	for (i = 0; i < dev->formatscount; ++i)
		if (dev->formats[i].fourcc == fival->pixel_format)
			break;

	if (i == dev->formatscount)
		return -EINVAL;

	for (i = 0; i < dev->modeinfocount; ++i) {
		if (fival->width != dev->modeinfo[i].w ||
			fival->height != dev->modeinfo[i].h)
			continue;
		if (count > 0) {
			count--;
			continue;
		}
		fival->discrete.numerator = 1;
		fival->discrete.denominator = dev->modeinfo[i].fps;
		fival->type = V4L2_FRMIVAL_TYPE_DISCRETE;
		return 0;
	}
	return -EINVAL;
}

int viv_gen_g_ctrl(struct v4l2_ctrl *ctrl)
{
	struct viv_custom_ctrls *cc =
		container_of(ctrl->handler, struct viv_custom_ctrls, handler);
	struct viv_video_device *vdev =
		container_of(cc, struct viv_video_device, ctrls);
	struct v4l2_event event;
	struct viv_video_event *v_event;
	struct v4l2_ctrl_data *p_data;
	struct v4l2_ext_control *p_ctrl;

	pr_debug("%s:ctrl->id=0x%x\n",__func__,ctrl->id);
	memset(&event, 0, sizeof(event));
	v_event = (struct viv_video_event *)&event.u.data[0];
	v_event->file = NULL;
	v_event->sync = true;
	v_event->addr = vdev->ctrls.buf_pa;
	event.type = VIV_VIDEO_EVENT_TYPE;
	event.id = VIV_VIDEO_EVENT_EXTCTRL2;

	p_data = (struct v4l2_ctrl_data *)vdev->ctrls.buf_va;
	if (unlikely(!p_data))
		return -ENOMEM;

	memset(p_data, 0, sizeof(*p_data));
	p_data->dir = V4L2_CTRL_GET;
	p_data->ctrls.count = 1;
	p_ctrl = nextof(p_data, struct v4l2_ext_control *);
	p_ctrl->id = ctrl->id;

	v4l2_event_queue(vdev->video, &event);

	reinit_completion(&vdev->ctrls.wait);
	if (wait_for_completion_timeout(&vdev->ctrls.wait,
			msecs_to_jiffies(VIV_VIDEO_EVENT_TIMOUT_MS)) == 0) {
		pr_err("connecting to server timed out (g)!");
		*ctrl->p_new.p_s32 = *ctrl->p_cur.p_s32;
		return 0;
	}

	if (p_ctrl->id == ctrl->id && !p_data->ret) {
		*ctrl->p_new.p_s32 = p_ctrl->value;
		return 0;
	}
	return -EINVAL;
}

int viv_gen_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct viv_custom_ctrls *cc =
		container_of(ctrl->handler, struct viv_custom_ctrls, handler);
	struct viv_video_device *vdev =
		container_of(cc, struct viv_video_device, ctrls);
	struct v4l2_event event;
	struct viv_video_event *v_event;
	struct v4l2_ctrl_data *p_data;
	struct v4l2_ext_control *p_ctrl;

	pr_debug("%s:ctrl->id=0x%x\n",__func__,ctrl->id);
	memset(&event, 0, sizeof(event));
	v_event = (struct viv_video_event *)&event.u.data[0];
	v_event->file = NULL;
	v_event->sync = true;
	v_event->addr = vdev->ctrls.buf_pa;
	event.type = VIV_VIDEO_EVENT_TYPE;
	event.id = VIV_VIDEO_EVENT_EXTCTRL2;

	p_data = (struct v4l2_ctrl_data *)vdev->ctrls.buf_va;
	if (unlikely(!p_data))
		return -ENOMEM;

	memset(p_data, 0, sizeof(*p_data));
	p_data->dir = V4L2_CTRL_SET;
	p_data->ctrls.count = 1;
	p_ctrl = nextof(p_data, struct v4l2_ext_control *);
	p_ctrl->id = ctrl->id;
	p_ctrl->value = *ctrl->p_new.p_s32;

	v4l2_event_queue(vdev->video, &event);

	reinit_completion(&vdev->ctrls.wait);
	if (wait_for_completion_timeout(&vdev->ctrls.wait,
			msecs_to_jiffies(VIV_VIDEO_EVENT_TIMOUT_MS)) == 0) {
		pr_err("connecting to server timed out (s)!");
		return 0;
	}

	if (p_ctrl->id == ctrl->id && !p_data->ret)
		return 0;
	return -EINVAL;
}

static const struct v4l2_ioctl_ops video_ioctl_ops = {
	.vidioc_querycap = video_querycap,
	.vidioc_enum_fmt_vid_cap = vidioc_enum_fmt_vid_cap,
	.vidioc_g_fmt_vid_cap = vidioc_g_fmt_vid_cap,
	.vidioc_try_fmt_vid_cap = vidioc_try_fmt_vid_cap,
	.vidioc_s_fmt_vid_cap = vidioc_s_fmt_vid_cap,
	.vidioc_reqbufs = vidioc_reqbufs,
	.vidioc_querybuf = vidioc_querybuf,
	.vidioc_qbuf = vidioc_qbuf,
	.vidioc_expbuf = vidioc_expbuf,
	.vidioc_dqbuf = vidioc_dqbuf,
	.vidioc_streamon = vidioc_streamon,
	.vidioc_streamoff = vidioc_streamoff,
	.vidioc_subscribe_event = subscribe_event,
	.vidioc_unsubscribe_event = unsubscribe_event,
	.vidioc_default = private_ioctl,
	.vidioc_enum_input = vidioc_enum_input,
	.vidioc_g_input = vidioc_g_input,
	.vidioc_s_input = vidioc_s_input,
	.vidioc_enum_framesizes = vidioc_enum_framesizes,
	.vidioc_enum_frameintervals = vidioc_enum_frameintervals,
	.vidioc_g_parm = vidioc_g_parm,
	.vidioc_s_parm = vidioc_s_parm,
};

/* sys /dev/mem can't map large memory size */
static int viv_private_mmap(struct file *file, struct vm_area_struct *vma)
{
	/* Map reserved video memory. */
	if (remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff,
			    vma->vm_end - vma->vm_start, vma->vm_page_prot))
		return -EAGAIN;
	return 0;
}

int vidioc_mmap(struct file *file, struct vm_area_struct *vma)
{
	int rc;
	struct viv_video_file *handle = priv_to_handle(file->private_data);

#ifdef ENABLE_IRQ
	if (handle->streamid < 0)
#else
	if (vma->vm_pgoff >= ((unsigned long)RESERVED_MEM_BASE) >> PAGE_SHIFT)
#endif
		rc = viv_private_mmap(file, vma);
	else
		rc = vb2_mmap(&handle->queue, vma);
	return rc;
}

static unsigned int video_poll(struct file *file,
			       struct poll_table_struct *wait)
{
	struct viv_video_file *handle = priv_to_handle(file->private_data);
	int rc = 0;

	if (handle->streamid < 0) {
		poll_wait(file, &handle->vfh.wait, wait);

		if (v4l2_event_pending(&handle->vfh))
			rc = POLLIN | POLLRDNORM;
	} else {
		mutex_lock(&handle->buffer_mutex);
		rc = vb2_poll(&handle->queue, file, wait) | v4l2_ctrl_poll(file, wait);
		mutex_unlock(&handle->buffer_mutex);
	}
	return rc;
}

static struct v4l2_file_operations video_ops = {
	.owner = THIS_MODULE,
	.open = video_open,
	.release = video_close,
	.poll = video_poll,
	.unlocked_ioctl = video_ioctl2,
	.mmap = vidioc_mmap,
};

static void pdev_release(struct device *dev)
{
	pr_debug("enter %s\n", __func__);
}

static struct platform_device viv_pdev = {
	.name = "vvcam-video",
	.dev.release = pdev_release,
};

static int viv_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct v4l2_event event;
	struct viv_video_event *v_event;
	struct viv_custom_ctrls *cc =
		container_of(ctrl->handler, struct viv_custom_ctrls, handler);
	struct viv_video_device *vdev =
		container_of(cc, struct viv_video_device, ctrls);
	int ret = -1;
	char *szbuf = NULL;

	switch (ctrl->id) {
	case V4L2_CID_VIV_STRING: {
		ret = 0;
		szbuf = (char *)vdev->ctrls.buf_va;
		if (!ctrl->p_new.p_char || !szbuf)
			return -EINVAL;
		strcpy(szbuf, ctrl->p_new.p_char);
		v_event = (struct viv_video_event *)&event.u.data[0];
		v_event->stream_id = 0;
		v_event->file = NULL;
		v_event->sync = true;
		v_event->addr = vdev->ctrls.buf_pa;
		event.type = VIV_VIDEO_EVENT_TYPE;
		event.id = VIV_VIDEO_EVENT_EXTCTRL;

		v4l2_event_queue(vdev->video, &event);

		reinit_completion(&vdev->ctrls.wait);
		if (wait_for_completion_timeout(&vdev->ctrls.wait,
						msecs_to_jiffies
						(VIV_VIDEO_EVENT_TIMOUT_MS)) == 0)
			ret = -ETIMEDOUT;
		strcpy(ctrl->p_new.p_char, szbuf);
		break;
	}
	}
	return ret;
}

static const struct v4l2_ctrl_ops viv_ctrl_ops = {
	.s_ctrl = viv_s_ctrl,
};

static const struct v4l2_ctrl_config viv_ext_ctrl = {
	.ops = &viv_ctrl_ops,
	.id = V4L2_CID_VIV_STRING,
	.name = "viv_ext_ctrl",
	.type = V4L2_CTRL_TYPE_STRING,
	.max = VIV_JSON_BUFFER_SIZE-1,
	.step = 1,
};

static int viv_video_probe(struct platform_device *pdev)
{
	struct viv_video_device *vdev;
	int rc = 0;
	int i;

	pr_debug("enter %s\n", __func__);

	for (i = 0; i < VIDEO_NODE_NUM; i++) {
		vvdev[i] = kzalloc(sizeof(*vdev), GFP_KERNEL);
		if (WARN_ON(!vvdev[i])) {
			rc = -ENOMEM;
			goto probe_end;
		}
		vdev = vvdev[i];
		memset(vdev, 0, sizeof(*vdev));
		
		vdev->id = i;
		vdev->v4l2_dev = kzalloc(sizeof(*vdev->v4l2_dev), GFP_KERNEL);
		if (WARN_ON(!vdev->v4l2_dev)) {
			rc = -ENOMEM;
			goto probe_end;
		}
		vdev->video = video_device_alloc();
		vdev->video->v4l2_dev = vdev->v4l2_dev;
		rc = v4l2_device_register(&(pdev->dev), vdev->video->v4l2_dev);
		if (WARN_ON(rc < 0))
			goto register_fail;
		sprintf(vdev->video->name, "viv_v4l2%d", i);
		v4l2_ctrl_handler_init(&vdev->ctrls.handler, 1);
		vdev->ctrls.request = v4l2_ctrl_new_custom(&vdev->ctrls.handler,
					        &viv_ext_ctrl, NULL);
		create_controls(&vdev->ctrls.handler);

		vdev->video->release = video_device_release;
		vdev->video->fops = &video_ops;
		vdev->video->ioctl_ops = &video_ioctl_ops;
		vdev->video->minor = -1;
		vdev->video->vfl_type = VFL_TYPE_GRABBER;
		vdev->video->ctrl_handler = &vdev->ctrls.handler;
#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 0, 0)
		vdev->video->device_caps = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING;
#endif
		rc = video_register_device(vdev->video, VFL_TYPE_GRABBER, -1);
		if (WARN_ON(rc < 0))
			goto register_fail;
		video_set_drvdata(vdev->video, vdev);

		rc = v4l2_device_register_subdev_nodes(vdev->v4l2_dev);
		vdev->ctrls.buf_va = kmalloc(VIV_JSON_BUFFER_SIZE, GFP_KERNEL);
		vdev->ctrls.buf_pa = __pa(vdev->ctrls.buf_va);
		init_completion(&vdev->ctrls.wait);
		continue;
register_fail:
		video_device_release(vdev->video);
	}
probe_end:
	return rc;
}

static int viv_video_remove(struct platform_device *pdev)
{
	struct viv_video_device *vdev;
	int i;

	pr_info("enter %s\n", __func__);

	for (i = VIDEO_NODE_NUM-1; i >=0; i--) {
		vdev = vvdev[i];
		if (!vdev || !vdev->video)
			continue;
		video_unregister_device(vdev->video);
		kfree(vdev->ctrls.buf_va);
		v4l2_ctrl_handler_free(&vdev->ctrls.handler);
		v4l2_device_disconnect(vdev->video->v4l2_dev);
		v4l2_device_put(vdev->video->v4l2_dev);
		kzfree(vvdev[i]);
		vvdev[i] = NULL;
	}
	return 0;
}

static struct platform_driver viv_video_driver = {
	.probe = viv_video_probe,
	.remove = viv_video_remove,
	.driver = {
		   .name = "vvcam-video",
		   .owner = THIS_MODULE,
		   },
};

static int __init viv_video_init_module(void)
{
	int ret = 0;

	pr_info("enter %s\n", __func__);
	ret = platform_device_register(&viv_pdev);
	if (ret) {
		pr_err("register platform device failed.\n");
		return ret;
	}

	ret = platform_driver_register(&viv_video_driver);
	if (ret) {
		pr_err("register platform driver failed.\n");
		platform_device_unregister(&viv_pdev);
		return ret;
	}
	return ret;
}

static void __exit viv_video_exit_module(void)
{
	pr_info("enter %s\n", __func__);
	platform_driver_unregister(&viv_video_driver);
	msleep(100);
	platform_device_unregister(&viv_pdev);
}

module_init(viv_video_init_module);
module_exit(viv_video_exit_module);

MODULE_DESCRIPTION("Verisilicon V4L2 video driver");
MODULE_AUTHOR("Verisilicon ISP SW Team");
MODULE_LICENSE("GPL v2");
