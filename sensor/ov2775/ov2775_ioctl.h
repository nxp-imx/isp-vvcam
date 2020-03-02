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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
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
#ifndef _OV2775_IOC_H_
#define _OV2775_IOC_H_
#ifdef __KERNEL__
#include <media/v4l2-subdev.h>
#else
#include <linux/videodev2.h>
#endif
enum {
	OV2775IOC_G_BLS = 0x100,
	OV2775IOC_G_GAIN,
	OV2775IOC_S_GAIN,
	OV2775IOC_S_VSGAIN,
	OV2775IOC_S_EXP,
	OV2775IOC_S_VSEXP,
	OV2775IOC_S_PATTERN,
	OV2775IOC_S_BLS,
	OV2775IOC_S_POWER,
	OV2775IOC_G_VERSION,
	OV2775IOC_STREAMON,
	OV2775IOC_STREAMOFF,
	OV2775IOC_READ_REG,
	OV2775IOC_WRITE_REG,
};

struct ov2775_reg_setting_t {
	__u16 addr;
	__u8 val;
};

struct ov2775_gain_context {
	__u32 again;
	__u32 dgain;
};

#ifdef __KERNEL__
long ov2775_priv_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg);

int ov2775_s_bayer_pattern(void *dev, __u8 pattern);
int ov2775_g_gain(void *dev, struct ov2775_gain_context *gain);
int ov2775_s_gain(void *dev, struct ov2775_gain_context *gain);
int ov2775_s_vsgain(void *dev, struct ov2775_gain_context *gain);
int ov2775_s_exp(void *dev, __u32 exp);
int ov2775_s_vsexp(void *dev, uint32_t exp);
int ov2775_g_version(void *dev, __u32 *version);
int ov2775_streamon(void *dev);
int ov2775_streamoff(void *dev);

#endif

#endif /* _OV2775_IOC_H_ */
