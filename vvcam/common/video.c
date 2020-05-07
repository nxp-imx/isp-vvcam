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
#include <linux/module.h>
#include <linux/slab.h>
#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-ioctl.h>

#include <linux/platform_device.h>
#include <linux/pm_runtime.h>

#include <linux/kthread.h>
#include <linux/freezer.h>

#include "video.h"
#include "isp_driver.h"
#include "isp_ioctl.h"
#include "isp_types.h"
#include "mrv_all_bits.h"
#include "cma.h"

#ifdef WITH_DWE
#include "dwe_driver.h"
#endif

#ifdef WITH_VSE
#include "vse_driver.h"
#endif

#ifdef CSI_SENSOR_KERNEL
#include "mxc-mipi-csi2-sam.h"
#include "ov2775/ov2775_mipi_v3.h"
#include "os08a20/os08a20_mipi_v3.h"
#include "camera-proxy-driver/basler-camera-driver-vvcam/basler-camera-driver-vvcam.h"
#endif

static struct viv_video_device *vdev;
LIST_HEAD(file_list_head);
struct mutex file_list_lock;

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
	v4l2_event_queue(vdev->video, event);
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

static int start_streaming(struct vb2_queue *vq, unsigned int count)
{
	struct viv_video_file *handle = queue_to_handle(vq);
	struct v4l2_fh *fh = &handle->vfh;

	pr_debug("enter %s\n", __func__);
	if (handle->state != 2) {
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

	if (!handle || handle->state != 2 || list_empty(&handle->entry))
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
	struct v4l2_event event;
	struct viv_video_event *v_event;
	struct vb2_dc_buf *buf;

	if (!vb)
		return;

	handle = queue_to_handle(vb->vb2_queue);
	buf = vb->planes[0].mem_priv;

	if (!buf || !handle)
		return;

	/* pr_debug("buffer_queue %p  %d", buf->cookie, vb->index); */
	v_event = (struct viv_video_event *)&event.u.data[0];
	v_event->stream_id = handle->streamid;
	v_event->file = &handle->vfh;
	v_event->addr = (u64) buf->cookie;
	v_event->buf_index = vb->index;
	v_event->sync = false;
	event.type = VIV_VIDEO_EVENT_TYPE;
	event.id = VIV_VIDEO_EVENT_QBUF;
	viv_post_event(&event, &handle->vfh, false);
}

static struct vb2_ops buffer_ops = {
	.queue_setup = queue_setup,
	.buf_init = buffer_init,
	.buf_queue = buffer_queue,
	.start_streaming = start_streaming,
	.stop_streaming = stop_streaming,
};

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
	cma_free((u64) buf->cookie);
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

	buf->cookie = (void *)cma_alloc(size);
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

static int video_open(struct file *file)
{
	struct viv_video_device *dev = video_drvdata(file);
	struct viv_video_file *handle;
	int rc;

	pr_debug("enter %s\n", __func__);
	/* isp_reset(&dev->isp->ic_dev); */
	handle = kzalloc(sizeof(*handle), GFP_KERNEL);
	memset(handle, 0, sizeof(*handle));

	v4l2_fh_init(&handle->vfh, dev->video);
	v4l2_fh_add(&handle->vfh);

	file->private_data = &handle->vfh;
	handle->queue.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	handle->queue.io_modes = VB2_MMAP;
	handle->queue.drv_priv = handle;
	handle->queue.ops = &buffer_ops;
	handle->queue.mem_ops = &cma_memops;
	handle->queue.buf_struct_size = sizeof(struct vb2_dc_buf);
	handle->queue.timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC;
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 5, 0)
	handle->queue.dev = &dev->video->dev;
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

	INIT_LIST_HEAD(&handle->qlist);
	mutex_lock(&file_list_lock);
	list_add_tail(&handle->entry, &file_list_head);
	mutex_unlock(&file_list_lock);
	return 0;
}

static int video_close(struct file *file)
{
	struct viv_video_file *handle = priv_to_handle(file->private_data);
	struct vb2_buffer *vb;

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

		handle->state = 0;
		handle->streamid = 0;
		mutex_lock(&file_list_lock);
		list_del(&handle->entry);
		mutex_unlock(&file_list_lock);

		v4l2_fh_del(&handle->vfh);
		v4l2_fh_exit(&handle->vfh);

		while (!list_empty(&handle->queue.queued_list)) {
			vb = list_first_entry(&handle->queue.queued_list,
					      struct vb2_buffer, queued_entry);
			list_del(&vb->queued_entry);
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
	return v4l2_event_subscribe(fh, sub, 30, 0);
}

static int unsubscribe_event(struct v4l2_fh *fh,
			     const struct v4l2_event_subscription *sub)
{
	struct viv_video_file *handle = priv_to_handle(fh);
	struct viv_video_file *ph;

	pr_debug("enter %s\n", __func__);

	if (!handle || !sub || !fh)
		return 0;
	list_for_each_entry(ph, &file_list_head, entry) {
		if (ph == handle && handle->streamid < 0)
			return v4l2_event_unsubscribe(fh, sub);
	}
	return 0;
}

static void viv_buffer_done(struct viv_video_file *handle, u64 addr)
{
	struct vb2_buffer *vb;
	struct vb2_dc_buf *buf;

	mutex_lock(&handle->buffer_mutex);
	list_for_each_entry(vb, &handle->queue.queued_list, queued_entry) {
		if (!vb || !vb->planes)
			continue;
		buf = vb->planes[0].mem_priv;
		if (buf && (buf->cookie == (void *)addr)) {
			buf->vb.field = V4L2_FIELD_NONE;
			vb->planes[0].bytesused = handle->fmt.fmt.pix.sizeimage;
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
	int rc = 0;

	if (!file || !fh)
		return -EINVAL;

	switch (cmd) {
	case VIV_VIDIOC_EVENT_COMPLETE:
		v_event = (struct viv_video_event *)arg;
		if (v_event->file) {
			handle = priv_to_handle(v_event->file);
			mutex_lock(&file_list_lock);
			list_for_each_entry(ph, &file_list_head, entry) {
				if (ph == handle) {
					complete(&handle->wait);
					break;
				}
			}
			mutex_unlock(&file_list_lock);
		}
		break;
	case VIV_VIDIOC_BUFDONE:
		/* pr_debug("priv ioctl VIV_VIDIOC_BUFDONE\n"); */
		user_buffer = (struct v4l2_user_buffer *)arg;
		if (user_buffer->file) {
			handle = priv_to_handle(user_buffer->file);
			if (handle && handle->state == 2) {
				/* handle the stream closed unexpected. */
				mutex_lock(&file_list_lock);
				list_for_each_entry(ph, &file_list_head, entry) {
					if (ph == handle) {
						viv_buffer_done(handle,
								user_buffer->addr);
						break;
					}
				}
				mutex_unlock(&file_list_lock);
			}
		}
		break;
	case VIV_VIDIOC_S_STREAMID:
		pr_debug("priv ioctl VIV_VIDIOC_S_STREAMID\n");
		handle = priv_to_handle(file->private_data);
		handle->streamid = *((int *)arg);
		break;
	case VIV_VIDIOC_BUFFER_ALLOC:
		pr_debug("priv ioctl VIV_VIDIOC_BUFFER_ALLOC\n");
		ext_buf = (struct ext_buf_info *)arg;
		ext_buf->addr = cma_alloc(ext_buf->size);
		break;
	case VIV_VIDIOC_BUFFER_FREE:
		pr_debug("priv ioctl VIV_VIDIOC_BUFFER_FREE\n");
		ext_buf = (struct ext_buf_info *)arg;
		cma_free(ext_buf->addr);
		break;
	case VIV_VIDIOC_CONTROL_EVENT:
		pr_debug("priv ioctl VIV_VIDIOC_CONTROL_EVENT\n");
		control_event = (struct viv_control_event *)arg;
		handle = priv_to_handle(file->private_data);
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
	int ret = 0;

	pr_debug("enter %s %d %d\n", __func__, p->count, p->memory);
	if (p->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	mutex_lock(&file_list_lock);
	list_for_each_entry(ph, &file_list_head, entry) {
		if (ph->streamid == handle->streamid) {
			if (ph != handle && ph->req) {
				pr_err("stream is busy %d\n", ph->streamid);
				mutex_unlock(&file_list_lock);
				return -EBUSY;
			}
		}
	}
	mutex_unlock(&file_list_lock);

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

	if (handle->state > 0) {
		return ret;
	}
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

static const struct v4l2_ioctl_ops video_ioctl_ops = {
	.vidioc_querycap = video_querycap,
	.vidioc_enum_fmt_vid_cap = vidioc_enum_fmt_vid_cap,
	.vidioc_g_fmt_vid_cap = vidioc_g_fmt_vid_cap,
	.vidioc_try_fmt_vid_cap = vidioc_try_fmt_vid_cap,
	.vidioc_s_fmt_vid_cap = vidioc_s_fmt_vid_cap,
	.vidioc_reqbufs = vidioc_reqbufs,
	.vidioc_querybuf = vidioc_querybuf,
	.vidioc_qbuf = vidioc_qbuf,
	.vidioc_dqbuf = vidioc_dqbuf,
	.vidioc_streamon = vidioc_streamon,
	.vidioc_streamoff = vidioc_streamoff,
	.vidioc_subscribe_event = subscribe_event,
	.vidioc_unsubscribe_event = unsubscribe_event,
	.vidioc_default = private_ioctl,
	.vidioc_enum_input = vidioc_enum_input,
	.vidioc_g_input = vidioc_g_input,
	.vidioc_s_input = vidioc_s_input,
};

/* sys /dev/mem can't map large memory size */
static int isp_mmap(struct file *file, struct vm_area_struct *vma)
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

	if (vma->vm_pgoff >= ((unsigned long)RESERVED_MEM_BASE) >> PAGE_SHIFT)
		rc = isp_mmap(file, vma);
	else
		rc = vb2_mmap(&handle->queue, vma);
	return rc;
}

static unsigned int video_poll(struct file *file,
			       struct poll_table_struct *wait)
{
	struct viv_video_file *handle = priv_to_handle(file->private_data);
	int rc = 0;

	if (handle->streamid == -1) {
		poll_wait(file, &handle->vfh.wait, wait);

		if (v4l2_event_pending(&handle->vfh))
			rc = POLLIN | POLLRDNORM;
	} else {
		mutex_lock(&handle->buffer_mutex);
		rc = vb2_poll(&handle->queue, file, wait);
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
	pr_info("enter %s\n", __func__);
}
static struct platform_device viv_pdev = {
	.name = "viv_isp",
	.dev.release = pdev_release,
};

static int viv_video_probe(struct platform_device *pdev)
{
	int rc = 0;
	int i = 0;

	u64 isp_base;
	pr_debug("enter %s\n", __func__);
	vdev = kzalloc(sizeof(*vdev), GFP_KERNEL);
	if (WARN_ON(!vdev)) {
		rc = -ENOMEM;
		goto probe_end;
	}
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
	strlcpy(vdev->video->name, "viv_v4l2", sizeof(vdev->video->name));
	vdev->video->release = video_device_release;
	vdev->video->fops = &video_ops;
	vdev->video->ioctl_ops = &video_ioctl_ops;
	vdev->video->minor = -1;
	vdev->video->vfl_type = VFL_TYPE_GRABBER;
#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 0, 0)
	vdev->video->device_caps = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING;
#endif
	rc = video_register_device(vdev->video, VFL_TYPE_GRABBER, -1);
	if (WARN_ON(rc < 0))
		goto v4l2_fail;
	video_set_drvdata(vdev->video, vdev);

	pm_runtime_enable(&pdev->dev);
	pm_runtime_get_sync(&pdev->dev);

	for (; i < ISP_HW_NUMBER; i++) {
		switch (i) {
		case 0:
			isp_base = ISP_REG_BASE0;
			break;
		case 1:
			isp_base = ISP_REG_BASE1;
			break;
		}
		vdev->isp[i] =
		    isp_hw_register(vdev->v4l2_dev, isp_base, ISP_REG_SIZE);
	}

#ifdef WITH_DWE
	dwe_hw_register(vdev);
#endif
#ifdef WITH_VSE
	vse_hw_register(vdev);
#endif
#ifdef CSI_SENSOR_KERNEL
	rc = mipi_csi_sam_add(vdev->v4l2_dev);
	rc = ov2775_hw_register(vdev->v4l2_dev);
	/* rc = os08a20_hw_register(vdev->v4l2_dev); */
	/* rc = basler_hw_register(vdev->v4l2_dev); */
#endif
	rc = v4l2_device_register_subdev_nodes(vdev->v4l2_dev);
	cma_init(RESERVED_MEM_BASE, RESERVED_MEM_SIZE, 4096);
	mutex_init(&file_list_lock);

	return 0;

v4l2_fail:
	/* v4l2_device_unregister(vdev->video->v4l2_dev); */
register_fail:
	video_device_release(vdev->video);
probe_end:
	return rc;
}

static int viv_video_remove(struct platform_device *pdev)
{
	int i = 0;

	pr_debug("enter %s\n", __func__);
	if (!vdev)
		return -1;

	for (; i < ISP_HW_NUMBER; i++) {
		isp_hw_unregister(vdev->isp[i]);
	}

#ifdef WITH_DWE
	dwe_hw_unregister(vdev);
#endif
#ifdef WITH_VSE
	vse_hw_unregister(vdev);
#endif
#ifdef CSI_SENSOR_KERNEL
	mipi_csi_sam_del();
	ov2775_hw_unregister();
	/* os08a20_hw_unregister(); */
	/* basler_hw_unregister(); */
#endif

	if (vdev->video) {
		video_unregister_device(vdev->video);
		v4l2_device_disconnect(vdev->video->v4l2_dev);
		v4l2_device_put(vdev->video->v4l2_dev);
	}
	kzfree(vdev);
	vdev = NULL;
	cma_release();
	mutex_destroy(&file_list_lock);
	pm_runtime_put(&pdev->dev);
	pm_runtime_disable(&pdev->dev);

	return 0;
}

static int isp_system_suspend(struct device *dev)
{
	return pm_runtime_force_suspend(dev);;
}

static int isp_system_resume(struct device *dev)
{
	int ret;

	ret = pm_runtime_force_resume(dev);
	if (ret < 0) {
		dev_err(dev, "force resume %s failed!\n", dev_name(dev));
		return ret;
	}

	return 0;
}

static const struct dev_pm_ops isp_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(isp_system_suspend, isp_system_resume)
};

static const struct of_device_id isp_of_match[] = {
	{.compatible = "fsl,imx8mp-isp", },
	{ /* sentinel */ },
};
MODULE_DEVICE_TABLE(of, isp_of_match);

static struct platform_driver viv_video_driver = {
	.probe = viv_video_probe,
	.remove = viv_video_remove,
	.driver = {
		   .name = "viv_isp",
		   .owner = THIS_MODULE,
		   .of_match_table = isp_of_match,
		   .pm = &isp_pm_ops,
		   },
};

static int __init viv_isp_init_module(void)
{
	int ret = 0;

	pr_info("enter %s\n", __func__);

	ret = platform_driver_register(&viv_video_driver);
	if (ret) {
		pr_err("register platform driver failed.\n");
		return ret;
	}
	return ret;
}

static void __exit viv_isp_exit_module(void)
{
	pr_info("enter %s\n", __func__);
	platform_driver_unregister(&viv_video_driver);
	msleep(100);
}

module_init(viv_isp_init_module);
module_exit(viv_isp_exit_module);

MODULE_DESCRIPTION("VIV ISP driver");
MODULE_AUTHOR("Verisilicon ISP SW Team");
MODULE_LICENSE("GPL v2");
