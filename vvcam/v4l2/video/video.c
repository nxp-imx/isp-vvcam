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

#include "video.h"
#include "cma.h"
#include "vvdefs.h"

#define DEF_PLANE_NO (0)
#define VIDEO_NODE_NUM 2

static struct viv_video_device *vvdev[VIDEO_NODE_NUM];
LIST_HEAD(file_list_head);
static DEFINE_SPINLOCK(file_list_lock);

#ifdef ENABLE_IRQ
struct ext_dma_buf {
	dma_addr_t addr;
	void *vaddr;
	size_t size;
	struct list_head entry;
};
#endif

struct viv_video_fmt {
	int fourcc;
	int depth;
};

static struct viv_video_fmt formats[] = {
	{
	 .fourcc = V4L2_PIX_FMT_YUYV,
	 .depth = 16,
	 },
	{
	 .fourcc = V4L2_PIX_FMT_NV12,
	 .depth = 12,
	 },
	{
	 .fourcc = V4L2_PIX_FMT_NV16,
	 .depth = 16,
	 },
};

static struct viv_video_fmt *format_by_fourcc(unsigned int fourcc)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(formats); i++)
		if (formats[i].fourcc == fourcc)
			return formats + i;
	return NULL;
}

static int viv_post_event(struct v4l2_event *event, void *fh, bool sync)
{
	struct viv_video_file *handle = priv_to_handle(fh);
	int rc;

	mutex_lock(&handle->event_mutex);
	v4l2_event_queue(handle->vdev->video, event);
	mutex_unlock(&handle->event_mutex);

	if (sync) {
		reinit_completion(&handle->wait);
		rc = wait_for_completion_timeout(&handle->wait,
						 msecs_to_jiffies
						 (VIV_VIDEO_EVENT_TIMOUT_MS));
		if (rc == 0)
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

	if (!file)
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

struct vb2_dc_buf *viv_dequeue_sg_buffer(int streamid, struct vb2_dc_buf *src)
{
	struct viv_video_file *file = get_fh(streamid);
	struct vb2_dc_buf *b, *buf = NULL;
	unsigned long flags;

	if (!src || !file)
		return NULL;

	spin_lock_irqsave(&file->irqlock, flags);
	if (list_empty(&file->dmaqueueidle)) {
		spin_unlock_irqrestore(&file->irqlock, flags);
		return NULL;
	}

	list_for_each_entry(b, &file->dmaqueueidle, irqlist) {
		if (b->group == src->group) {
			buf = b;
			break;
		}
	}
	if (buf)
		list_del(&buf->irqlist);
	spin_unlock_irqrestore(&file->irqlock, flags);
	return buf;
}
EXPORT_SYMBOL(viv_dequeue_sg_buffer);

void viv_queue_buffer(int streamid, struct vb2_dc_buf *buf)
{
	struct viv_video_file *file = get_fh(streamid);
	unsigned long flags;

	if (!file)
		return;

	spin_lock_irqsave(&file->irqlock, flags);
	list_add_tail(&buf->irqlist, &file->dmaqueueidle);
	spin_unlock_irqrestore(&file->irqlock, flags);
}
EXPORT_SYMBOL(viv_queue_buffer);

void viv_buffer_ready(int streamid, struct vb2_dc_buf *buf)
{
	struct viv_video_file *file;
	int targetid;
	unsigned long flags;

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
	if (!file)
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
		buf->vb.vb2_buf.planes[DEF_PLANE_NO].bytesused =
				file->fmt.fmt.pix.sizeimage;
#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 0, 0)
		buf->vb.vb2_buf.timestamp = ktime_get_ns();
#endif
		vb2_buffer_done(&buf->vb.vb2_buf, VB2_BUF_STATE_DONE);
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
	if (!file)
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
	if (!file)
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
		viv_post_simple_event(VIV_VIDEO_EVENT_START_STREAM,
				      handle->streamid, fh, true);
		handle->state = 2;
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

	if (!buf->group)
		buf->group = RESV_STREAMID_ISP(video->num);
	if (!buf->user)
		buf->user = buf->group;

	spin_lock_irqsave(&file_list_lock, flags);
	do {
		list_for_each_entry(ph, &file_list_head, entry) {
			if (ph->streamid == buf->user) {
				file = ph;
				break;
			}
		}
	} while (!file && buf->user != buf->group && (buf->user = buf->group));
	spin_unlock_irqrestore(&file_list_lock, flags);

	if (!file)
		return;

	spin_lock_irqsave(&file->irqlock, flags);
	list_add_tail(&buf->irqlist, &file->dmaqueueidle);
	spin_unlock_irqrestore(&file->irqlock, flags);

	spin_lock_irqsave(&file->idle.lock, flags);
	if (file->idle.on_notify)
		file->idle.on_notify(buf->group, file->idle.data);
	spin_unlock_irqrestore(&file->idle.lock, flags);
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
#else
	v_event->addr = (u64) buf->cookie;
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

#ifndef CONFIG_VIDEOBUF2_DMA_CONTIG
static void vb2_cma_put(void *buf_priv)
{
	struct vb2_dc_buf *buf = buf_priv;

	pr_debug("enter %s\n", __func__);
	if (!buf)
		return;
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 9, 0)
	if (!refcount_dec_and_test(&buf->refcount))
		return;
#else
	if (!atomic_dec_and_test(&buf->refcount))
		return;
#endif
	pr_debug("buf->cookie: %p\n", buf->cookie);
	if ((u64) buf->cookie < RESERVED_MEM_BASE)
		return;
	vsi_cma_free((u64) buf->cookie);
	put_device(buf->dev);
	kfree(buf);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0)
static void *vb2_cma_alloc(void *alloc_ctx, unsigned long size,
			   enum dma_data_direction dma_dir, gfp_t gfp_flags)
{
	struct vb2_dc_buf *conf = alloc_ctx;
	struct device *dev = conf->dev;
#else
static void *vb2_cma_alloc(struct device *dev, unsigned long attrs,
			   unsigned long size, enum dma_data_direction dma_dir,
			   gfp_t gfp_flags)
{
#endif
	struct vb2_dc_buf *buf;

	if (!dev || !dev->driver_data) {
		pr_err("dev or dev->driver_data is NULL.\n");
		return ERR_PTR(-EINVAL);
	}

	buf = kzalloc(sizeof(*buf), GFP_KERNEL);
	if (!buf)
		return ERR_PTR(-ENOMEM);

	buf->cookie = (void *)vsi_cma_alloc(size);
	pr_debug("alloc memory addr: %p %p.\n", buf, buf->cookie);

	if (!buf->cookie) {
		dev_err(dev, "dma_alloc_coherent of size %ld failed\n", size);
		kfree(buf);
		return ERR_PTR(-ENOMEM);
	}

	buf->dev = get_device(dev);
	buf->size = size;
	buf->handler.refcount = &buf->refcount;
	buf->handler.put = vb2_cma_put;
	buf->handler.arg = buf;

#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 9, 0)
	refcount_set(&buf->refcount, 1);
#else
	atomic_inc(&buf->refcount);
#endif
	return buf;
}

static int vb2_cma_mmap(void *buf_priv, struct vm_area_struct *vma)
{
	struct vb2_dc_buf *buf = buf_priv;

	pr_debug("enter %s\n", __func__);

	if (!buf || buf->cookie == 0) {
		pr_err("No buffer to map\n");
		return -EINVAL;
	}

	pr_debug("mmap buf->cookie: %p, size:%lu. %p\n",
		 (void *)(unsigned long)buf->cookie, buf->size, buf);
	vma->vm_pgoff = 0;
	if (remap_pfn_range(vma, vma->vm_start,
			    ((unsigned long)buf->cookie) >> PAGE_SHIFT,
			    buf->size, vma->vm_page_prot)) {
		pr_err("failed.\n");
		return -EAGAIN;
	}
	vma->vm_flags |= VM_DONTEXPAND | VM_DONTDUMP;
	vma->vm_private_data = &buf->handler;
	vma->vm_ops = &vb2_common_vm_ops;

	vma->vm_ops->open(vma);

	return 0;
}

static unsigned int vb2_cma_num_users(void *buf_priv)
{
	return 0;
}

const struct vb2_mem_ops cma_memops = {
	.alloc = vb2_cma_alloc,
	.put = vb2_cma_put,
	.mmap = vb2_cma_mmap,
	.num_users = vb2_cma_num_users,
};
#endif

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
#else
	handle->queue.io_modes = VB2_MMAP;
	handle->queue.mem_ops = &cma_memops;
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
		}
		if (handle->streamid >= 0 && handle->state == 1)
			viv_post_simple_event(VIV_VIDEO_EVENT_DEL_STREAM,
					      handle->streamid, &handle->vfh,
					      false);

		handle->state = -1;
		handle->streamid = 0;
		spin_lock_irqsave(&file_list_lock, flags);
		list_del(&handle->entry);
		spin_unlock_irqrestore(&file_list_lock, flags);

		v4l2_fh_del(&handle->vfh);
		v4l2_fh_exit(&handle->vfh);

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
#ifndef CONFIG_VIDEOBUF2_DMA_CONTIG
	struct vb2_dc_buf *buf;
#endif

	mutex_lock(&handle->buffer_mutex);
	list_for_each_entry(vb, &handle->queue.queued_list, queued_entry) {
		if (!vb || !vb->planes)
			continue;
#ifdef CONFIG_VIDEOBUF2_DMA_CONTIG
		if (vb2_dma_contig_plane_dma_addr(vb, DEF_PLANE_NO) == addr) {
			vb->planes[DEF_PLANE_NO].bytesused =
					handle->fmt.fmt.pix.sizeimage;
#else
		buf = vb->planes[DEF_PLANE_NO].mem_priv;
		if (buf && (buf->cookie == (void *)addr)) {
			buf->vb.field = V4L2_FIELD_NONE;
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

static long private_ioctl(struct file *file, void *fh,
			  bool valid_prio, unsigned int cmd, void *arg)
{
	struct viv_video_file *handle;
	struct viv_video_file *ph;
	struct viv_video_event *v_event;
	struct v4l2_user_buffer *user_buffer;
	struct ext_buf_info *ext_buf;
	struct viv_control_event *control_event;
	unsigned long flags;
	int rc = 0;

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
		handle->is_endpoint = *((int *)arg) & 1;
#endif
		break;
	case VIV_VIDIOC_BUFFER_ALLOC: {
		struct ext_buf_info *ext_buf = (struct ext_buf_info *)arg;
#ifdef ENABLE_IRQ
		struct ext_dma_buf *edb = kzalloc(sizeof(*edb), GFP_KERNEL);

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
#else
		pr_debug("priv ioctl VIV_VIDIOC_BUFFER_ALLOC\n");
		ext_buf->addr = vsi_cma_alloc(ext_buf->size);
#endif
		break;
	}
	case VIV_VIDIOC_BUFFER_FREE: {
		struct ext_buf_info *ext_buf = (struct ext_buf_info *)arg;
#ifdef ENABLE_IRQ
		struct ext_dma_buf *b, *edb = NULL;

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
#else
		pr_debug("priv ioctl VIV_VIDIOC_BUFFER_FREE\n");
		vsi_cma_free(ext_buf->addr);
#endif
		break;
	}
	case VIV_VIDIOC_CONTROL_EVENT:
		pr_debug("priv ioctl VIV_VIDIOC_CONTROL_EVENT\n");
		control_event = (struct viv_control_event *)arg;
		rc = viv_post_control_event(handle->streamid, &handle->vfh,
					    control_event);
		break;
	case VIV_VIDIOC_QUERY_EXTMEM:{
			pr_debug("priv ioctl VIV_VIDIOC_QUERY_EXTMEM\n");
			ext_buf = (struct ext_buf_info *)arg;
			ext_buf->addr = RESERVED_MEM_BASE;
			ext_buf->size = RESERVED_MEM_SIZE;
		}
		break;
	}
	return rc;
}

static int video_querycap(struct file *file, void *fh,
			  struct v4l2_capability *cap)
{
	pr_debug("enter %s\n", __func__);
	strcpy(cap->driver, "viv_v4l2_device");
	strcpy(cap->card, "VIV");
	strcpy((char *)cap->bus_info, "PCI:viv");

	cap->capabilities =
	    V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING | V4L2_CAP_DEVICE_CAPS;
	cap->device_caps = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING;
	return 0;
}

static int vidioc_enum_fmt_vid_cap(struct file *file, void *priv,
				   struct v4l2_fmtdesc *f)
{
	if (unlikely(f->index >= ARRAY_SIZE(formats)))
		return -EINVAL;
	f->pixelformat = formats[f->index].fourcc;
	return 0;
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
	struct viv_video_fmt *format;

	pr_debug("enter %s\n", __func__);

	format = format_by_fourcc(f->fmt.pix.pixelformat);
	if (format == NULL) {
		return -EINVAL;
	}

	if (f->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	f->fmt.pix.field = V4L2_FIELD_NONE;
	v4l_bound_align_image(&f->fmt.pix.width, 48, 3840, 2,
			      &f->fmt.pix.height, 32, 2160, 0, 0);
	f->fmt.pix.bytesperline =
	    ALIGN_UP((f->fmt.pix.width * format->depth) / 8, 16);
	f->fmt.pix.sizeimage =
	    ALIGN_UP(f->fmt.pix.height * f->fmt.pix.bytesperline, 4096);
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

	if (handle->streamid < 0 || handle->state > 0)
		return ret;
	handle->state = 1;
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

static struct vvcam_constant_modeinfo {
	unsigned w;
	unsigned h;
	unsigned fps;
} vvcam_info[] = {
	{3840, 2160, 30},
	{1920, 1080, 30},
	{1280, 720, 30},
	{640, 480, 30},
};

static int vidioc_enum_framesizes(struct file *file, void *priv,
							struct v4l2_frmsizeenum *fsize)
{
	if (fsize->index >= ARRAY_SIZE(vvcam_info))
		return -EINVAL;
	switch (fsize->pixel_format) {
	case V4L2_PIX_FMT_YUYV:
	case V4L2_PIX_FMT_NV12:
	case V4L2_PIX_FMT_NV16:
		fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
		fsize->discrete.width = vvcam_info[fsize->index].w;
		fsize->discrete.height = vvcam_info[fsize->index].h;
		return 0;
	default:
		return -EINVAL;
	}
}

static int vidioc_g_parm(struct file *file, void *fh,
			        struct v4l2_streamparm *a)
{
	struct viv_video_file *handle = priv_to_handle(file->private_data);

	if (a->type != V4L2_BUF_TYPE_VIDEO_OUTPUT)
		return -EINVAL;

	memset(a, 0, sizeof(*a));
	a->type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	a->parm.output.capability = V4L2_CAP_TIMEPERFRAME;
	a->parm.output.timeperframe = handle->timeperframe;

	return 0;
}

static int vidioc_s_parm(struct file *file, void *fh,
			        struct v4l2_streamparm *a)
{
	struct viv_video_file *handle = priv_to_handle(file->private_data);

	if (a->type != V4L2_BUF_TYPE_VIDEO_OUTPUT)
		return -EINVAL;

	if (a->parm.output.timeperframe.denominator == 0)
		a->parm.output.timeperframe.denominator = 1;

	handle->timeperframe = a->parm.output.timeperframe;

	return 0;
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
	vsi_cma_init(RESERVED_MEM_BASE, RESERVED_MEM_SIZE, 4096);
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
	vsi_cma_release();
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
