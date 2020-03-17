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

#include "viv_video_kevent.h"
#include "isp_version.h"

struct viv_video_device {
	struct file *filep;
	struct video_device *video;
	//struct media_device media_dev;
	struct v4l2_device *v4l2_dev;
	struct isp_device *isp[ISP_HW_NUMBER];
#ifdef WITH_DWE
	struct dwe_device *dwe;
#endif
#ifdef WITH_VSE
	struct vse_device *vse;
#endif
};

struct viv_video_file {
	struct v4l2_fh vfh;
	int streamid;
	/* 0-free, 1-ready to streaming, 2-streaming */
	int state;
	int sequence;
	bool req;
	struct vb2_queue queue;
	struct mutex event_mutex;
	struct mutex buffer_mutex;
	struct completion wait;
	struct v4l2_format fmt;
	struct list_head entry;
	struct list_head qlist;
};

#define priv_to_handle(priv)	container_of(priv, struct viv_video_file, vfh)
#define queue_to_handle(__q)	container_of(__q, struct viv_video_file, queue)

struct vb2_dc_buf {
	struct vb2_v4l2_buffer vb;
	struct list_head list;
	struct device *dev;
	unsigned long size;
	void *cookie;
	/* MMAP related */
	struct vb2_vmarea_handler handler;
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 9, 0)
	refcount_t refcount;
#else
	atomic_t refcount;
#endif
};

enum {
	MEDIA_PIX_FMT_YUV422SP = 0,
	MEDIA_PIX_FMT_YUV422I,
	MEDIA_PIX_FMT_YUV420SP,
	MEDIA_PIX_FMT_YUV444,
	MEDIA_PIX_FMT_RGB888,
	MEDIA_PIX_FMT_RGB888P,
	MEDIA_PIX_FMT_METADATA,
};

#endif// _ISP_VIDEO_H_

