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
#ifndef _OS08a20_IOC_H_
#define _OS08a20_IOC_H_
#ifdef __KERNEL__
#include <media/v4l2-subdev.h>
#else
#include <linux/videodev2.h>
#endif
enum {
	OS08a20IOC_G_BLS = 0x100,
	OS08a20IOC_G_GAIN,
	OS08a20IOC_S_GAIN,
	OS08a20IOC_S_VSGAIN,
	OS08a20IOC_S_EXP,
	OS08a20IOC_S_VSEXP,
	OS08a20IOC_S_PATTERN,
	OS08a20IOC_S_BLS,
	OS08a20IOC_S_POWER,
	OS08a20IOC_G_VERSION,
	OS08a20IOC_STREAMON,
	OS08a20IOC_STREAMOFF,
	OS08a20IOC_READ_REG,
	OS08a20IOC_WRITE_REG,
};

struct os08a20_reg_setting_t {
	__u16 addr;
	__u8 val;
};

struct os08a20_gain_context {
	__u32 again;
	__u32 dgain;
};

#ifdef __KERNEL__
long os08a20_priv_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg);

int os08a20_s_bayer_pattern(void *dev, __u8 pattern);
int os08a20_g_gain(void *dev, struct os08a20_gain_context *gain);
int os08a20_s_gain(void *dev, struct os08a20_gain_context *gain);
int os08a20_s_vsgain(void *dev, struct os08a20_gain_context *gain);
int os08a20_s_exp(void *dev, __u32 exp);
int os08a20_s_vsexp(void *dev, uint32_t exp);
int os08a20_g_version(void *dev, __u32 * version);
int os08a20_streamon(void *dev);
int os08a20_streamoff(void *dev);

#endif

#endif /* _OS08a20_IOC_H_ */
