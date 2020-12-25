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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
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
#ifndef _VIV_VIDEO_KEVENT_H_
#define _VIV_VIDEO_KEVENT_H_
#include <linux/videodev2.h>

#ifndef __KERNEL__
#include <stdint.h>
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
#endif

enum {
	VIV_VIDEO_EVENT_MIN = 0,
	VIV_VIDEO_EVENT_NEW_STREAM,
	VIV_VIDEO_EVENT_DEL_STREAM,
	VIV_VIDEO_EVENT_START_STREAM,
	VIV_VIDEO_EVENT_STOP_STREAM,
	VIV_VIDEO_EVENT_SET_FMT,
	VIV_VIDEO_EVENT_QUERYCAPS,
	VIV_VIDEO_EVENT_PASS_JSON,
	VIV_VIDEO_EVENT_EXTCTRL,
	VIV_VIDEO_EVENT_EXTCTRL2,
	VIV_VIDEO_EVENT_SET_CAPSMODE,
	VIV_VIDEO_EVENT_GET_CAPS_SUPPORTS,
	VIV_VIDEO_EVENT_QBUF,
	VIV_VIDEO_EVENT_MAX,
};

enum {
	VIV_DWE_EVENT_MIN = VIV_VIDEO_EVENT_MAX + 1,
	VIV_DWE_EVENT_FRAMEDONE,
	VIV_DWE_EVENT_MAX,
};

/* max support to 64 bytes! */
struct viv_video_event {
	u32 stream_id;
	void *file;
	u64 addr;
	int buf_index;
	u64 response;
	u32 sync;
};

struct v4l2_user_buffer {
	u64 addr;
	int streamid;
	void *file;
};

/* UNUSED */
#define VIV_JSON_BUFFER_SIZE  (64*1024)
struct viv_control_event {
	/* physical address of json request, fixed size 64K */
	u64 request;
	/* physical address of json response fixed size 64K */
	u64 response;
	u32 id;
};


struct ext_buf_info {
	u64 addr;
	u64 size;
};

struct vvcam_constant_modeinfo {
	unsigned index;
	unsigned w;
	unsigned h;
	unsigned fps;
	unsigned brpat; /*bayer pattern*/
	unsigned bitw; /*bit width*/
};

#define CALIBXML_FILE_NAME_SIZE 64
struct viv_caps_mode_s {
	int mode;
	char CalibXmlName[CALIBXML_FILE_NAME_SIZE];
};

enum viv_caps_hdr_mode_e
{
	VIV_CAPS_MODE_LINEAR ,
	VIV_CAPS_MODE_HDR_STITCH,
	VIV_CAPS_MODE_HDR_NATIVE,
};

struct viv_caps_mode_info_s{
	unsigned int index;
	unsigned int width;
	unsigned int height;
	unsigned int hdr_mode;
	unsigned int fps;
};

#define VIV_CAPS_MODE_MAX_COUNT    20
struct viv_caps_supports{
	unsigned int count;
	struct viv_caps_mode_info_s mode[VIV_CAPS_MODE_MAX_COUNT];
};

#define VIV_VIDEO_ISPIRQ_TYPE	(V4L2_EVENT_PRIVATE_START + 0x0)
#define VIV_VIDEO_MIIRQ_TYPE	(V4L2_EVENT_PRIVATE_START + 0x1)
#define VIV_VIDEO_EVENT_TYPE	(V4L2_EVENT_PRIVATE_START + 0x2000)
#define VIV_DWE_EVENT_TYPE   	(V4L2_EVENT_PRIVATE_START + 0x3000)

#define VIV_VIDEO_EVENT_TIMOUT_MS	5000

#define VIV_VIDIOC_EVENT_COMPLETE		_IOW('V', BASE_VIDIOC_PRIVATE + 100, struct viv_video_event)
#define VIV_VIDIOC_BUFFER_ALLOC			_IOWR('V', BASE_VIDIOC_PRIVATE + 101, struct ext_buf_info)
#define VIV_VIDIOC_BUFFER_FREE			_IOWR('V', BASE_VIDIOC_PRIVATE + 102, struct ext_buf_info)
#define VIV_VIDIOC_CONTROL_EVENT		_IOWR('V', BASE_VIDIOC_PRIVATE + 103, struct viv_control_event)
#define VIV_VIDIOC_S_STREAMID			_IOW('V', BASE_VIDIOC_PRIVATE + 104, int)
#define VIV_VIDIOC_BUFDONE				_IOW('V', BASE_VIDIOC_PRIVATE + 105, struct v4l2_user_buffer)
#define VIV_VIDIOC_QUERY_EXTMEM			_IOWR('V', BASE_VIDIOC_PRIVATE + 106, struct ext_buf_info)
#define VIV_VIDIOC_S_ENDPOINT			_IOW('V', BASE_VIDIOC_PRIVATE + 107, int)
#define VIV_VIDIOC_S_MODEINFO			_IOW('V', BASE_VIDIOC_PRIVATE + 108, struct vvcam_constant_modeinfo)
#define VIV_VIDIOC_S_CAPS_MODE			_IOW('V', BASE_VIDIOC_PRIVATE + 109, struct viv_caps_mode_s)
#define VIV_VIDIOC_G_CAPS_MODE			_IOWR('V', BASE_VIDIOC_PRIVATE + 110, struct viv_caps_mode_s)
#define VIV_VIDIOC_EVENT_RESULT	    	_IOWR('V', BASE_VIDIOC_PRIVATE + 111,int)
#define VIV_VIDIOC_GET_CAPS_SUPPORTS    _IOWR('V', BASE_VIDIOC_PRIVATE + 112,struct viv_caps_supports)
#define VIV_VIDIOC_SET_CAPS_SUPPORTS    _IOWR('V', BASE_VIDIOC_PRIVATE + 113,struct viv_caps_supports)
#define VIV_VIDIOC_S_DWECFG				_IOW('V', BASE_VIDIOC_PRIVATE + 114, int)
#define VIV_VIDIOC_G_DWECFG				_IOR('V', BASE_VIDIOC_PRIVATE + 115, int)

#endif
