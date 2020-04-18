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
#include "os08a20_ioctl.h"
#include "os08a20_mipi_v3.h"

int os08a20_s_gain(void *dev, struct os08a20_gain_context *gain)
{
	struct os08a20 *sensor = dev;

	/* pr_info("enter %s\n", __func__); */
	os08a20_write_reg(sensor, 0x320d, 0x00);
	os08a20_write_reg(sensor, 0x3208, 0x00);

	os08a20_write_reg(sensor, 0x350a, (gain->dgain & 0xFF00) >> 8);
	os08a20_write_reg(sensor, 0x350b, gain->dgain & 0x00FF);
	os08a20_write_reg(sensor, 0x350e, (gain->dgain & 0xFF000000) >> 24);
	os08a20_write_reg(sensor, 0x350f, gain->dgain & (0x00FF0000) >> 16);

	os08a20_write_reg(sensor, 0x320d, 0x00);
	os08a20_write_reg(sensor, 0x3208, 0xe0);

	return 0;
}

int os08a20_s_vsgain(void *dev, struct os08a20_gain_context *gain)
{
	struct os08a20 *sensor = dev;
	__u8 again = 0;

	os08a20_write_reg(sensor, 0x3467, 0x00);
	os08a20_write_reg(sensor, 0x3464, 0x04);

	os08a20_write_reg(sensor, 0x315e, (gain->dgain & 0xFF00) >> 8);
	os08a20_write_reg(sensor, 0x315f, gain->dgain & (0x00FF));

	os08a20_read_reg(sensor, 0x30bb, &again);
	again &= 0x3F;
	again |= gain->again;
	os08a20_write_reg(sensor, 0x30bb, again);

	os08a20_write_reg(sensor, 0x3464, 0x14);
	os08a20_write_reg(sensor, 0x3467, 0x01);

	return 0;
}

int os08a20_s_exp(void *dev, uint32_t exp)
{
	struct os08a20 *sensor = dev;

	/* pr_info("enter %s 0x%08x\n", __func__, exp); */
	os08a20_write_reg(sensor, 0x320d, 0x00);
	os08a20_write_reg(sensor, 0x3208, 0x00);

	os08a20_write_reg(sensor, 0x3501, (exp & 0xFF00) >> 8);
	os08a20_write_reg(sensor, 0x3502, exp & 0x00FF);

	os08a20_write_reg(sensor, 0x320d, 0x00);
	os08a20_write_reg(sensor, 0x3208, 0xe0);
	return 0;
}

int os08a20_s_vsexp(void *dev, uint32_t exp)
{
	struct os08a20 *sensor = dev;

	/* pr_info("enter %s 0x%08x\n", __func__, exp); */
	os08a20_write_reg(sensor, 0x350d, 0x00);
	os08a20_write_reg(sensor, 0x3508, 0x00);

	os08a20_write_reg(sensor, 0x3511, (exp & 0xFF00) >> 8);
	os08a20_write_reg(sensor, 0x3512, exp & 0x00FF);

	os08a20_write_reg(sensor, 0x350d, 0x00);
	os08a20_write_reg(sensor, 0x350f, 0xe0);
	return 0;
}

int os08a20_g_gain(void *dev, struct os08a20_gain_context *gain)
{
	struct os08a20 *sensor = dev;
	__u8 val = 0;

	os08a20_read_reg(sensor, 0x3513, &val);
	gain->dgain = val << 8;
	os08a20_read_reg(sensor, 0x3514, &val);
	gain->dgain |= val << 0;
	os08a20_read_reg(sensor, 0x3516, &val);
	gain->dgain |= val << 24;
	os08a20_read_reg(sensor, 0x3517, &val);
	gain->dgain |= val << 16;
	return 0;
}

int os08a20_g_version(void *dev, __u32 * version)
{
	struct os08a20 *sensor = dev;
	__u8 val = 0;

	os08a20_read_reg(sensor, 0x300a, &val);
	*version = val << 16;
	os08a20_read_reg(sensor, 0x300b, &val);
	*version |= val << 8;
	os08a20_read_reg(sensor, 0x300c, &val);
	*version |= val;
	return 0;
}

int os08a20_streamon(void *dev)
{
	struct os08a20 *sensor = dev;

	os08a20_write_reg(sensor, 0x0100, 0x01);
	return 0;
}

int os08a20_streamoff(void *dev)
{
	struct os08a20 *sensor = dev;

	os08a20_write_reg(sensor, 0x0100, 0x00);
	return 0;
}

int os08a20_s_bayer_pattern(void *dev, __u8 pattern)
{
	struct os08a20 *sensor = dev;
	u8 h_shift = 0, v_shift = 0;
	u8 val_h, val_l, val;

	h_shift = pattern % 2;
	v_shift = pattern / 2;

	os08a20_read_reg(sensor, 0x30a0, &val_h);
	os08a20_read_reg(sensor, 0x30a1, &val_l);
	val = (((val_h << 8) & 0xff00) | (val_l & 0x00ff)) + h_shift;
	val_h = (val >> 8) & 0xff;
	val_l = val & 0xff;
	os08a20_write_reg(sensor, 0x30a0, val_h);
	os08a20_write_reg(sensor, 0x30a1, val_l);

	os08a20_read_reg(sensor, 0x30a2, &val_h);
	os08a20_read_reg(sensor, 0x30a3, &val_l);
	val = (((val_h << 8) & 0xff00) | (val_l & 0x00ff)) + v_shift;
	val_h = (val >> 8) & 0xff;
	val_l = val & 0xff;
	os08a20_write_reg(sensor, 0x30a2, val_h);
	os08a20_write_reg(sensor, 0x30a3, val_l);

	os08a20_read_reg(sensor, 0x30a4, &val_h);
	os08a20_read_reg(sensor, 0x30a5, &val_l);
	val = (((val_h << 8) & 0xff00) | (val_l & 0x00ff)) + h_shift;
	val_h = (val >> 8) & 0xff;
	val_l = val & 0xff;
	os08a20_write_reg(sensor, 0x30a4, val_h);
	os08a20_write_reg(sensor, 0x30a5, val_l);

	os08a20_read_reg(sensor, 0x30a6, &val_h);
	os08a20_read_reg(sensor, 0x30a7, &val_l);
	val = (((val_h << 8) & 0xff00) | (val_l & 0x00ff)) + v_shift;
	val_h = (val >> 8) & 0xff;
	val_l = val & 0xff;
	os08a20_write_reg(sensor, 0x30a6, val_h);
	os08a20_write_reg(sensor, 0x30a7, val_l);
	return 0;
}

int os08a20_ioc_qcap(void *dev, void *args)
{
	struct v4l2_capability *cap = (struct v4l2_capability *)args;

	strcpy((char *)cap->driver, "os08a20");
	return 0;
}

long os08a20_priv_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct os08a20 *sensor = client_to_os08a20(client);
	struct os08a20_reg_setting_t reg;
	struct os08a20_gain_context *gain;
	uint32_t val = 0;
	int ret = -1;

	/* pr_info("enter %s\n", __func__); */
	switch (cmd) {
	case OS08a20IOC_G_GAIN:
		gain = (struct os08a20_gain_context *)arg;
		ret = os08a20_g_gain(sensor, gain);
		break;
	case OS08a20IOC_S_GAIN:
		gain = (struct os08a20_gain_context *)arg;
		ret = os08a20_s_gain(sensor, gain);
		break;
	case OS08a20IOC_S_VSGAIN:
		/* gain = (struct os08a20_gain_context *)arg; */
		/* ret = os08a20_s_vsgain(sensor, gain); */
		ret = 0;
		break;
	case OS08a20IOC_G_VERSION:
		ret = os08a20_g_version(sensor, (__u32 *) arg);
		break;
	case OS08a20IOC_STREAMON:
		ret = os08a20_streamon(sensor);
		break;
	case OS08a20IOC_STREAMOFF:
		ret = os08a20_streamoff(sensor);
		break;
	case OS08a20IOC_S_EXP:
		val = *(uint32_t *) arg;
		ret = os08a20_s_exp(sensor, val);
		break;
	case OS08a20IOC_S_VSEXP:
		val = *(uint32_t *) arg;
		ret = os08a20_s_vsexp(sensor, val);
		break;
	case OS08a20IOC_S_PATTERN:
		val = *(uint32_t *) arg;
		ret = os08a20_s_bayer_pattern(sensor, val);
		break;
	case OS08a20IOC_WRITE_REG:
		reg = *(struct os08a20_reg_setting_t *)arg;
		ret = os08a20_write_reg(sensor, reg.addr, reg.val) < 0;
		break;
	case OS08a20IOC_READ_REG:
		reg = *(struct os08a20_reg_setting_t *)arg;
		ret = os08a20_read_reg(sensor, reg.addr, &reg.val) < 0;
		break;
	case VIDIOC_QUERYCAP:
		ret = os08a20_ioc_qcap(NULL, arg);
		break;
	default:
		/* pr_err("unsupported os08a20 command %d.", cmd); */
		break;
	}

	return ret;
}
