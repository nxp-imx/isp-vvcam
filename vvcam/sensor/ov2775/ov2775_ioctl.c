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
#include "ov2775_ioctl.h"
#include "ov2775_mipi_v3.h"
#include <linux/delay.h>

int ov2775_s_gain(void *dev, struct ov2775_gain_context *gain)
{
	struct ov2775 *sensor = dev;
	__u8 again = 0;

	/* pr_info("enter %s\n", __func__); */
	ov2775_write_reg(sensor, 0x3467, 0x00);
	ov2775_write_reg(sensor, 0x3464, 0x04);

	ov2775_write_reg(sensor, 0x315a, (gain->dgain & 0xFF00) >> 8);
	ov2775_write_reg(sensor, 0x315b, gain->dgain & 0x00FF);
	ov2775_write_reg(sensor, 0x315c, (gain->dgain & 0xFF000000) >> 24);
	ov2775_write_reg(sensor, 0x315d, gain->dgain & (0x00FF0000) >> 16);

	ov2775_read_reg(sensor, 0x30bb, &again);
	again &= 0xF0;
	again |= gain->again;
	ov2775_write_reg(sensor, 0x30bb, again);

	ov2775_write_reg(sensor, 0x3464, 0x14);
	ov2775_write_reg(sensor, 0x3467, 0x01);

	return 0;
}

int ov2775_s_vsgain(void *dev, struct ov2775_gain_context *gain)
{
	struct ov2775 *sensor = dev;
	__u8 again = 0;

	ov2775_write_reg(sensor, 0x3467, 0x00);
	ov2775_write_reg(sensor, 0x3464, 0x04);

	ov2775_write_reg(sensor, 0x315e, (gain->dgain & 0xFF00) >> 8);
	ov2775_write_reg(sensor, 0x315f, gain->dgain & (0x00FF));

	ov2775_read_reg(sensor, 0x30bb, &again);
	again &= 0x3F;
	again |= gain->again;
	ov2775_write_reg(sensor, 0x30bb, again);

	ov2775_write_reg(sensor, 0x3464, 0x14);
	ov2775_write_reg(sensor, 0x3467, 0x01);

	return 0;
}

int ov2775_s_exp(void *dev, uint32_t exp)
{
	struct ov2775 *sensor = dev;

	/* pr_info("enter %s 0x%08x\n", __func__, exp); */
	ov2775_write_reg(sensor, 0x3467, 0x00);
	ov2775_write_reg(sensor, 0x3464, 0x04);

	ov2775_write_reg(sensor, 0x30b6, (exp & 0xFF00) >> 8);
	ov2775_write_reg(sensor, 0x30b7, exp & 0x00FF);

	ov2775_write_reg(sensor, 0x3464, 0x14);
	ov2775_write_reg(sensor, 0x3467, 0x01);
	return 0;
}

int ov2775_s_vsexp(void *dev, uint32_t exp)
{
	struct ov2775 *sensor = dev;

	/* pr_info("enter %s 0x%08x\n", __func__, exp); */
	ov2775_write_reg(sensor, 0x3467, 0x00);
	ov2775_write_reg(sensor, 0x3464, 0x04);

	ov2775_write_reg(sensor, 0x30b8, (exp & 0xFF00) >> 8);
	ov2775_write_reg(sensor, 0x30b9, exp & 0x00FF);

	ov2775_write_reg(sensor, 0x3464, 0x14);
	ov2775_write_reg(sensor, 0x3467, 0x01);
	return 0;
}

int ov2775_g_gain(void *dev, struct ov2775_gain_context *gain)
{
	struct ov2775 *sensor = dev;
	__u8 val = 0;

	ov2775_read_reg(sensor, 0x315a, &val);
	gain->dgain = val << 8;
	ov2775_read_reg(sensor, 0x315b, &val);
	gain->dgain |= val << 0;
	ov2775_read_reg(sensor, 0x315c, &val);
	gain->dgain |= val << 24;
	ov2775_read_reg(sensor, 0x315d, &val);
	gain->dgain |= val << 16;
	ov2775_read_reg(sensor, 0x30bb, &val);
	gain->again = val;
	return 0;
}

int ov2775_g_version(void *dev, __u32 * version)
{
	struct ov2775 *sensor = dev;
	__u8 val = 0;

	ov2775_read_reg(sensor, 0x300a, &val);
	*version = val << 8;
	ov2775_read_reg(sensor, 0x300b, &val);
	*version |= val;
	return 0;
}

int ov2775_streamon(void *dev)
{
	struct ov2775 *sensor = dev;

	ov2775_write_reg(sensor, 0x3012, 0x01);
	return 0;
}

int ov2775_streamoff(void *dev)
{
	struct ov2775 *sensor = dev;

	ov2775_write_reg(sensor, 0x3012, 0x00);
	return 0;
}

int ov2775_s_bayer_pattern(void *dev, __u8 pattern)
{
	struct ov2775 *sensor = dev;
	u8 h_shift = 0, v_shift = 0;
	u8 val_h, val_l;
	u16 val;

	h_shift = pattern % 2;
	v_shift = pattern / 2;

	ov2775_read_reg(sensor, 0x30a0, &val_h);
	ov2775_read_reg(sensor, 0x30a1, &val_l);
	val = (((val_h << 8) & 0xff00) | (val_l & 0x00ff)) + h_shift;
	val_h = (val >> 8) & 0xff;
	val_l = val & 0xff;
	ov2775_write_reg(sensor, 0x30a0, val_h);
	ov2775_write_reg(sensor, 0x30a1, val_l);

	ov2775_read_reg(sensor, 0x30a2, &val_h);
	ov2775_read_reg(sensor, 0x30a3, &val_l);
	val = (((val_h << 8) & 0xff00) | (val_l & 0x00ff)) + v_shift;
	val_h = (val >> 8) & 0xff;
	val_l = val & 0xff;
	ov2775_write_reg(sensor, 0x30a2, val_h);
	ov2775_write_reg(sensor, 0x30a3, val_l);

	ov2775_read_reg(sensor, 0x30a4, &val_h);
	ov2775_read_reg(sensor, 0x30a5, &val_l);
	val = (((val_h << 8) & 0xff00) | (val_l & 0x00ff)) + h_shift;
	val_h = (val >> 8) & 0xff;
	val_l = val & 0xff;
	ov2775_write_reg(sensor, 0x30a4, val_h);
	ov2775_write_reg(sensor, 0x30a5, val_l);

	ov2775_read_reg(sensor, 0x30a6, &val_h);
	ov2775_read_reg(sensor, 0x30a7, &val_l);
	val = (((val_h << 8) & 0xff00) | (val_l & 0x00ff)) + v_shift;
	val_h = (val >> 8) & 0xff;
	val_l = val & 0xff;
	ov2775_write_reg(sensor, 0x30a6, val_h);
	ov2775_write_reg(sensor, 0x30a7, val_l);
	return 0;
}

int ov2775_s_hdr(void *dev, bool enable)
{
	struct ov2775 *sensor = dev;
	pr_info("%s: %d\n", __func__, enable);
	sensor->hdr = enable;
	ov2775_write_reg(sensor, 0x3190, enable ? 0x05 : 0x08);
	msleep(5);
	return 0;
}

int ov2775_s_fps(void *dev, __u32 fps)
{
	struct ov2775 *sensor = dev;
	pr_info("%s: %d\n", __func__, fps);

	/* minimize to 5 fps */
	if (fps < 0x04)
		fps = 0x04;
	ov2775_write_reg(sensor, 0x3005, fps);
	return 0;
}

int ov2775_ioc_qcap(void *dev, void *args)
{
	struct v4l2_capability *cap = (struct v4l2_capability *)args;

	strcpy((char *)cap->driver, "ov2775");
	return 0;
}

long ov2775_priv_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov2775 *sensor = client_to_ov2775(client);
	struct ov2775_reg_setting_t reg;
	int ret = -1;

	/* pr_info("enter %s\n", __func__); */
	switch (cmd) {
	case OV2775IOC_G_GAIN:
		ret = ov2775_g_gain(sensor, (struct ov2775_gain_context *)arg);
		break;
	case OV2775IOC_S_GAIN:
		ret = ov2775_s_gain(sensor, (struct ov2775_gain_context *)arg);
		break;
	case OV2775IOC_S_VSGAIN:
		ret =
		    ov2775_s_vsgain(sensor, (struct ov2775_gain_context *)arg);
		break;
	case OV2775IOC_G_VERSION:
		ret = ov2775_g_version(sensor, (__u32 *) arg);
		break;
	case OV2775IOC_STREAMON:
		ret = ov2775_streamon(sensor);
		break;
	case OV2775IOC_STREAMOFF:
		ret = ov2775_streamoff(sensor);
		break;
	case OV2775IOC_S_EXP:
		ret = ov2775_s_exp(sensor, *(uint32_t *) arg);
		break;
	case OV2775IOC_S_VSEXP:
		ret = ov2775_s_vsexp(sensor, *(uint32_t *) arg);
		break;
	case OV2775IOC_S_PATTERN:
		ret = ov2775_s_bayer_pattern(sensor, *(__u8 *) arg);
		break;
	case OV2775IOC_WRITE_REG:
		reg = *(struct ov2775_reg_setting_t *)arg;
		ret = ov2775_write_reg(sensor, reg.addr, reg.val) < 0;
		break;
	case OV2775IOC_READ_REG:
		reg = *(struct ov2775_reg_setting_t *)arg;
		ret = ov2775_read_reg(sensor, reg.addr, &reg.val) < 0;
		break;
	case VIDIOC_QUERYCAP:
		ret = ov2775_ioc_qcap(NULL, arg);
		break;
	case OV2775IOC_S_HDR:
		ret = ov2775_s_hdr(sensor, *(bool *) arg);
		break;
	case OV2775IOC_S_FPS:
		ret = ov2775_s_fps(sensor, *(__u32 *) arg);
		break;
	default:
		/* pr_err("unsupported ov2775 command %d.", cmd); */
		break;
	}

	return ret;
}
