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
# include <linux/dma-direct.h>
#endif
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>
#include <linux/version.h>
#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-ioctl.h>
#include <media/videobuf2-dma-contig.h>

#include "video.h"
#include "vvctrl.h"
#include "vvdefs.h"
#include "vvsensor.h"

#define DEF_PLANE_NO    (0)
#define VIDEO_NODE_NUM  (2)

static struct viv_video_device *vvdev[VIDEO_NODE_NUM];
static struct list_head file_list_head[VIDEO_NODE_NUM];
static spinlock_t file_list_lock[VIDEO_NODE_NUM];

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

static int set_stream(struct viv_video_device *vdev, int enable)
{
	struct v4l2_subdev *sd;
	struct media_pad *pad;

	if (!vdev)
		return -EINVAL;

	pad = &vdev->video->entity.pads[0];
	if (pad)
		pad = media_entity_remote_pad(pad);

	if (pad && is_media_entity_v4l2_subdev(pad->entity)) {
		sd = media_entity_to_v4l2_subdev(pad->entity);
		v4l2_subdev_call(sd, video, s_stream, enable);
	}
	return 0;
}

static int start_streaming(struct vb2_queue *vq, unsigned int count)
{
	struct viv_video_file *handle = queue_to_handle(vq);
	struct v4l2_fh *fh = &handle->vfh;

	pr_debug("enter %s\n", __func__);
	if (handle->streamid >= 0 && handle->state != 2) {
		handle->state = 2;
		set_stream(handle->vdev, 1);
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

	if (!handle || handle->streamid < 0 || handle->state != 2)
		return;

	handle->state = 1;
	set_stream(handle->vdev, 0);
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
	struct vb2_v4l2_buffer *vbuf;
	struct vb2_dc_buf *buf;
#ifdef ENABLE_IRQ
	struct viv_video_device *vdev;
	struct media_pad *pad;
#else
	struct v4l2_event event;
	struct viv_video_event *v_event;
#endif

	if (!vb)
		return;

	handle = queue_to_handle(vb->vb2_queue);
	vbuf = container_of(vb, struct vb2_v4l2_buffer, vb2_buf);
	buf = container_of(vbuf, struct vb2_dc_buf, vb);
	if (!buf)
		return;

	if (!handle)
		return;

#ifdef CONFIG_VIDEOBUF2_DMA_CONTIG
	buf->dma = vb2_dma_contig_plane_dma_addr(vb, DEF_PLANE_NO);
#endif

#ifdef ENABLE_IRQ
	vdev = handle->vdev;
	if (!vdev)
		return;

	pad = &vdev->video->entity.pads[0];
	vvbuf_ready(&vdev->bctx, pad, buf);
#endif

#ifndef ENABLE_IRQ
	if (handle->streamid < 0)
		return;

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

#ifdef CONFIG_VIDEOBUF2_DMA_CONTIG
	INIT_LIST_HEAD(&handle->extdmaqueue);
#endif

	spin_lock_irqsave(&file_list_lock[handle->vdev->id], flags);
	list_add_tail(&handle->entry, &file_list_head[handle->vdev->id]);
	spin_unlock_irqrestore(&file_list_lock[handle->vdev->id], flags);
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
			set_stream(handle->vdev, 0);
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
		spin_lock_irqsave(&file_list_lock[handle->vdev->id], flags);
		list_del(&handle->entry);
		spin_unlock_irqrestore(&file_list_lock[handle->vdev->id], flags);

		v4l2_fh_del(&handle->vfh);
		v4l2_fh_exit(&handle->vfh);

#ifdef CONFIG_VIDEOBUF2_DMA_CONTIG
		{
			struct ext_dma_buf *edb = NULL;
			while (!list_empty(&handle->extdmaqueue)) {
				edb = list_first_entry(&handle->extdmaqueue,
						struct ext_dma_buf, entry);
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
	int ret;
	unsigned long flags;
	struct viv_video_file *handle = priv_to_handle(fh);

	if (!handle || !sub)
		return -EINVAL;
	if (sub->type == VIV_VIDEO_EVENT_TYPE)
	{
		ret = v4l2_event_subscribe(fh, sub, 10, 0);
		if(ret == 0 && handle->vdev &&
			sub->id == VIV_VIDEO_EVENT_GET_CAPS_SUPPORTS)
		{
			spin_lock_irqsave(&file_list_lock[handle->vdev->id], flags);
			handle->vdev->subscribed_cnt++;
			spin_unlock_irqrestore(&file_list_lock[handle->vdev->id], flags);
			complete_all(&handle->vdev->subscribed_wait);
		}
		return ret;
	}
	else
		return v4l2_ctrl_subscribe_event(fh, sub);
}

static int unsubscribe_event(struct v4l2_fh *fh,
				 const struct v4l2_event_subscription *sub)
{
	struct viv_video_file *handle = priv_to_handle(fh);
	struct viv_video_file *ph;
	unsigned long flags;

	pr_debug("enter %s\n", __func__);
	if (!handle || !sub || !fh)
		return 0;
	if (sub->type == VIV_VIDEO_EVENT_TYPE) {
		spin_lock_irqsave(&file_list_lock[handle->vdev->id], flags);
		list_for_each_entry(ph, &file_list_head[handle->vdev->id], entry) {
			if (ph == handle && handle->streamid < 0) {
				if(sub->id == VIV_VIDEO_EVENT_GET_CAPS_SUPPORTS)
				{
					handle->vdev->subscribed_cnt--;
					if(handle->vdev->subscribed_cnt <= 0)
					{
						reinit_completion(&handle->vdev->subscribed_wait);
						handle->vdev->subscribed_cnt = 0;
					}
				}
				spin_unlock_irqrestore(&file_list_lock[handle->vdev->id], flags);
				return v4l2_event_unsubscribe(fh, sub);
			}
		}
		spin_unlock_irqrestore(&file_list_lock[handle->vdev->id], flags);
	} else {
		return v4l2_event_unsubscribe(fh, sub);
	}
	return 0;
}

#ifndef ENABLE_IRQ
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
#endif

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
	
	if (dev->subscribed_cnt == 0 && wait_for_completion_timeout(&dev->subscribed_wait, 
			msecs_to_jiffies(VIV_VIDEO_EVENT_TIMOUT_MS)) == 0)
		return -ETIMEDOUT;
	return viv_post_event(&event, &handle->vfh, true);
}

static struct media_entity *viv_find_entity(struct viv_video_device *dev,
				const char *name)
{
	struct v4l2_subdev *sd = NULL;
	int i;

	for (i = 0; i < dev->sdcount; ++i) {
		if (!strncmp(dev->subdevs[i]->name, name, strlen(name))) {
			sd = dev->subdevs[i];
			break;
		}
	}
	return sd ? &sd->entity : NULL;
}

static int viv_create_link(struct media_entity *source, u16 source_pad,
				 struct media_entity *sink, u16 sink_pad)
{
	int rc = 0;
	u32 flags = MEDIA_LNK_FL_ENABLED;

	if (!source || !sink)
		return -EINVAL;
	rc = media_create_pad_link(source, source_pad, sink, sink_pad, flags);
	if (rc)
		goto end;

	rc = media_entity_call(sink, link_setup, &sink->pads[sink_pad],
			&source->pads[source_pad], flags);
	if (rc)
		goto end;

	rc = media_entity_call(source, link_setup, &source->pads[source_pad],
			&sink->pads[sink_pad], flags);
	if (rc)
		goto end;
end:
	return rc;
}

static int viv_create_default_links(struct viv_video_device *dev)
{
	struct media_entity *source, *sink;
	const char *devname = ISP_DEVICE_NAME;
	u32 source_pad = ISP_PAD_SOURCE;

	if (dev->id == 0 && dev->sdcount > 1) {
		source = viv_find_entity(dev, ISP_DEVICE_NAME);
		sink = viv_find_entity(dev, DWE_DEVICE_NAME);
		if (viv_create_link(source, ISP_PAD_SOURCE, sink, DWE_PAD_SINK))
			pr_err("failed to create link between isp and dwe!\n");

		devname = DWE_DEVICE_NAME;
		source_pad = DWE_PAD_SOURCE;
	}

	source = viv_find_entity(dev, devname);
	sink = &dev->video->entity;
	return viv_create_link(source, source_pad, sink, 0);
}

static int viv_config_dwe(struct viv_video_file *handle, bool enable)
{
	struct viv_video_device *vdev = handle->vdev;
	int rc;
	struct media_entity *source, *sink;

	if (!vdev || !vdev->sdcount)
		return -EINVAL;

	if (vdev->dweEnabled == enable)
		return 0;

	source = viv_find_entity(vdev, DWE_DEVICE_NAME);
	if (!source)
		return -EINVAL;

	if (!enable) {
		rc = viv_create_default_links(vdev);
		if (!rc)
			vdev->dweEnabled = false;
		return rc;
	}

	sink = &vdev->video->entity;
	rc = viv_create_link(source, DWE_PAD_SOURCE, sink, 0);
	if (rc)
		goto end;

	if (vdev->sdcount > 1) {
		source = viv_find_entity(vdev, ISP_DEVICE_NAME);
		sink = viv_find_entity(vdev, DWE_DEVICE_NAME);
		if (viv_create_link(source, ISP_PAD_SOURCE, sink, DWE_PAD_SINK))
			pr_err("failed to create link between isp and dwe!\n");
	}

	vdev->dweEnabled = true;
end:
	return rc;
}

static long private_ioctl(struct file *file, void *fh,
			  bool valid_prio, unsigned int cmd, void *arg)
{
	struct viv_video_file *handle;
	struct viv_video_file *ph;
	struct viv_video_event *v_event;
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
			spin_lock_irqsave(&file_list_lock[handle->vdev->id], flags);
			list_for_each_entry(ph, &file_list_head[handle->vdev->id], entry) {
				if (ph == handle) {
					complete(&handle->wait);
					break;
				}
			}
			spin_unlock_irqrestore(&file_list_lock[handle->vdev->id], flags);
		} else {
			complete(&handle->vdev->ctrls.wait);
		}
		break;
#ifndef ENABLE_IRQ
	case VIV_VIDIOC_BUFDONE: {
		/* pr_debug("priv ioctl VIV_VIDIOC_BUFDONE\n"); */
		struct v4l2_user_buffer *user_buffer =
				(struct v4l2_user_buffer *)arg;
		if (user_buffer->file) {
			handle = priv_to_handle(user_buffer->file);
			if (handle && handle->state == 2) {
				/* handle the stream closed unexpected. */
				spin_lock_irqsave(&file_list_lock[handle->vdev->id], flags);
				list_for_each_entry(ph, &file_list_head[handle->vdev->id], entry) {
					if (ph == handle) {
						viv_buffer_done(handle,
								user_buffer->addr);
						break;
					}
				}
				spin_unlock_irqrestore(&file_list_lock[handle->vdev->id], flags);
			}
		}
		break;
	}
#endif
	case VIV_VIDIOC_S_STREAMID:
		pr_debug("priv ioctl VIV_VIDIOC_S_STREAMID\n");
		handle->streamid = *((int *)arg);
		break;
	case VIV_VIDIOC_S_DWECFG:
		viv_config_dwe(handle, !!*((int *)arg));
		break;
	case VIV_VIDIOC_G_DWECFG:
		*((int *)arg) = handle->vdev->dweEnabled ? 1 : 0;
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
	unsigned long flags;
	int ret;

	pr_debug("enter %s\n", __func__);
	if (f->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	ret = vidioc_try_fmt_vid_cap(file, priv, f);

	if (ret < 0)
		return -EINVAL;

	spin_lock_irqsave(&file_list_lock[handle->vdev->id], flags);
	list_for_each_entry(ph, &file_list_head[handle->vdev->id], entry) {
		if (ph->streamid == handle->streamid) {
			memcpy(&ph->fmt, f, sizeof(*f));
		}
	}
	spin_unlock_irqrestore(&file_list_lock[handle->vdev->id], flags);
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

	spin_lock_irqsave(&file_list_lock[handle->vdev->id], flags);
	list_for_each_entry(ph, &file_list_head[handle->vdev->id], entry) {
		if (ph->streamid == handle->streamid) {
			if (ph != handle && ph->req) {
				pr_err("stream is busy %d\n", ph->streamid);
				spin_unlock_irqrestore(&file_list_lock[handle->vdev->id], flags);
				return -EBUSY;
			}
		}
	}
	spin_unlock_irqrestore(&file_list_lock[handle->vdev->id], flags);

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

	reinit_completion(&vdev->ctrls.wait);

	v4l2_event_queue(vdev->video, &event);

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

	reinit_completion(&vdev->ctrls.wait);

	v4l2_event_queue(vdev->video, &event);

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

		reinit_completion(&vdev->ctrls.wait);

		v4l2_event_queue(vdev->video, &event);

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

static int viv_notifier_bound(struct v4l2_async_notifier *notifier,
				struct v4l2_subdev *sd, struct v4l2_async_subdev *asd)
{
	int i;
	struct viv_video_device *dev = container_of(notifier,
			struct viv_video_device, subdev_notifier);

	if (!dev)
		return 0;

	for (i = 0; i < dev->asdcount; ++i) {
		if (sd->dev && dev->asd[i].match.fwnode ==
				of_fwnode_handle(sd->dev->of_node)) {
			dev->subdevs[dev->sdcount] = sd;
			dev->sdcount++;
			break;
		}
	}
	return 0;
}

static int viv_notifier_complete(struct v4l2_async_notifier *notifier)
{
	struct viv_video_device *dev = container_of(notifier,
			struct viv_video_device, subdev_notifier);
	int rc;

	if (!dev)
		return 0;

	if (dev->sdcount > 0) {
		mutex_lock(&dev->mdev.graph_mutex);
		rc = viv_create_default_links(dev);
		mutex_unlock(&dev->mdev.graph_mutex);

		if (rc) {
			pr_err("failed to create media links!\n");
			return rc;
		}
	}

	rc = v4l2_device_register_subdev_nodes(dev->v4l2_dev);
	if (!rc)
		rc = media_device_register(&dev->mdev);
	return rc;
}

static const struct v4l2_async_notifier_operations sd_async_notifier_ops = {
	.bound = viv_notifier_bound,
	.complete = viv_notifier_complete,
};

static int viv_mdev_link_notify(struct media_link *link, unsigned int flags,
				unsigned int notification)
{
	return 0;
}

static const struct media_device_ops viv_mdev_ops = {
	.link_notify = viv_mdev_link_notify,
};

static int viv_link_setup(struct media_entity *entity,
				const struct media_pad *local,
				const struct media_pad *remote, u32 flags)
{
	return 0;
}

static const struct media_entity_operations viv_media_ops = {
	.link_setup = viv_link_setup,
	.link_validate = v4l2_subdev_link_validate,
};

#ifdef ENABLE_IRQ
static void viv_buf_notify(struct vvbuf_ctx *ctx, struct vb2_dc_buf *buf)
{
	struct viv_video_file *fh;
	struct viv_video_device *vdev;
	u64 cur_ts, interval;
	u32 fps;
    static u32 loop_cnt = 0;

	if (!buf || buf->vb.vb2_buf.state != VB2_BUF_STATE_ACTIVE)
		return;

	fh = container_of(buf->vb.vb2_buf.vb2_queue,
			struct viv_video_file, queue);
	vdev = fh->vdev;
	buf->vb.vb2_buf.planes[DEF_PLANE_NO].bytesused =
			fh->fmt.fmt.pix.sizeimage;
	cur_ts = ktime_get_ns();
#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 0, 0)
	buf->vb.vb2_buf.timestamp = cur_ts;
#endif
	vb2_buffer_done(&buf->vb.vb2_buf, VB2_BUF_STATE_DONE);

	/* print fps info for debugging purpose */
	interval = (cur_ts - vdev->last_ts) / 1000000; /*ms*/
	if (vdev->duration >= 3 * 1000/*ms*/) {
		loop_cnt++;
		if (loop_cnt >= 10) {
			fps = vdev->frame_count * 100000 / vdev->duration;
			pr_info("###### video%d(%d) %d.%02d fps ######\n",
					vdev->video->num, vdev->id,
					fps / 100, fps % 100);
			loop_cnt = 0;
		}
		vdev->frame_count = 0;
		vdev->duration = 0;
	} else if (interval > 0) {
		vdev->frame_count++;
		vdev->duration += interval;
	}
	vdev->last_ts = cur_ts;
}

static const struct vvbuf_ops viv_buf_ops = {
	.notify = viv_buf_notify,
};
#endif

struct dev_node {
	struct device_node *node;
	int id;
};

#ifdef ENABLE_IRQ
static inline int viv_find_compatible_nodes(struct dev_node *nodes, int size)
{
	static const char *compat_name[] = {ISP_COMPAT_NAME,DWE_COMPAT_NAME};
	struct device_node *node, *avail;
	int i, rc, id, cnt = 0;

	for (i = 0; i < ARRAY_SIZE(compat_name) && cnt < size; ++i) {
		node = NULL;
		for (;cnt < size;) {
			node = of_find_compatible_node(node, NULL, compat_name[i]);
			if (!node)
				break;
			avail = NULL;
			id = -1;
			rc = fwnode_property_read_u32(of_fwnode_handle(node), "id", &id);
			if (rc) {
				if (of_device_is_available(node))
					avail = node;
			} else if (of_device_is_available(node))
				avail = node;

			if (avail) {
				nodes[cnt].node = avail;
				nodes[cnt].id = id;
				cnt++;
			}
		}
	}
	return cnt;
}
#endif

static int viv_video_probe(struct platform_device *pdev)
{
	struct viv_video_device *vdev;
	int rc = 0;
	int i;
#ifdef ENABLE_IRQ
	int j;
	struct dev_node nodes[MAX_SUBDEVS_NUM];
	int nodecount = viv_find_compatible_nodes(nodes, MAX_SUBDEVS_NUM);
#endif

	for (i = 0; i < VIDEO_NODE_NUM; i++) {
		spin_lock_init(&file_list_lock[i]);
		INIT_LIST_HEAD(&file_list_head[i]);

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
		init_completion(&vdev->subscribed_wait);
		vdev->subscribed_cnt = 0;
		vdev->video = video_device_alloc();
		vdev->video->v4l2_dev = vdev->v4l2_dev;
		rc = v4l2_device_register(&pdev->dev, vdev->video->v4l2_dev);
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
#ifdef ENABLE_IRQ
		video_set_drvdata(vdev->video, vdev);

		vvbuf_ctx_init(&vdev->bctx);
		vdev->bctx.ops = &viv_buf_ops;

		snprintf(vdev->mdev.model, sizeof(vdev->mdev.model)-1,
				"viv_media%d", i);
		vdev->mdev.ops = &viv_mdev_ops;
		vdev->mdev.dev = &pdev->dev;
		vdev->v4l2_dev->mdev = &vdev->mdev;

		media_device_init(&vdev->mdev);

		vdev->video->entity.name = vdev->video->name;
		vdev->video->entity.obj_type = MEDIA_ENTITY_TYPE_VIDEO_DEVICE;
		vdev->video->entity.function = MEDIA_ENT_F_IO_V4L;
		vdev->video->entity.ops = &viv_media_ops;

		vdev->pad.flags = MEDIA_PAD_FL_SINK | MEDIA_PAD_FL_MUST_CONNECT;
		rc = media_entity_pads_init(&vdev->video->entity, 1, &vdev->pad);
		if (WARN_ON(rc < 0))
			goto register_fail;

		v4l2_async_notifier_init(&vdev->subdev_notifier);
		vdev->subdev_notifier.ops = &sd_async_notifier_ops;
#endif
		rc = video_register_device(vdev->video, VFL_TYPE_GRABBER, -1);
		if (WARN_ON(rc < 0))
			goto register_fail;

#ifdef ENABLE_IRQ
		for (j = 0; j < nodecount; ++j) {
			if (nodes[j].id == i) {
				vdev->asd[vdev->asdcount].match_type =
						V4L2_ASYNC_MATCH_FWNODE;
				vdev->asd[vdev->asdcount].match.fwnode =
						of_fwnode_handle(nodes[j].node);
				v4l2_async_notifier_add_subdev(&vdev->subdev_notifier,
						&vdev->asd[vdev->asdcount]);
				vdev->asdcount++;
			}
		}

		rc = v4l2_async_notifier_register(vdev->v4l2_dev,
				&vdev->subdev_notifier);
		if (WARN_ON(rc < 0))
			goto register_fail;
#else
		video_set_drvdata(vdev->video, vdev);

		rc = v4l2_device_register_subdev_nodes(vdev->v4l2_dev);
#endif

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

	for (i = VIDEO_NODE_NUM-1; i >=0; i--) {
		vdev = vvdev[i];
		if (!vdev || !vdev->video)
			continue;
#ifdef ENABLE_IRQ
		media_entity_cleanup(&vdev->video->entity);
		media_device_unregister(&vdev->mdev);
		media_device_cleanup(&vdev->mdev);
		v4l2_async_notifier_unregister(&vdev->subdev_notifier);
		v4l2_device_unregister(vdev->v4l2_dev);
#endif
		video_unregister_device(vdev->video);
#ifdef ENABLE_IRQ
		vvbuf_ctx_deinit(&vdev->bctx);
#else
		v4l2_device_disconnect(vdev->video->v4l2_dev);
		v4l2_device_put(vdev->video->v4l2_dev);
#endif

		kfree(vdev->ctrls.buf_va);
		v4l2_ctrl_handler_free(&vdev->ctrls.handler);
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
