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
#include <linux/of_reserved_mem.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include "video.h"
#include "vvctrl.h"
#include "vvdefs.h"
#include "vvsensor.h"

#define DEF_PLANE_NO    (0)

static struct viv_video_device *vvdev[VIDEO_NODE_NUM];
static struct list_head file_list_head[VIDEO_NODE_NUM];
static spinlock_t file_list_lock[VIDEO_NODE_NUM];
static struct media_device mdev;

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

/* Caller must hold fh->vdev->fh_lock! */
static struct v4l2_subscribed_event *video_event_fh_subscribed(
		struct v4l2_fh *fh, u32 type, u32 id)
{
	struct v4l2_subscribed_event *sev;

	assert_spin_locked(&fh->vdev->fh_lock);

	list_for_each_entry(sev, &fh->subscribed, list)
		if (sev->type == type && sev->id == id) {
			return sev;
		}

	return NULL;
}

/*0-unsubscribed  1-subscribed*/
static int video_event_subscribed(struct video_device *vdev,
		const struct v4l2_event *event)
{
	struct v4l2_fh *fh;
	unsigned long flags;

	spin_lock_irqsave(&vdev->fh_lock, flags);

	list_for_each_entry(fh, &vdev->fh_list, list) {
		if(video_event_fh_subscribed(fh, event->type, event->id)) {
			spin_unlock_irqrestore(&vdev->fh_lock, flags);
			return 1;
		}
	}
	spin_unlock_irqrestore(&vdev->fh_lock, flags);

	return 0;
}

static int viv_event_wait_complete(struct viv_video_device *vdev,
								   struct v4l2_event *event)
{
	int timeout_ms = 1000;
	int i = 0;

	for (i = 0; i < timeout_ms; i++) {
		if (vdev->event_shm.complete)
			return 0;
		if (video_event_subscribed(vdev->video, event) == true) {
			msleep(1);
		} else {
			return -EINVAL;
		}

	}

	return -ETIMEDOUT;
}

static int viv_v4l2_post_event(struct viv_video_device *vdev,
		struct v4l2_event *event, bool sync)
{
	int ret = 0;
	int i = 0;

	if ((event->id != VIV_VIDEO_EVENT_DEL_STREAM) &&
		(event->id != VIV_VIDEO_EVENT_STOP_STREAM)) {
		for (i = 0; i < 10; i++) {
			if (video_event_subscribed(vdev->video, event) == true) {
				break;
			} else {
				msleep(2);
			}
		}
	}

	mutex_lock(&vdev->event_lock);

	if (sync) {
		vdev->event_shm.complete = 0;
		vdev->event_shm.result = 0;
	}

	v4l2_event_queue(vdev->video, event);

	if (sync) {
		ret = viv_event_wait_complete(vdev, event);
	}

	mutex_unlock(&vdev->event_lock);

	return ret;
}

static int viv_post_event(struct v4l2_event *event, void *fh, bool sync)
{
	struct viv_video_file *handle = priv_to_handle(fh);
	struct v4l2_fh *video_fh = (struct v4l2_fh *)fh;

	if(!video_fh || !event) {
		return -EINVAL;
	}


	return viv_v4l2_post_event(handle->vdev, event, sync);
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
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 19, 0)
		pad = media_entity_remote_pad(pad);
#else
		pad = media_pad_remote_pad_first(pad);
#endif
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
		handle->vdev->active = 1;
		set_stream(handle->vdev, 1);
		viv_post_simple_event(VIV_VIDEO_EVENT_START_STREAM,
				      handle->streamid, fh, true);
		handle->vdev->pipeline_status = PIPELINE_STREAMON;
		handle->vdev->frame_cnt = 0;
		handle->vdev->last_ts = 0;
	} else {
		pr_err("can't start streaming, device busy!\n");
		return -EBUSY;
	}
	return 0;
}

static void stop_streaming(struct vb2_queue *vq)
{
	struct viv_video_file *handle = queue_to_handle(vq);
	int i;

	pr_debug("enter %s\n", __func__);

	if (!handle || handle->streamid < 0 || handle->state != 2)
		return;

	handle->state = 1;
	handle->vdev->pipeline_status = PIPELINE_STREAMOFF;
	set_stream(handle->vdev, 0);
	viv_post_simple_event(VIV_VIDEO_EVENT_STOP_STREAM, handle->streamid,
			      &handle->vfh, false);
	handle->sequence = 0;
	for(i = 0; i < vq->num_buffers; i++) {
		if(vq->bufs[i]->state == VB2_BUF_STATE_ACTIVE)
			vb2_buffer_done(vq->bufs[i], VB2_BUF_STATE_ERROR);
	}
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0)
static int queue_setup(struct vb2_queue *vq, const struct v4l2_format *fmt,
		       unsigned int *nbuffers, unsigned int *nplanes,
		       unsigned int sizes[], void *alloc_ctxs[])
{
	struct viv_video_file *handle = queue_to_handle(vq);
	unsigned long size = handle->vdev->fmt.fmt.pix.sizeimage;

	pr_debug("enter %s\n", __func__);
	if (*nbuffers == 0)
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
	unsigned long size = handle->vdev->fmt.fmt.pix.sizeimage;

	pr_debug("enter %s\n", __func__);
	if (*num_buffers == 0)
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
	struct viv_video_device *vdev;
	struct media_pad *pad;

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

	vdev = handle->vdev;
	if (!vdev)
		return;

	if ((vdev->dumpbuf_status == DUMPBUF_ENABLE) && (vdev->dumpbuf == NULL)){
		vdev->dumpbuf = buf;
		vdev->dumpbuf_status = DUMPBUF_DONE;
	}
	else {
		pad = &vdev->video->entity.pads[0];
		vvbuf_ready(&vdev->bctx, pad, buf);

		if ((vdev->dumpbuf_status == DUMPBUF_DISABLE) && vdev->dumpbuf) {
			vvbuf_ready(&vdev->bctx, pad, vdev->dumpbuf);
			vdev->dumpbuf = NULL;
		}
	}
}

static struct vb2_ops buffer_ops = {
	.queue_setup = queue_setup,
	.buf_init = buffer_init,
	.buf_queue = buffer_queue,
	.start_streaming = start_streaming,
	.stop_streaming = stop_streaming,
};

static int video_open(struct file *file)
{
	struct viv_video_device *dev = video_drvdata(file);
	struct viv_video_file *handle;
	unsigned long flags;
	int rc;

	pr_debug("enter %s\n", __func__);
	handle = kzalloc(sizeof(*handle), GFP_KERNEL);

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
		kfree(handle);
		return rc;
	}
	mutex_init(&handle->buffer_mutex);
	init_completion(&handle->wait);

#ifdef CONFIG_VIDEOBUF2_DMA_CONTIG
	INIT_LIST_HEAD(&handle->extdmaqueue);
#endif

	spin_lock_irqsave(&file_list_lock[handle->vdev->id], flags);
	list_add_tail(&handle->entry, &file_list_head[handle->vdev->id]);
	spin_unlock_irqrestore(&file_list_lock[handle->vdev->id], flags);
	atomic_inc(&(handle->vdev->refcnt));
	return 0;
}

static int video_close(struct file *file)
{
	struct viv_video_file *handle = priv_to_handle(file->private_data);
	struct viv_video_device *vdev = video_drvdata(file);
	struct vb2_buffer *vb;
	spinlock_t *lock;
	unsigned long flags;

	atomic_inc(&(vdev->refcnt));
	pr_debug("enter %s\n", __func__);
	if (handle) {
		handle->req = false;
		if (handle->streamid >= 0 && handle->state == 2) {
			set_stream(handle->vdev, 0);
			viv_post_simple_event(VIV_VIDEO_EVENT_STOP_STREAM,
						handle->streamid, &handle->vfh,
						true);
			handle->state = 1;
			handle->vdev->pipeline_status = PIPELINE_STREAMOFF;
		}
		if (handle->streamid >= 0 && handle->state == 1) {
			viv_post_simple_event(VIV_VIDEO_EVENT_DEL_STREAM,
						handle->streamid, &handle->vfh,
						true);
			handle->vdev->pipeline_status = PIPELINE_INIT;
		}

		if (handle->state > 0)
			handle->vdev->active = 0;
		handle->state = -1;
		handle->streamid = 0;
		lock = &file_list_lock[handle->vdev->id];
		spin_lock_irqsave(lock, flags);
		list_del(&handle->entry);
		spin_unlock_irqrestore(lock, flags);

		list_for_each_entry(vb, &handle->queue.queued_list,
				queued_entry) {
			if (vb->state == VB2_BUF_STATE_ACTIVE)
				vb2_buffer_done(vb, VB2_BUF_STATE_ERROR);
		}

		v4l2_fh_del(&handle->vfh);
		v4l2_fh_exit(&handle->vfh);

#ifdef CONFIG_VIDEOBUF2_DMA_CONTIG
		{
			struct ext_dma_buf *edb = NULL;

			while (!list_empty(&handle->extdmaqueue)) {
				edb = list_first_entry(&handle->extdmaqueue,
						struct ext_dma_buf, entry);
				if (edb) {
					dma_free_attrs(handle->queue.dev,
							edb->size, edb->vaddr,
							edb->addr,
							DMA_ATTR_WRITE_COMBINE);
					list_del(&edb->entry);
					kfree(edb);
				}
			}
		}
#endif

		vb2_queue_release(&handle->queue);
		mutex_destroy(&handle->buffer_mutex);
		kfree(handle);
		handle = NULL;
	}

	atomic_dec(&(vdev->refcnt));
	if (atomic_read(&(vdev->refcnt)) == 0) {
		reinit_completion(&vdev->subscribed_wait);
		vdev->subscribed_cnt = 0;
	}

	return 0;
}

static int subscribe_event(struct v4l2_fh *fh,
			   const struct v4l2_event_subscription *sub)
{
	int ret;
	unsigned long flags;
	struct viv_video_file *handle = priv_to_handle(fh);
	struct viv_video_device *vdev;

	if (!handle || !sub)
		return -EINVAL;
	if (unlikely(sub->type != VIV_VIDEO_EVENT_TYPE))
		return v4l2_ctrl_subscribe_event(fh, sub);
	ret = v4l2_event_subscribe(fh, sub, 10, 0);
	vdev = handle->vdev;
	if (!ret && vdev && sub->id == VIV_VIDEO_EVENT_GET_CAPS_SUPPORTS) {
		spin_lock_irqsave(&file_list_lock[vdev->id], flags);
		vdev->subscribed_cnt++;
		spin_unlock_irqrestore(&file_list_lock[vdev->id], flags);
		complete_all(&vdev->subscribed_wait);
	}
	return ret;
}

static int unsubscribe_event(struct v4l2_fh *fh,
				 const struct v4l2_event_subscription *sub)
{
	struct viv_video_file *handle = priv_to_handle(fh);
	struct viv_video_file *ph;
	struct viv_video_device *vdev;
	spinlock_t *lock;
	unsigned long flags;

	pr_debug("enter %s\n", __func__);
	if (!handle || !handle->vdev || !sub)
		return 0;
	if (unlikely(sub->type != VIV_VIDEO_EVENT_TYPE))
		return v4l2_event_unsubscribe(fh, sub);

	vdev = handle->vdev;
	lock = &file_list_lock[vdev->id];
	spin_lock_irqsave(lock, flags);
	list_for_each_entry(ph, &file_list_head[vdev->id], entry) {
		if (ph == handle && handle->streamid < 0) {
			if (sub->id == VIV_VIDEO_EVENT_GET_CAPS_SUPPORTS) {
				vdev->subscribed_cnt--;
				if (vdev->subscribed_cnt <= 0) {
					reinit_completion(
							&vdev->subscribed_wait);
					vdev->subscribed_cnt = 0;
				}
			}
			spin_unlock_irqrestore(lock, flags);
			return v4l2_event_unsubscribe(fh, sub);
		}
	}
	spin_unlock_irqrestore(lock, flags);
	return 0;
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

	if (dev->subscribed_cnt == 0 &&
			wait_for_completion_timeout(&dev->subscribed_wait,
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

	source = viv_find_entity(dev, ISP_DEVICE_NAME);
	sink = &dev->video->entity;
	return viv_create_link(source, ISP_PAD_SOURCE, sink, 0);
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
	sink = &vdev->video->entity;
	media_entity_remove_links(source);
	media_entity_remove_links(sink);

	if (!enable) {
		source = viv_find_entity(vdev, ISP_DEVICE_NAME);
		media_entity_remove_links(source);
		rc = viv_create_link(source, ISP_PAD_SOURCE, sink, 0);
		if (!rc)
			vdev->dweEnabled = false;
		return rc;
	}

	rc = viv_create_link(source, DWE_PAD_SOURCE, sink, 0);
	if (rc)
		goto end;

	if (vdev->sdcount > 1) {
		source = viv_find_entity(vdev, ISP_DEVICE_NAME);
		sink = viv_find_entity(vdev, DWE_DEVICE_NAME);
		media_entity_remove_links(source);
		if (viv_create_link(source, ISP_PAD_SOURCE, sink, DWE_PAD_SINK))
			pr_err("failed to create link between isp and dwe!\n");
	}

	vdev->dweEnabled = true;
end:
	return rc;
}

static inline void init_v4l2_fmt(struct v4l2_format *f, unsigned int bpp,
		    unsigned int depth, unsigned int *bytesperline,
		    unsigned int *sizeimage)
{
	v4l_bound_align_image(&f->fmt.pix.width,
		VIDEO_FRAME_MIN_WIDTH, VIDEO_FRAME_MAX_WIDTH, 0,
		&f->fmt.pix.height, VIDEO_FRAME_MIN_HEIGHT,
		VIDEO_FRAME_MAX_HEIGHT, 0, 0);
	*bytesperline = (f->fmt.pix.width) * bpp;
	*sizeimage = (f->fmt.pix.width) * (f->fmt.pix.height) * depth / 8;
	return;
}

static int viv_set_modeinfo(struct viv_video_file *handle,
			  struct vvcam_constant_modeinfo *pcamera_mode)
{
	struct viv_video_device *vdev = handle->vdev;
	struct viv_video_fmt fmt;
	struct viv_video_fmt *pfmt = NULL;

	if (pcamera_mode->size.width == 0 || pcamera_mode->size.height == 0 ) {
		vdev->camera_status = 0;
		return -EINVAL;
	}

	vdev->camera_status = 1;
	memcpy(&vdev->camera_mode, pcamera_mode, sizeof(struct vvcam_constant_modeinfo));

	memcpy(vdev->formats, formats, sizeof(formats));
	vdev->formatscount = ARRAY_SIZE(formats);
	if (bayer_pattern_to_format(pcamera_mode->bayer_pattern, pcamera_mode->bit_width, &fmt) == 0) {
		memcpy(&vdev->formats[vdev->formatscount], &fmt, sizeof(fmt));
		vdev->formatscount++;
	}

	pfmt = &vdev->formats[0];

	vdev->fmt.fmt.pix.width = vdev->camera_mode.size.width;
	vdev->fmt.fmt.pix.height = vdev->camera_mode.size.height;
	vdev->fmt.fmt.pix.pixelformat = pfmt->fourcc;
	vdev->timeperframe.numerator = 1;

	vdev->timeperframe.denominator = vdev->camera_mode.fps;
	init_v4l2_fmt(&vdev->fmt, pfmt->bpp, pfmt->depth,
				&vdev->fmt.fmt.pix.bytesperline,
				&vdev->fmt.fmt.pix.sizeimage);

	vdev->crop.width = vdev->camera_mode.size.width;
	vdev->crop.height = vdev->camera_mode.size.height;
	vdev->compose.width = vdev->camera_mode.size.width;
	vdev->compose.height = vdev->camera_mode.size.height;

	return 0;
}

static long private_ioctl(struct file *file, void *fh,
			  bool valid_prio, unsigned int cmd, void *arg)
{
	struct viv_video_file *handle;
	struct ext_buf_info *ext_buf;
	struct viv_control_event *control_event;
	struct viv_caps_supports *pcaps_supports;
	struct viv_video_device *dev = video_drvdata(file);
	int rc = 0;
	struct reserved_mem *rmem;

	if (!file || !fh)
		return -EINVAL;

	handle = priv_to_handle(file->private_data);
	if (!handle || handle->state == -1) {
		pr_err("call ioctl after file closed\n");
		return -EINVAL;
	}

	switch (cmd) {
	case VIV_VIDIOC_EVENT_COMPLETE:
		dev->event_shm.complete = 1;
		break;
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
#ifdef CONFIG_VIDEOBUF2_DMA_CONTIG
		struct ext_buf_info *ext_buf = (struct ext_buf_info *)arg;
		struct ext_dma_buf *edb = kzalloc(sizeof(*edb), GFP_KERNEL);

		pr_debug("priv ioctl VIV_VIDIOC_BUFFER_ALLOC\n");
		if (!edb) {
			rc = -ENOMEM;
			break;
		}
		edb->vaddr = dma_alloc_coherent(handle->queue.dev,
				ext_buf->size, &(ext_buf->addr), GFP_KERNEL);
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
#ifdef CONFIG_VIDEOBUF2_DMA_CONTIG
		struct ext_buf_info *ext_buf = (struct ext_buf_info *)arg;
		struct ext_dma_buf *b, *edb = NULL;

		pr_debug("priv ioctl VIV_VIDIOC_BUFFER_FREE\n");
		list_for_each_entry(b, &handle->extdmaqueue, entry) {
			if (b->addr == ext_buf->addr) {
				edb = b;
				break;
			}
		}

		if (edb) {
			dma_free_coherent(handle->queue.dev, edb->size,
			edb->vaddr, edb->addr);
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
		rmem = (struct reserved_mem *)dev->rmem;
		if (!rmem) {
			ext_buf->addr = 0;
			ext_buf->size = 0;
		} else {
			ext_buf->addr = rmem->base;
			ext_buf->size = rmem->size;
		}
		break;
	case VIV_VIDIOC_S_MODEINFO:
		rc = viv_set_modeinfo(handle, arg);
		break;

	case VIV_VIDIOC_S_CAPS_MODE:
		memcpy(&(dev->caps_mode), arg, sizeof(dev->caps_mode));
		rc = set_caps_mode_event(file);
		if (rc == 0)
			rc = dev->event_result;
		break;

	case VIV_VIDIOC_G_CAPS_MODE:
		memcpy(arg, &(dev->caps_mode), sizeof(dev->caps_mode));
		break;

	case VIV_VIDIOC_EVENT_RESULT:
		dev->event_result = *(int *)arg;
		break;

	case VIV_VIDIOC_GET_CAPS_SUPPORTS:{
		pcaps_supports = (struct viv_caps_supports *)arg;
		rc = get_caps_suppots_event(file);
		memcpy(pcaps_supports, &(dev->caps_supports),
				sizeof(dev->caps_supports));
		break;
	}

	case VIV_VIDIOC_SET_CAPS_SUPPORTS:{
		pcaps_supports = (struct viv_caps_supports *)arg;
		memcpy(&(dev->caps_supports), arg, sizeof(dev->caps_supports));
		break;
	}

	case VIV_VIDIOC_S_DUMPBUF_STATUS: {
		dev->dumpbuf_status = *(int *)arg;
		break;
	}

	case VIV_VIDIOC_G_DUMPBUF_STATUS: {
		*(int *)arg = dev->dumpbuf_status;
		break;
	}

	case VIV_VIDIOC_DUMPBUF: {
		struct viv_caps_dump_buf_s *dump_buf = (struct viv_caps_dump_buf_s *)arg;
		if (dev->dumpbuf != NULL) {
			dump_buf->offset = dev->dumpbuf->vb.planes[DEF_PLANE_NO].m.offset;
			dump_buf->size = dev->fmt.fmt.pix.sizeimage;
		} else {
			dump_buf->offset = 0;
			dump_buf->size = 0;
			rc = -1;
		}
		break;
	}
	default:
		return -ENOTTY;
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
	if (dev)
		snprintf((char *)cap->bus_info, sizeof(cap->bus_info),
				"platform:viv%d", dev->id);

	cap->capabilities = V4L2_CAP_VIDEO_CAPTURE |
			V4L2_CAP_STREAMING | V4L2_CAP_DEVICE_CAPS | V4L2_CAP_TIMEPERFRAME;
	cap->device_caps = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING;
	return 0;
}

static int vidioc_enum_fmt_vid_cap(struct file *file, void *priv,
				   struct v4l2_fmtdesc *f)
{
	struct viv_video_device *dev = video_drvdata(file);

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

	if (f->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;
	*f = handle->vdev->fmt;
	return 0;
}

static int vidioc_try_fmt_vid_cap(struct file *file, void *priv,
				  struct v4l2_format *f)
{
	struct viv_video_device *dev = video_drvdata(file);
	struct viv_video_file *handle = priv_to_handle(file->private_data);
	struct viv_video_fmt *format = NULL;
	int bytesperline, sizeimage;
	int i;

	pr_debug("enter %s\n", __func__);

	if (f->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	viv_post_simple_event(VIV_VIDEO_EVENT_CREATE_PIPELINE,
		handle->streamid, &handle->vfh, true);

	f->fmt.pix.width = clamp_t(u32, f->fmt.pix.width, VIDEO_FRAME_MIN_WIDTH,
				VIDEO_FRAME_MAX_WIDTH);
	f->fmt.pix.height = clamp_t(u32, f->fmt.pix.height, VIDEO_FRAME_MIN_HEIGHT,
				 VIDEO_FRAME_MAX_HEIGHT);

	f->fmt.pix.width = ALIGN_UP(f->fmt.pix.width, VIDEO_FRAME_WIDTH_ALIGN);
	f->fmt.pix.height = ALIGN_UP(f->fmt.pix.height, VIDEO_FRAME_HEIGHT_ALIGN);

	for (i = 0; i < dev->formatscount; ++i) {
		if (dev->formats[i].fourcc == f->fmt.pix.pixelformat) {
			format = &dev->formats[i];
			break;
		}
	}
	if (format == NULL)
		return -EINVAL;

	f->fmt.pix.field = dev->fmt.fmt.pix.field;
	if (f->fmt.pix.colorspace == V4L2_COLORSPACE_DEFAULT) {
		f->fmt.pix.colorspace = dev->fmt.fmt.pix.colorspace;
	}
	init_v4l2_fmt(f, format->bpp, format->depth, &bytesperline, &sizeimage);
	f->fmt.pix.bytesperline = bytesperline;
	f->fmt.pix.sizeimage = sizeimage;
	return 0;
}

static int viv_post_fmt_event(struct file *file)
{
	int ret = 0;
	struct viv_video_file *handle = priv_to_handle(file->private_data);
	struct viv_video_device *vdev = handle->vdev;

	struct v4l2_event event;
	struct viv_video_event *v_event;

	if (vdev->pipeline_status == PIPELINE_INIT) {
		ret = viv_post_simple_event(VIV_VIDEO_EVENT_NEW_STREAM,
			handle->streamid, &handle->vfh, true);
		if (ret)
			return ret;

		vdev->pipeline_status = PIPELINE_CREATED;
		handle->state = 1;
	}

	v_event = (struct viv_video_event *)&event.u.data[0];
	v_event->stream_id = handle->streamid;
	v_event->file = &handle->vfh;
	v_event->addr = handle->vdev->fmt.fmt.pix.width;
	v_event->response = handle->vdev->fmt.fmt.pix.height;
	v_event->buf_index = handle->vdev->fmt.fmt.pix.pixelformat;
	v_event->sync = true;
	event.type = VIV_VIDEO_EVENT_TYPE;
	event.id = VIV_VIDEO_EVENT_SET_FMT;
	ret = viv_post_event(&event, &handle->vfh, true);
	if (ret)
		return ret;

	vdev->pipeline_status = PIPELINE_FMTSETTED;

	return ret;
}

static int vidioc_s_fmt_vid_cap(struct file *file, void *priv,
				struct v4l2_format *f)
{
	struct viv_video_file *handle = priv_to_handle(file->private_data);
	struct viv_video_device *vdev = handle->vdev;
	int ret;
	struct v4l2_event event;
	struct viv_video_event *v_event;
	struct viv_rect *rect = (struct viv_rect *)vdev->ctrls.buf_va;

	pr_debug("enter %s\n", __func__);
	if (f->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	ret = vidioc_try_fmt_vid_cap(file, priv, f);

	if (ret < 0)
		return -EINVAL;

	if ((f->fmt.pix.pixelformat == V4L2_PIX_FMT_SBGGR8) ||
	    (f->fmt.pix.pixelformat == V4L2_PIX_FMT_SGBRG8) ||
	    (f->fmt.pix.pixelformat == V4L2_PIX_FMT_SGRBG8) ||
	    (f->fmt.pix.pixelformat == V4L2_PIX_FMT_SRGGB8) ||
	    (f->fmt.pix.pixelformat == V4L2_PIX_FMT_SBGGR10) ||
	    (f->fmt.pix.pixelformat == V4L2_PIX_FMT_SGBRG10) ||
	    (f->fmt.pix.pixelformat == V4L2_PIX_FMT_SGRBG10) ||
	    (f->fmt.pix.pixelformat == V4L2_PIX_FMT_SRGGB10) ||
	    (f->fmt.pix.pixelformat == V4L2_PIX_FMT_SBGGR12) ||
	    (f->fmt.pix.pixelformat == V4L2_PIX_FMT_SGBRG12) ||
	    (f->fmt.pix.pixelformat == V4L2_PIX_FMT_SGRBG12) ||
	    (f->fmt.pix.pixelformat == V4L2_PIX_FMT_SRGGB12)) {
		viv_config_dwe(handle, false);
	}

	handle->vdev->fmt = *f;

	vdev->compose.left   = 0;
	vdev->compose.top    = 0;
	vdev->compose.width  = f->fmt.pix.width;
	vdev->compose.height = f->fmt.pix.height;

	rect->left   = vdev->compose.left;
	rect->top    = vdev->compose.top;
	rect->width  = vdev->compose.width;
	rect->height = vdev->compose.height;
	v_event = (struct viv_video_event *)&event.u.data[0];
	v_event->stream_id = handle->streamid;
	v_event->file = &(handle->vfh);
	v_event->sync = true;
	v_event->addr = vdev->ctrls.buf_pa;
	event.type = VIV_VIDEO_EVENT_TYPE;
	event.id = VIV_VIDEO_EVENT_SET_COMPOSE;
	viv_post_event(&event, &handle->vfh, true);

	viv_post_fmt_event(file);

	return ret;
}

static int vidioc_reqbufs(struct file *file, void *priv,
			  struct v4l2_requestbuffers *p)
{
	struct viv_video_file *handle = priv_to_handle(file->private_data);
	struct viv_video_file *ph;
	struct viv_video_device *vdev = handle->vdev;
	unsigned long flags;
	int ret = 0;

	pr_debug("enter %s %d %d\n", __func__, p->count, p->memory);
	if (p->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	spin_lock_irqsave(&file_list_lock[vdev->id], flags);
	list_for_each_entry(ph, &file_list_head[vdev->id], entry) {
		if (ph->streamid == handle->streamid &&
				ph != handle && ph->req) {
			pr_err("stream is busy %d\n", ph->streamid);
			spin_unlock_irqrestore(&file_list_lock[vdev->id],
					flags);
			return -EBUSY;
		}
	}
	spin_unlock_irqrestore(&file_list_lock[vdev->id], flags);

	if (p->count == 0)
		handle->req = false;
	else
		handle->req = true;

	if ((p->count) && (vdev->pipeline_status != PIPELINE_FMTSETTED))
		viv_post_fmt_event(file);

	mutex_lock(&handle->buffer_mutex);
	ret = vb2_reqbufs(&handle->queue, p);
	mutex_unlock(&handle->buffer_mutex);

	return ret;
}

static int vidioc_querybuf(struct file *file, void *priv, struct v4l2_buffer *p)
{
	struct viv_video_file *handle = priv_to_handle(file->private_data);
	struct vb2_buffer *vb;
	int rc = 0;

	pr_debug("enter %s\n", __func__);

	if (p->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	mutex_lock(&handle->buffer_mutex);
	rc = vb2_querybuf(&handle->queue, p);
	if (!rc) {
		if (p->flags & V4L2_BUF_FLAG_MAPPED) {
			vb = handle->queue.bufs[p->index];
			p->m.offset = vb2_dma_contig_plane_dma_addr(vb, 0);
		}
	}
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

	if (handle->vdev->pipeline_status != PIPELINE_STREAMOFF) {
		rc = vb2_dqbuf(&handle->queue, p, file->f_flags & O_NONBLOCK);
		p->field = V4L2_FIELD_NONE;
		p->sequence = handle->sequence++;
	} else {
		rc = -EINVAL;
	}

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

static int vidioc_enum_framesizes(struct file *file, void *priv,
			    struct v4l2_frmsizeenum *fsize)
{
	struct viv_video_device *dev = video_drvdata(file);
	struct viv_video_file *handle = priv_to_handle(file->private_data);
	int i;

	viv_post_simple_event(VIV_VIDEO_EVENT_CREATE_PIPELINE,
		handle->streamid, &handle->vfh, true);

	if (fsize->index > 0)
		return -EINVAL;

	for (i = 0; i < dev->formatscount; ++i)
		if (dev->formats[i].fourcc == fsize->pixel_format)
			break;

	if (i == dev->formatscount)
		return -EINVAL;

	fsize->stepwise.min_width = VIDEO_FRAME_MIN_WIDTH;
	fsize->stepwise.min_height = VIDEO_FRAME_MIN_HEIGHT;
	fsize->stepwise.max_width = VIDEO_FRAME_MAX_WIDTH;
	fsize->stepwise.max_height = VIDEO_FRAME_MAX_HEIGHT;
	fsize->stepwise.step_width = VIDEO_FRAME_WIDTH_ALIGN;
	fsize->stepwise.step_height = VIDEO_FRAME_HEIGHT_ALIGN;

	fsize->type = V4L2_FRMSIZE_TYPE_STEPWISE;

	return 0;
}

static int vidioc_g_parm(struct file *file, void *fh,
			    struct v4l2_streamparm *a)
{
	struct viv_video_file *handle = priv_to_handle(file->private_data);

	if (a->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	memset(&a->parm, 0, sizeof(a->parm));
	a->parm.capture.capability = V4L2_CAP_TIMEPERFRAME;
	a->parm.capture.timeperframe = handle->vdev->timeperframe;
	return 0;
}

static int vidioc_s_parm(struct file *file, void *fh,
			    struct v4l2_streamparm *a)
{
	struct viv_video_file *handle = priv_to_handle(file->private_data);
	struct viv_video_device *vdev = handle->vdev;
	struct v4l2_event event;
	struct viv_video_event *v_event;

	if (a->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;
	if (a->parm.output.timeperframe.denominator > handle->vdev->camera_mode.fps)
		return -EINVAL;

	if (vdev->ctrls.buf_va == NULL)
		return -EINVAL;

	if (a->parm.capture.timeperframe.numerator == 0 ||
		a->parm.capture.timeperframe.denominator == 0) {
			memset(&a->parm, 0, sizeof(a->parm));
		a->parm.capture.capability = V4L2_CAP_TIMEPERFRAME;
		a->parm.capture.timeperframe = handle->vdev->timeperframe;
	}
	handle->vdev->timeperframe = a->parm.output.timeperframe;
	sprintf(vdev->ctrls.buf_va,"{<id>:<s.fps>;<fps>:%d}",handle->vdev->timeperframe.denominator);
	v_event = (struct viv_video_event *)&event.u.data[0];
	v_event->stream_id = 0;
	v_event->file = &handle->vfh;
	v_event->sync = true;
	v_event->addr = vdev->ctrls.buf_pa;
	event.type = VIV_VIDEO_EVENT_TYPE;
	event.id = VIV_VIDEO_EVENT_EXTCTRL;

	viv_post_event(&event, &handle->vfh, true);

	return 0;
}

static int vidioc_enum_frameintervals(struct file *filp, void *priv,
			    struct v4l2_frmivalenum *fival)
{
	struct viv_video_device *dev = video_drvdata(filp);
	struct viv_video_file *handle = priv_to_handle(filp->private_data);
	int i;

	if (!handle->pipeline) {
		viv_post_simple_event(VIV_VIDEO_EVENT_CREATE_PIPELINE,
			handle->streamid, &handle->vfh, true);
		handle->pipeline = 1;
	}

	for (i = 0; i < dev->formatscount; ++i)
		if (dev->formats[i].fourcc == fival->pixel_format)
			break;

	if (i == dev->formatscount)
		return -EINVAL;

	if (fival->index >= dev->camera_mode.fps)
		return -EINVAL;

	if (fival->width  % VIDEO_FRAME_WIDTH_ALIGN  ||
		fival->height % VIDEO_FRAME_HEIGHT_ALIGN ||
		fival->width  < VIDEO_FRAME_MIN_WIDTH    ||
		fival->height < VIDEO_FRAME_MIN_HEIGHT   ||
		fival->width  > VIDEO_FRAME_MAX_WIDTH    ||
		fival->height > VIDEO_FRAME_MAX_HEIGHT)
		return -EINVAL;

	fival->discrete.numerator = 1;
	fival->discrete.denominator = dev->camera_mode.fps - fival->index;
	fival->type = V4L2_FRMSIZE_TYPE_DISCRETE;

	return 0;
}

static int vidioc_g_pixelaspect(struct file *file, void *fh,
				    int buf_type, struct v4l2_fract *aspect)
{
	if (buf_type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;
	pr_debug("%s not implemented\n", __func__);
	return 0;
}

static int vidioc_g_selection(struct file *file, void *fh,
				    struct v4l2_selection *s)
{
	struct viv_video_file *handle = priv_to_handle(file->private_data);
	struct viv_video_device *vdev = handle->vdev;

	if (s->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	viv_post_simple_event(VIV_VIDEO_EVENT_CREATE_PIPELINE,
		handle->streamid, &handle->vfh, true);

	switch (s->target) {
	case V4L2_SEL_TGT_CROP_DEFAULT:
	case V4L2_SEL_TGT_CROP_BOUNDS:
		s->r.left   = 0;
		s->r.top    = 0;
		s->r.width  = vdev->camera_mode.size.width;
		s->r.height = vdev->camera_mode.size.height;
		break;
	case V4L2_SEL_TGT_CROP:
		s->r = vdev->crop;
		break;

	case V4L2_SEL_TGT_COMPOSE_DEFAULT:
	case V4L2_SEL_TGT_COMPOSE_BOUNDS:
		s->r.left   = 0;
		s->r.top    = 0;
		s->r.width  = vdev->crop.width;
		s->r.height = vdev->crop.height;
		break;
	case V4L2_SEL_TGT_COMPOSE:
		s->r = vdev->compose;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int vidioc_s_selection(struct file *file, void *fh,
				    struct v4l2_selection *s)
{
	struct viv_video_file *handle = priv_to_handle(file->private_data);
	struct viv_video_device *vdev = handle->vdev;
	struct v4l2_event event;
	struct viv_video_event *v_event;
	struct viv_rect * rect;
	int rc;

	if (s->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	viv_post_simple_event(VIV_VIDEO_EVENT_CREATE_PIPELINE,
		handle->streamid, &handle->vfh, true);

	if (s->r.top < 0 || s->r.left < 0)
		return -EINVAL;

	switch (s->target) {
	case V4L2_SEL_TGT_CROP:
		if (((s->r.left + s->r.width) < VIDEO_FRAME_MIN_WIDTH) ||
		    ((s->r.left + s->r.width) > vdev->camera_mode.size.width) ||
		    ((s->r.top + s->r.height) < VIDEO_FRAME_MIN_HEIGHT) ||
		    ((s->r.top + s->r.height) > vdev->camera_mode.size.height))
			return -EINVAL;
		break;
	case V4L2_SEL_TGT_COMPOSE:
		if (((s->r.left + s->r.width) < VIDEO_FRAME_MIN_WIDTH) ||
		    ((s->r.left + s->r.width) > VIDEO_FRAME_MAX_WIDTH) ||
		    ((s->r.top + s->r.height) < VIDEO_FRAME_MIN_HEIGHT) ||
		    ((s->r.top + s->r.height) > VIDEO_FRAME_MAX_HEIGHT))
			return -EINVAL;
		break;
	default:
		return -EINVAL;
	}

	rect = (struct viv_rect *)vdev->ctrls.buf_va;
	if (!rect)
		return -ENOMEM;
	rect->left   = s->r.left;
	rect->top    = s->r.top;
	rect->width  = s->r.width;
	rect->height = s->r.height;

	v_event = (struct viv_video_event *)&event.u.data[0];
	v_event->stream_id = handle->streamid;
	v_event->file = &(handle->vfh);
	v_event->sync = true;
	v_event->addr = vdev->ctrls.buf_pa;
	event.type = VIV_VIDEO_EVENT_TYPE;
	if (s->target == V4L2_SEL_TGT_CROP)
		event.id = VIV_VIDEO_EVENT_SET_CROP;
	else
		event.id = VIV_VIDEO_EVENT_SET_COMPOSE;
	rc = viv_post_event(&event, &handle->vfh, true);
	if (rc == 0) {
		if (s->target == V4L2_SEL_TGT_COMPOSE) {
			vdev->compose.left   = rect->left;
			vdev->compose.top    = rect->top;
			vdev->compose.width  = rect->width;
			vdev->compose.height = rect->height;
		} else {
			vdev->crop.left   = rect->left;
			vdev->crop.top    = rect->top;
			vdev->crop.width  = rect->width;
			vdev->crop.height = rect->height;
		}
	}
	return rc;
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
	.vidioc_g_pixelaspect = vidioc_g_pixelaspect,
	.vidioc_g_selection = vidioc_g_selection,
	.vidioc_s_selection = vidioc_s_selection,
};

/* sys /dev/mem can't map large memory size */
static int viv_private_mmap(struct file *file, struct vm_area_struct *vma)
{
	/* Map reserved video memory. */
	struct viv_video_file *handle = priv_to_handle(file->private_data);
	struct viv_video_device *vdev = video_drvdata(file);
	int ret  = 0;
	bool dma_coherent = false;
	struct ext_dma_buf *b, *edb = NULL;

	list_for_each_entry(b, &handle->extdmaqueue, entry) {
		if ((b->addr >> PAGE_SHIFT) == vma->vm_pgoff) {
			dma_coherent = true;
			edb = b;
			break;
		}
	}

	if(vma->vm_pgoff == vdev->ctrls.buf_pa >> PAGE_SHIFT) {
		vma->vm_pgoff = 0;
		vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
		ret = dma_mmap_coherent(handle->queue.dev, vma, vdev->ctrls.buf_va,
			vdev->ctrls.buf_pa, vma->vm_end - vma->vm_start);
	} else if (dma_coherent) {
		vma->vm_pgoff = 0;
		vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
		ret = dma_mmap_coherent(handle->queue.dev, vma, edb->vaddr,
			edb->addr, vma->vm_end - vma->vm_start);
	}else {
		ret = remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff,
			vma->vm_end - vma->vm_start,vma->vm_page_prot);
	}
	if (ret)
		return -EAGAIN;
	return 0;
}

int vidioc_mmap(struct file *file, struct vm_area_struct *vma)
{
	int rc;
	struct viv_video_file *handle = priv_to_handle(file->private_data);
	struct viv_video_device *dev = video_drvdata(file);

	if (dev->dumpbuf != NULL) {
		return vb2_mmap(dev->dumpbuf->vb.vb2_buf.vb2_queue, vma);
	}

	if (handle->streamid < 0)
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
		rc = vb2_poll(&handle->queue, file, wait) |
				v4l2_ctrl_poll(file, wait);
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

	if (!vdev->ctrls.buf_va)
		return -EINVAL;

	switch (ctrl->id) {
	case V4L2_CID_VIV_STRING: {
		ret = 0;
		szbuf = (char *)vdev->ctrls.buf_va;
		if (!ctrl->p_new.p_char)
			return -EINVAL;
		strcpy(szbuf, ctrl->p_new.p_char);
		v_event = (struct viv_video_event *)&event.u.data[0];
		v_event->stream_id = 0;
		v_event->file = NULL;
		v_event->sync = true;
		v_event->addr = vdev->ctrls.buf_pa;
		event.type = VIV_VIDEO_EVENT_TYPE;
		event.id = VIV_VIDEO_EVENT_EXTCTRL;

		ret = viv_v4l2_post_event(vdev, &event, true);

		strcpy(ctrl->p_new.p_char, szbuf);
		break;
	}
	}
	return ret;
}

static const struct v4l2_ctrl_ops viv_ctrl_ops = {
	.s_ctrl = viv_s_ctrl,
};

const struct v4l2_ctrl_config viv_video_ctrls[] = {
	{
		.ops = &viv_ctrl_ops,
		.id = V4L2_CID_VIV_STRING,
		.type = V4L2_CTRL_TYPE_STRING,
		.name = "viv_ext_ctrl",
		.max = VIV_JSON_BUFFER_SIZE-1,
		.step = 1,
	},
};

static int viv_notifier_bound(struct v4l2_async_notifier *notifier,
		    struct v4l2_subdev *sd, struct v4l2_async_subdev *asd)
{
	struct viv_video_device *dev = container_of(notifier,
			struct viv_video_device, subdev_notifier);

	if (!dev)
		return 0;
	dev->subdevs[dev->sdcount] = sd;
	dev->sdcount++;
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
		mutex_lock(&dev->mdev->graph_mutex);
		rc = viv_create_default_links(dev);
		mutex_unlock(&dev->mdev->graph_mutex);

		if (rc) {
			pr_err("failed to create media links!\n");
			return rc;
		}
	}

	return v4l2_device_register_subdev_nodes(dev->v4l2_dev);
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

static void viv_buf_notify(struct vvbuf_ctx *ctx, struct vb2_dc_buf *buf)
{
	struct viv_video_file *fh;
	struct viv_video_device *vdev;
	u64 cur_ts, interval;

	if (!buf || buf->vb.vb2_buf.state != VB2_BUF_STATE_ACTIVE)
		return;

	fh = container_of(buf->vb.vb2_buf.vb2_queue,
			struct viv_video_file, queue);
	vdev = fh->vdev;
	if (!vdev->active)
		return;
	buf->vb.vb2_buf.planes[DEF_PLANE_NO].bytesused =
			vdev->fmt.fmt.pix.sizeimage;
	cur_ts = ktime_get_ns();
#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 0, 0)
	buf->vb.vb2_buf.timestamp = buf->timestamp;
#endif
	vb2_buffer_done(&buf->vb.vb2_buf, VB2_BUF_STATE_DONE);

	/* print fps info for debugging purpose */
	interval = ktime_us_delta(cur_ts,vdev->last_ts);
	if (interval < 3 * USEC_PER_SEC) {
		vdev->frame_cnt++;
	} else {
		if (vdev->last_ts > 0) {
			vdev->fps = (uint32_t) (vdev->frame_cnt * USEC_PER_SEC * 100 / interval);
		}
		vdev->frame_cnt = 0;
		vdev->last_ts = cur_ts;
	}

}

static const struct vvbuf_ops viv_buf_ops = {
	.notify = viv_buf_notify,
};

static int video_info_procfs_show(struct seq_file *sfile, void *offset)
{
	struct viv_video_device *vdev;
	vdev = (struct viv_video_device *) sfile->private;

	seq_printf(sfile, "Name\t\t: %s\n", vdev->video->name);
	seq_printf(sfile, "Resolution\t: %dx%d (max:%4dx%-4d min:%dx%d)\n",
				vdev->compose.width, vdev->compose.height,
				VIDEO_FRAME_MAX_WIDTH, VIDEO_FRAME_MAX_HEIGHT,
				VIDEO_FRAME_MIN_WIDTH, VIDEO_FRAME_MIN_HEIGHT);
	seq_printf(sfile, "Status\t\t: %s\n", vdev->pipeline_status == PIPELINE_STREAMON ? "run" : "idle");
	seq_printf(sfile, "Fps\t\t: %d.%d\n", vdev->fps/100, vdev->fps%100);

	return 0;
}

static int video_procfs_open(struct inode *inode, struct file *file)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 17, 0)
	return single_open(file, video_info_procfs_show, PDE_DATA(inode));
#else
	return single_open(file, video_info_procfs_show, pde_data(inode));
#endif
}

static const struct proc_ops video_procfs_ops = {
	.proc_open = video_procfs_open,
	.proc_release = seq_release,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
};

static int video_create_procfs(struct viv_video_device *vdev)
{
	struct proc_dir_entry *ent;
	char video_procfs_filename[30];
	if (!vdev->video) {
		return -1;
	}

	sprintf(video_procfs_filename, "vsivideo%d", vdev->video->num);
	ent = proc_create_data(video_procfs_filename, 0444, NULL, &video_procfs_ops, vdev);
	if (!ent)
		return -1;

	vdev->pde = ent;
	return 0;
}

struct dev_node {
	enum v4l2_async_match_type match_type;
	struct device_node *node;
	int id;
	const char *name;
};

static inline int viv_find_compatible_nodes(struct dev_node *nodes, int size)
{
	static const char * const compat_name[] = {
			ISP_COMPAT_NAME};
	struct device_node *node, *avail;
	int i, rc, id, cnt = 0;

	for (i = 0; i < ARRAY_SIZE(compat_name) && cnt < size; ++i) {
		node = NULL;
		for (; cnt < size;) {
			node = of_find_compatible_node(node,
					NULL, compat_name[i]);
			if (!node)
				break;
			avail = NULL;
			id = -1;
			rc = fwnode_property_read_u32(
					of_fwnode_handle(node), "id", &id);
			if (rc) {
				if (of_device_is_available(node))
					avail = node;
			} else if (of_device_is_available(node))
				avail = node;

			if (avail) {
				nodes[cnt].node = avail;
				nodes[cnt].id = id;
				nodes[cnt].match_type = V4L2_ASYNC_MATCH_FWNODE;
				cnt++;
			}
		}
	}
	return cnt;
}

extern struct v4l2_subdev *g_dwe_subdev[2];
static int viv_video_probe(struct platform_device *pdev)
{
	struct viv_video_device *vdev;
	int rc = 0;
	int i, video_id;
	struct dev_node nodes[MAX_SUBDEVS_NUM];
	int nodecount;

	int j;
	struct v4l2_async_subdev *asd;
	strscpy(mdev.model, "viv_media", sizeof(mdev.model));
	mdev.ops = &viv_mdev_ops;
	mdev.dev = &pdev->dev;
	media_device_init(&mdev);

	memset(nodes, 0, sizeof(nodes));
	nodecount = viv_find_compatible_nodes(nodes, MAX_SUBDEVS_NUM);
	for (i = 0; i < nodecount; i++) {
		if (nodes[i].node) {
			video_id = nodes[i].id ;
			if(video_id >= VIDEO_NODE_NUM) {
				pr_err("%s: id %d is too large (id > %d) \n",
						__func__, video_id, VIDEO_NODE_NUM);
				rc = -EINVAL;
				goto probe_end;
			}

			spin_lock_init(&file_list_lock[video_id]);
			INIT_LIST_HEAD(&file_list_head[video_id]);

			vvdev[video_id] = kzalloc(sizeof(*vdev), GFP_KERNEL);
			if (WARN_ON(!vvdev[video_id])) {
				rc = -ENOMEM;
				goto probe_end;
			}
			vdev = vvdev[video_id];
			vdev->id = video_id;
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
			sprintf(vdev->video->name, "viv_v4l2%d", video_id);

			v4l2_ctrl_handler_init(&vdev->ctrls.handler,  2 + ARRAY_SIZE(viv_video_ctrls));
			vdev->ctrls.request = v4l2_ctrl_new_custom(&vdev->ctrls.handler, &viv_video_ctrls[0], NULL);
			vdev->video->ctrl_handler = &vdev->ctrls.handler;

			vdev->video->release = video_device_release;
			vdev->video->fops = &video_ops;
			vdev->video->ioctl_ops = &video_ioctl_ops;
			vdev->video->minor = -1;
#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0)
			vdev->video->vfl_type = VFL_TYPE_VIDEO;
#else
			vdev->video->vfl_type = VFL_TYPE_GRABBER;
#endif
#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 0, 0)
			vdev->video->device_caps =
					V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING;
#endif
			video_set_drvdata(vdev->video, vdev);

			vvbuf_ctx_init(&vdev->bctx);
			vdev->bctx.ops = &viv_buf_ops;

			vdev->mdev = &mdev;
			vdev->v4l2_dev->mdev = &mdev;

			vdev->video->entity.name = vdev->video->name;
			vdev->video->entity.obj_type = MEDIA_ENTITY_TYPE_VIDEO_DEVICE;
			vdev->video->entity.function = MEDIA_ENT_F_IO_V4L;
			vdev->video->entity.ops = &viv_media_ops;

			vdev->pad.flags = MEDIA_PAD_FL_SINK | MEDIA_PAD_FL_MUST_CONNECT;
			rc = media_entity_pads_init(&vdev->video->entity,
					1, &vdev->pad);
			if (WARN_ON(rc < 0))
				goto register_fail;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
			v4l2_async_nf_init(&vdev->subdev_notifier);
#else
			v4l2_async_notifier_init(&vdev->subdev_notifier);
#endif

			vdev->subdev_notifier.ops = &sd_async_notifier_ops;

#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0)
			rc = video_register_device(vdev->video, VFL_TYPE_VIDEO, -1);
#else
			rc = video_register_device(vdev->video, VFL_TYPE_GRABBER, -1);
#endif
			if (WARN_ON(rc < 0))
				goto register_fail;

			for (j = 0; j < nodecount; ++j) {
				if (nodes[j].id == video_id) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
						asd = __v4l2_async_nf_add_fwnode(
							&vdev->subdev_notifier,
							of_fwnode_handle(nodes[j].node),
							sizeof(struct v4l2_async_subdev));
#elif LINUX_VERSION_CODE > KERNEL_VERSION(5, 12, 0)
						asd = __v4l2_async_notifier_add_fwnode_subdev(
							&vdev->subdev_notifier,
							of_fwnode_handle(nodes[j].node),
							sizeof(struct v4l2_async_subdev));
#else
						asd = v4l2_async_notifier_add_fwnode_subdev(
							&vdev->subdev_notifier,
							of_fwnode_handle(nodes[j].node),
							sizeof(struct v4l2_async_subdev));
#endif
					if (asd) {
						vdev->asd[vdev->asdcount] = asd;
						vdev->asdcount++;
					}
				}
			}

			if (g_dwe_subdev[video_id]) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
				asd = __v4l2_async_nf_add_fwnode(
					&vdev->subdev_notifier,
					g_dwe_subdev[video_id]->fwnode,
					sizeof(struct v4l2_async_subdev));
#elif LINUX_VERSION_CODE > KERNEL_VERSION(5, 12, 0)
				asd = __v4l2_async_notifier_add_fwnode_subdev(
					&vdev->subdev_notifier,
					g_dwe_subdev[video_id]->fwnode,
					sizeof(struct v4l2_async_subdev));
#else
				asd = v4l2_async_notifier_add_fwnode_subdev(
					&vdev->subdev_notifier,
					g_dwe_subdev[video_id]->fwnode,
					sizeof(struct v4l2_async_subdev));
#endif
				if (asd) {
						vdev->asd[vdev->asdcount] = asd;
						vdev->asdcount++;
				}
			}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
			rc = v4l2_async_nf_register(vdev->v4l2_dev,
					&vdev->subdev_notifier);
#else
			rc = v4l2_async_notifier_register(vdev->v4l2_dev,
					&vdev->subdev_notifier);
#endif
			if (WARN_ON(rc < 0))
				goto register_fail;
			rc = video_create_procfs(vdev);
			if (rc < 0) {
				pr_err("create video proc fs failed.\n");
				goto register_fail;
			}

			vdev->ctrls.buf_va = dma_alloc_coherent(&(pdev->dev),
				VIV_JSON_BUFFER_SIZE, &(vdev->ctrls.buf_pa), GFP_KERNEL);
			if (vdev->ctrls.buf_va == NULL) {
				pr_err("%s: alloc v4l2 ctrls resourse failed \n", __func__);
				goto register_fail;
			}
			init_completion(&vdev->ctrls.wait);

			vdev->fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			vdev->fmt.fmt.pix.field = V4L2_FIELD_NONE;
			vdev->fmt.fmt.pix.colorspace = V4L2_COLORSPACE_SMPTE170M;

			if (sizeof(vdev->formats) >= sizeof(formats)) {
				memcpy(vdev->formats, formats, sizeof(formats));
				vdev->formatscount = ARRAY_SIZE(formats);
			}
			vdev->last_ts = 0;
			vdev->fps = 0;
			vdev->frame_cnt = 0;
			atomic_set(&(vdev->refcnt), 0);
			mutex_init(&vdev->event_lock);

			continue;
register_fail:
			video_device_release(vdev->video);
		}
	}
	if (!rc)
		rc = media_device_register(&mdev);
probe_end:
	return rc;
}

static int viv_video_remove(struct platform_device *pdev)
{
	struct viv_video_device *vdev;
	int i;

	for (i = VIDEO_NODE_NUM-1; i >= 0; i--) {
		vdev = vvdev[i];
		if (!vdev || !vdev->video)
			continue;
		media_entity_cleanup(&vdev->video->entity);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
		v4l2_async_nf_cleanup(&vdev->subdev_notifier);
		v4l2_async_nf_unregister(&vdev->subdev_notifier);
#else
		v4l2_async_notifier_cleanup(&vdev->subdev_notifier);
		v4l2_async_notifier_unregister(&vdev->subdev_notifier);
#endif

		v4l2_device_unregister(vdev->v4l2_dev);
		kfree(vdev->v4l2_dev);
		video_unregister_device(vdev->video);
		vvbuf_ctx_deinit(&vdev->bctx);

		mutex_destroy(&vdev->event_lock);
		dma_free_coherent(&(pdev->dev), VIV_JSON_BUFFER_SIZE,
			vdev->ctrls.buf_va, vdev->ctrls.buf_pa);
		v4l2_ctrl_handler_free(&vdev->ctrls.handler);
		proc_remove(vdev->pde);
		kfree(vvdev[i]);
		vvdev[i] = NULL;
	}

	media_device_unregister(&mdev);
	media_device_cleanup(&mdev);
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
