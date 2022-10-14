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
#include <linux/videodev2.h>
#include <media/media-device.h>
#include <media/v4l2-async.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-fh.h>

#include "viv_video_kevent.h"
#include "vvbuf.h"

#define MAX_SUBDEVS_NUM (8)
#define VIDEO_NODE_NUM  (2)

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

enum vsi_pipeline_status {
	PIPELINE_INIT = 0,
	PIPELINE_CREATED,
	PIPELINE_FMTSETTED,
	PIPELINE_STREAMON,
	PIPELINE_STREAMOFF,
};

struct viv_video_device {
	struct vvbuf_ctx bctx;
	struct video_device *video;
	struct v4l2_device *v4l2_dev;
	struct v4l2_async_notifier subdev_notifier;
	struct v4l2_subdev *subdevs[MAX_SUBDEVS_NUM];
	int sdcount;
	struct v4l2_async_subdev *asd[MAX_SUBDEVS_NUM];
	int asdcount;
	struct media_device *mdev;
	struct media_pad pad;
	struct v4l2_format fmt;
	struct v4l2_fract timeperframe;
	struct v4l2_rect crop, compose;
	struct viv_custom_ctrls ctrls;
	struct vvcam_constant_modeinfo camera_mode;
	uint32_t camera_status;
	struct viv_video_fmt formats[20];
	int formatscount;
	int id;
	struct viv_caps_mode_s caps_mode;
	int event_result;
	bool dweEnabled;
	atomic_t refcnt;
	struct viv_caps_supports caps_supports;
	u64 duration, last_ts, frameCnt[VIDEO_NODE_NUM];
	u32 loop_cnt[VIDEO_NODE_NUM];
	struct completion subscribed_wait;
	int subscribed_cnt;
	int active;
	void *rmem;
	bool frame_flag;
	int dumpbuf_status;
	struct vb2_dc_buf* dumpbuf;
	struct mutex event_lock;
	struct video_event_shm event_shm;
	int pipeline_status;
};

struct viv_video_file {
	struct v4l2_fh vfh;
	int streamid;
	int state; /* 0-free,1-ready,2-streaming,-1-closed */
	int sequence;
	bool req;
	bool capsqueried;
	struct vb2_queue queue;
	struct mutex buffer_mutex;
	struct completion wait;
	struct list_head entry;
	struct viv_video_device *vdev;
	int pipeline;
#ifdef CONFIG_VIDEOBUF2_DMA_CONTIG
	struct list_head extdmaqueue;
#endif
};

#define VIDEO_FRAME_MIN_WIDTH 176
#define VIDEO_FRAME_MIN_HEIGHT 144
#define VIDEO_FRAME_MAX_WIDTH 4096
#define VIDEO_FRAME_MAX_HEIGHT 3072
#define VIDEO_FRAME_WIDTH_ALIGN 16
#define VIDEO_FRAME_HEIGHT_ALIGN 8



#define priv_to_handle(priv)	container_of(priv, struct viv_video_file, vfh)
#define queue_to_handle(__q)	container_of(__q, struct viv_video_file, queue)

#endif /* _ISP_VIDEO_H_ */
