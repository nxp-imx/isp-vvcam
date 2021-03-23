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
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/pinctrl/consumer.h>
#include <linux/regulator/consumer.h>
#include <linux/v4l2-mediabus.h>
#include <media/v4l2-device.h>
#include <media/v4l2-ctrls.h>

#include "vvsensor.h"

#include "os08a20_regs_1080p.h"
#include "os08a20_regs_1080p_hdr.h"
#include "os08a20_regs_4k.h"
#include "os08a20_regs_4k_hdr.h"
#include "os08a20_regs_1080p_hdr.h"

#define OS08a20_VOLTAGE_ANALOG			2800000
#define OS08a20_VOLTAGE_DIGITAL_CORE		1500000
#define OS08a20_VOLTAGE_DIGITAL_IO		1800000

#define MIN_FPS 15
#define MAX_FPS 30
#define DEFAULT_FPS 30

#define OS08a20_XCLK_MIN 6000000
#define OS08a20_XCLK_MAX 24000000

#define OS08a20_CHIP_ID_HIGH_BYTE	0x300A
#define OS08a20_CHIP_ID_LOW_BYTE		0x300B

#define OS08a20_SENS_PAD_SOURCE	0
#define OS08a20_SENS_PADS_NUM	1

enum os08a20_mode {
	os08a20_mode_MIN = 0,
	os08a20_mode_4K_3840_2160 = 0,
	os08a20_mode_1080P_1920_1080 = 0,
	os08a20_mode_720P_1280_720 = 1,
	os08a20_mode_NTSC_720_480 = 2,
	os08a20_mode_VGA_640_480 = 3,
	os08a20_mode_QVGA_320_240 = 4,
	os08a20_mode_QSXGA_2592_1944 = 5,
	os08a20_mode_MAX,
	os08a20_mode_INIT = 0xff,	/* only for sensor init */
};

enum os08a20_frame_rate {
	os08a20_15_fps,
	os08a20_30_fps
};

struct os08a20_datafmt {
	u32 code;
	enum v4l2_colorspace colorspace;
};

/* image size under 1280 * 960 are SUBSAMPLING
 * image size upper 1280 * 960 are SCALING
 */
enum os08a20_downsize_mode {
	SUBSAMPLING,
	SCALING,
};

struct os08a20_mode_info {
	enum os08a20_mode mode;
	enum os08a20_downsize_mode dn_mode;
	u32 width;
	u32 height;
	struct vvsensor_reg_value_t *init_data_ptr;
	u32 init_data_size;
	u32 bit_width;
	u32 fps;
	bool is_default;
	u32 stitching_mode;
};

struct os08a20_pll_info {
	enum os08a20_mode mode;
	struct vvsensor_reg_value_t *init_data_ptr;
	u32 init_data_size;
};

struct os08a20_hs_info {
	u32 width;
	u32 height;
	u32 frame_rate;
	u32 val;
};

struct os08a20 {
	struct regulator *io_regulator;
	struct regulator *core_regulator;
	struct regulator *analog_regulator;
	struct v4l2_subdev subdev;
	struct v4l2_device *v4l2_dev;
	struct i2c_client *i2c_client;
	struct v4l2_pix_format pix;
	const struct os08a20_datafmt *fmt;
	struct v4l2_captureparm streamcap;
	struct media_pad pads[OS08a20_SENS_PADS_NUM];
	bool on;

	/* control settings */
	int brightness;
	int hue;
	int contrast;
	int saturation;
	int red;
	int green;
	int blue;
	int ae_mode;

	u32 mclk;
	u8 mclk_source;
	struct clk *sensor_clk;
	int csi;

	void (*io_init) (struct os08a20 *);
	int pwn_gpio, rst_gpio;
	bool hdr;
	int fps;

	vvcam_mode_info_t cur_mode;
	struct mutex lock;
};

#define client_to_os08a20(client)\
	container_of(i2c_get_clientdata(client), struct os08a20, subdev)

long os08a20_priv_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg);
static void os08a20_stop(struct os08a20 *sensor);
s32 os08a20_write_reg(struct os08a20 *sensor, u16 reg, u8 val);

static struct vvcam_mode_info pos08a20_mode_info[] = {
	{
		.index	   = 0,
		.width	   = 1920,
		.height    = 1080,
		.fps	   = 60,
		.hdr_mode  = SENSOR_MODE_LINEAR,
		.bit_width = 10,
		.bayer_pattern = BAYER_BGGR,
		.ae_info = {
			.DefaultFrameLengthLines = 0x492,
			.one_line_exp_time_ns = 14250,
			.max_integration_time = 0x492 - 8,
			.min_integration_time = 8,
			.gain_accuracy = 1024,
			.max_gain = 62 * 1024,
			.min_gain = 1 * 1024,
		},
		.preg_data = os08a20_init_setting_1080p,
		.reg_data_count = ARRAY_SIZE(os08a20_init_setting_1080p),
	},
	{
		.index	   = 1,
		.width	  = 1920,
		.height   = 1080,
		.fps	  = 30,
		.hdr_mode = SENSOR_MODE_HDR_STITCH,
		.stitching_mode = SENSOR_STITCHING_DUAL_DCG,
		.bit_width = 10,
		.bayer_pattern = BAYER_BGGR,
		.ae_info = {
			.DefaultFrameLengthLines = 0x492,
			.one_line_exp_time_ns = 14250,
			.max_integration_time = 0x492 - 8,
			.min_integration_time = 8,
			.gain_accuracy = 1024,
			.max_gain = 62 * 1024,
			.min_gain = 1 * 1024,
		},
		.preg_data = os08a20_init_setting_1080p_hdr,
		.reg_data_count = ARRAY_SIZE(os08a20_init_setting_1080p_hdr),
	},
	{
		.index	   = 2,
		.width	  = 3840,
		.height   = 2160,
		.fps	  = 20,
		.hdr_mode = SENSOR_MODE_LINEAR,
		.bit_width = 12,
		.bayer_pattern = BAYER_BGGR,
		.ae_info = {
			.DefaultFrameLengthLines = 0xDB4,
			.one_line_exp_time_ns = 14250,
			.max_integration_time = 0xDB4 - 64 - 4,
			.min_integration_time = 8,
			.gain_accuracy = 1024,
			.max_gain = 62 * 1024,
			.min_gain = 1 * 1024,
		},
		.preg_data = os08a20_init_setting_4k,
		.reg_data_count = ARRAY_SIZE(os08a20_init_setting_4k),
	},
	{
		.index	   = 3,
		.width	  = 3840,
		.height   = 2160,
		.fps	  = 15,
		.hdr_mode = SENSOR_MODE_HDR_STITCH,
		.stitching_mode = SENSOR_STITCHING_L_AND_S,
		.bit_width = 10,
		.bayer_pattern = BAYER_BGGR,
		.ae_info = {
			.DefaultFrameLengthLines = 0x960,
			.one_line_exp_time_ns = 13889,
			.max_integration_time = 0x960 - 64 - 4,//T_long + Tshort < VTS - 4
			.min_integration_time = 8,
			.gain_accuracy = 1024,
			.max_gain = 62 * 1024,
			.min_gain = 1 * 1024,
		},
		.preg_data = os08a20_init_setting_4k_hdr,
		.reg_data_count = ARRAY_SIZE(os08a20_init_setting_4k_hdr),
	},
};


static struct os08a20_hs_info hs_setting[] = {
	{1920, 1080, 30, 0x0B},
	{1920, 1080, 15, 0x10},
	{3840, 2160, 30, 0x0B},
	{3840, 2160, 20, 0x10},
};

static int os08a20_probe(struct i2c_client *adapter,
			const struct i2c_device_id *device_id);
static int os08a20_remove(struct i2c_client *client);

static void os08a20_stop(struct os08a20 *sensor);

static const struct i2c_device_id os08a20_id[] = {
	{"ov2775", 0},
	{},
};

MODULE_DEVICE_TABLE(i2c, os08a20_id);

static int __maybe_unused os08a20_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct os08a20 *sensor = client_to_os08a20(client);

	if (sensor->on) {
		os08a20_stop(sensor);
	}

	return 0;
}

static int __maybe_unused os08a20_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct os08a20 *sensor = client_to_os08a20(client);

	if (sensor->on) {
		os08a20_write_reg(sensor, 0x0100, 0x01);
	}
	return 0;
}


static const struct dev_pm_ops os08a20_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(os08a20_suspend, os08a20_resume)
};
static const struct of_device_id os08a20_dt_ids[] = {
	{ .compatible = "ovti,ov2775" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, os08a20_dt_ids);

static struct i2c_driver os08a20_i2c_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name  = "ov2775",
		.pm = &os08a20_pm_ops,
		.of_match_table	= os08a20_dt_ids,
	},
	.probe	= os08a20_probe,
	.remove = os08a20_remove,
	.id_table = os08a20_id,
};

#if 0
static const struct os08a20_datafmt os08a20_colour_fmts[] = {
	{MEDIA_BUS_FMT_YVYU8_2X8, V4L2_COLORSPACE_JPEG},
	{MEDIA_BUS_FMT_UYVY8_2X8, V4L2_COLORSPACE_JPEG},
};
#else
static const struct os08a20_datafmt os08a20_colour_fmts[] = {
	{MEDIA_BUS_FMT_SBGGR12_1X12, V4L2_COLORSPACE_JPEG},
};
#endif

#if 0
static enum os08a20_frame_rate to_os08a20_frame_rate(struct v4l2_fract
						   *timeperframe)
{
	enum os08a20_frame_rate rate;
	u32 tgt_fps;		/* target frames per secound */

	pr_info("enter %s\n", __func__);
	tgt_fps = timeperframe->denominator / timeperframe->numerator;

	if (tgt_fps == 30)
		rate = os08a20_30_fps;
	else if (tgt_fps == 15)
		rate = os08a20_15_fps;
	else
		rate = -EINVAL;

	return rate;
}
#endif

static __u16 find_hs_configure(struct os08a20 *sensor)
{
	struct device *dev = &sensor->i2c_client->dev;
	struct v4l2_fract *timeperframe = &sensor->streamcap.timeperframe;
	struct v4l2_pix_format *pix = &sensor->pix;
	u32 frame_rate = timeperframe->denominator / timeperframe->numerator;
	int i;

	pr_info("enter %s\n", __func__);

	for (i = 0; i < ARRAY_SIZE(hs_setting); i++) {
		if (hs_setting[i].width == pix->width &&
			hs_setting[i].height == pix->height &&
			hs_setting[i].frame_rate == frame_rate)
			return hs_setting[i].val;
	}

	if (i == ARRAY_SIZE(hs_setting))
		dev_err(dev, "%s can not find hs configure\n", __func__);

	return -EINVAL;
}

/* Find a data format by a pixel code in an array */
static const struct os08a20_datafmt
*os08a20_find_datafmt(u32 code)
{
	int i;

	pr_debug("enter %s\n", __func__);
	for (i = 0; i < ARRAY_SIZE(os08a20_colour_fmts); i++)
		if (os08a20_colour_fmts[i].code == code)
			return os08a20_colour_fmts + i;

	return NULL;
}

static inline void os08a20_power_up(struct os08a20 *sensor)
{
	pr_debug("enter %s\n", __func__);
	if (gpio_is_valid(sensor->pwn_gpio)) {
		gpio_set_value_cansleep(sensor->pwn_gpio, 1);
	}
}

static inline void os08a20_power_down(struct os08a20 *sensor)
{
	pr_debug("enter %s\n", __func__);
	if (gpio_is_valid(sensor->pwn_gpio)) {
		gpio_set_value_cansleep(sensor->pwn_gpio, 0);
	}
}

static inline void os08a20_reset(struct os08a20 *sensor)
{
	pr_debug("enter %s\n", __func__);
	if (!gpio_is_valid(sensor->rst_gpio))
		return;

	gpio_set_value_cansleep(sensor->rst_gpio, 0);
	msleep(20);

	gpio_set_value_cansleep(sensor->rst_gpio, 1);
	msleep(20);
}

static int os08a20_regulator_enable(struct os08a20 *sensor)
{
	int ret = 0;
	struct device *dev = &(sensor->i2c_client->dev);

	pr_debug("enter %s\n", __func__);
	if (sensor->io_regulator)
	{
		regulator_set_voltage(sensor->io_regulator,
					  OS08a20_VOLTAGE_DIGITAL_IO,
					  OS08a20_VOLTAGE_DIGITAL_IO);
		ret = regulator_enable(sensor->io_regulator);
		if (ret)
		{
			dev_err(dev, "set io voltage failed\n");
			return ret;
		}
	}

	if (sensor->analog_regulator)
	{
		regulator_set_voltage(sensor->analog_regulator,
					  OS08a20_VOLTAGE_ANALOG,
					  OS08a20_VOLTAGE_ANALOG);
		ret = regulator_enable(sensor->analog_regulator);
		if (ret)
		{
			dev_err(dev, "set analog voltage failed\n");
			goto err_disable_io;
		}
	}

	if (sensor->core_regulator)
	{
		regulator_set_voltage(sensor->core_regulator,
					  OS08a20_VOLTAGE_DIGITAL_CORE,
					  OS08a20_VOLTAGE_DIGITAL_CORE);
		ret = regulator_enable(sensor->core_regulator);
		if (ret) {
			dev_err(dev, "set core voltage failed\n");
			goto err_disable_analog;
		}
	}

	return 0;

err_disable_analog:
	regulator_disable(sensor->analog_regulator);
err_disable_io:
	regulator_disable(sensor->io_regulator);
	return ret;
}

static void os08a20_regulator_disable(struct os08a20 *sensor)
{
	int ret = 0;
	struct device *dev = &(sensor->i2c_client->dev);

	if (sensor->core_regulator)
	{
		ret = regulator_disable(sensor->core_regulator);
		if (ret < 0)
			dev_err(dev, "core regulator disable failed\n");
	}

	if (sensor->analog_regulator)
	{
		ret = regulator_disable(sensor->analog_regulator);
		if (ret < 0)
			dev_err(dev, "analog regulator disable failed\n");
	}

	if (sensor->io_regulator)
	{
		ret = regulator_disable(sensor->io_regulator);
		if (ret < 0)
			dev_err(dev, "io regulator disable failed\n");
	}
	return ;
}


s32 os08a20_write_reg(struct os08a20 *sensor, u16 reg, u8 val)
{
	u8 au8Buf[3] = { 0 };

	au8Buf[0] = reg >> 8;
	au8Buf[1] = reg & 0xff;
	au8Buf[2] = val;
	if (i2c_master_send(sensor->i2c_client, au8Buf, 3) < 0) {
		pr_err("Write reg error: reg=%x, val=%x\n", reg, val);
		return -1;
	}

	return 0;
}

s32 os08a20_read_reg(struct os08a20 *sensor, u16 reg, u8 *val)
{
	struct device *dev = &sensor->i2c_client->dev;
	u8 au8RegBuf[2] = { 0 };
	u8 u8RdVal = 0;

	au8RegBuf[0] = reg >> 8;
	au8RegBuf[1] = reg & 0xff;

	if (i2c_master_send(sensor->i2c_client, au8RegBuf, 2) != 2) {
		dev_err(dev, "Read reg error: reg=%x\n", reg);
		return -1;
	}

	if (i2c_master_recv(sensor->i2c_client, &u8RdVal, 1) != 1) {
		dev_err(dev, "Read reg error: reg=%x, val=%x\n", reg, u8RdVal);
		return -1;
	}

	*val = u8RdVal;
	return u8RdVal;
}

static int os08a20_set_clk_rate(struct os08a20 *sensor)
{
	u32 tgt_xclk;		/* target xclk */
	int ret;

	/* mclk */
	tgt_xclk = sensor->mclk;
	tgt_xclk = min_t(u32, tgt_xclk, (u32) OS08a20_XCLK_MAX);
	tgt_xclk = max_t(u32, tgt_xclk, (u32) OS08a20_XCLK_MIN);
	sensor->mclk = tgt_xclk;

	pr_debug("	 Setting mclk to %d MHz\n", tgt_xclk / 1000000);
	ret = clk_set_rate(sensor->sensor_clk, sensor->mclk);
	if (ret < 0)
		pr_debug("set rate filed, rate=%d\n", sensor->mclk);
	return ret;
}

/* download os08a20 settings to sensor through i2c */
static int os08a20_download_firmware(struct os08a20 *sensor,
					struct vvsensor_reg_value_t *mode_setting,
					s32 size)
{
	register u32 delay_ms = 0;
	register u16 reg_addr = 0;
	register u8 mask = 0;
	register u8 val = 0;
	u8 reg_val = 0;
	int i, retval = 0;

	pr_err("enter %s\n", __func__);
	for (i = 0; i < size; ++i, ++mode_setting) {
		delay_ms = mode_setting->delay;
		reg_addr = mode_setting->addr;
		val = mode_setting->val;
		mask = mode_setting->mask;

		if (mask) {
			retval = os08a20_read_reg(sensor, reg_addr, &reg_val);
			if (retval < 0)
				break;

			reg_val &= ~(u8)mask;
			val &= mask;
			val |= reg_val;
		}

		retval = os08a20_write_reg(sensor, reg_addr, val);
		os08a20_read_reg(sensor, reg_addr, &reg_val);

		if (retval < 0)
			break;

		if (delay_ms)
			msleep(delay_ms);
	}

	return retval;
}

static void os08a20_start(struct os08a20 *sensor)
{
	pr_err("enter %s\n", __func__);
	os08a20_write_reg(sensor, 0x0100, 0x01);
	msleep(100);
}

static void os08a20_stop(struct os08a20 *sensor)
{
	pr_err("enter %s\n", __func__);
	os08a20_write_reg(sensor, 0x0100, 0x00);
}

/*!
 * os08a20_s_power - V4L2 sensor interface handler for VIDIOC_S_POWER ioctl
 * @s: pointer to standard V4L2 device structure
 * @on: indicates power mode (on or off)
 *
 * Turns the power on or off, depending on the value of on and returns the
 * appropriate error code.
 */
static int os08a20_s_power(struct v4l2_subdev *sd, int on)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct os08a20 *sensor = client_to_os08a20(client);

	pr_debug("enter %s\n", __func__);
	if (on)
		clk_prepare_enable(sensor->sensor_clk);
	else
		clk_disable_unprepare(sensor->sensor_clk);

	sensor->on = on;
	return 0;
}

/*!
 * os08a20_g_parm - V4L2 sensor interface handler for VIDIOC_G_PARM ioctl
 * @s: pointer to standard V4L2 sub device structure
 * @a: pointer to standard V4L2 VIDIOC_G_PARM ioctl structure
 *
 * Returns the sensor's video CAPTURE parameters.
 */
static int os08a20_g_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *a)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct os08a20 *sensor = client_to_os08a20(client);
	struct v4l2_captureparm *cparm = &a->parm.capture;
	int ret = 0;

	pr_debug("enter %s\n", __func__);
	switch (a->type) {
		/* This is the only case currently handled. */
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
	case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
		memset(a, 0, sizeof(*a));
		a->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		cparm->capability = sensor->streamcap.capability;
		cparm->timeperframe = sensor->streamcap.timeperframe;
		cparm->capturemode = sensor->streamcap.capturemode;
		ret = 0;
		break;

		/* These are all the possible cases. */
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
	case V4L2_BUF_TYPE_VIDEO_OVERLAY:
	case V4L2_BUF_TYPE_VBI_CAPTURE:
	case V4L2_BUF_TYPE_VBI_OUTPUT:
	case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE:
	case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT:
		ret = -EINVAL;
		break;

	default:
		pr_debug("	 type is unknown - %d\n", a->type);
		ret = -EINVAL;
		break;
	}

	return ret;
}

/*!
 * ov5460_s_parm - V4L2 sensor interface handler for VIDIOC_S_PARM ioctl
 * @s: pointer to standard V4L2 sub device structure
 * @a: pointer to standard V4L2 VIDIOC_S_PARM ioctl structure
 *
 * Configures the sensor to use the input parameters, if possible.	If
 * not possible, reverts to the old parameters and returns the
 * appropriate error code.
 */
static int os08a20_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *a)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct os08a20 *sensor = client_to_os08a20(client);
	struct v4l2_fract *timeperframe = &a->parm.capture.timeperframe;
	u32 tgt_fps;		/* target frames per secound */
	enum os08a20_mode mode = a->parm.capture.capturemode;
	int ret = 0;

	pr_debug("enter %s\n", __func__);
	switch (a->type) {
		/* This is the only case currently handled. */
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
	case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
		/* Check that the new frame rate is allowed. */
		if ((timeperframe->numerator == 0) ||
			(timeperframe->denominator == 0)) {
			timeperframe->denominator = DEFAULT_FPS;
			timeperframe->numerator = 1;
		}

		tgt_fps = timeperframe->denominator / timeperframe->numerator;

		if (tgt_fps > MAX_FPS) {
			timeperframe->denominator = MAX_FPS;
			timeperframe->numerator = 1;
		} else if (tgt_fps < MIN_FPS) {
			timeperframe->denominator = MIN_FPS;
			timeperframe->numerator = 1;
		}

		if (mode > os08a20_mode_MAX || mode < os08a20_mode_MIN) {
			pr_err("The camera mode[%d] is not supported!\n", mode);
			return -EINVAL;
		}

		sensor->streamcap.capturemode = mode;
		sensor->streamcap.timeperframe = *timeperframe;
		break;

		/* These are all the possible cases. */
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
	case V4L2_BUF_TYPE_VIDEO_OVERLAY:
	case V4L2_BUF_TYPE_VBI_CAPTURE:
	case V4L2_BUF_TYPE_VBI_OUTPUT:
	case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE:
	case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT:
		pr_debug("	 type is not V4L2_BUF_TYPE_VIDEO_CAPTURE but %d\n",
			 a->type);
		ret = -EINVAL;
		break;

	default:
		pr_debug("	 type is unknown - %d\n", a->type);
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int os08a20_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct os08a20 *sensor = client_to_os08a20(client);

	pr_debug("enter %s\n", __func__);
	if (enable)
		os08a20_start(sensor);
	else
		os08a20_stop(sensor);

	sensor->on = enable;
	return 0;
}

#if 0
static struct os08a20_mode_info *get_max_resolution(enum os08a20_frame_rate rate)
{
	u32 max_width;
	enum os08a20_mode mode;
	int i;

	pr_debug("enter %s\n", __func__);
	mode = 0;
	max_width = os08a20_mode_info_data[rate][0].width;

	for (i = 0; i < (os08a20_mode_MAX + 1); i++) {
		if (os08a20_mode_info_data[rate][i].width > max_width) {
			max_width = os08a20_mode_info_data[rate][i].width;
			mode = i;
		}
	}
	return &os08a20_mode_info_data[rate][mode];
}

static struct os08a20_mode_info *match(struct v4l2_mbus_framefmt *fmt,
					  enum os08a20_frame_rate rate)
{
	struct os08a20_mode_info *info;
	int i;

	pr_debug("enter %s\n", __func__);
	for (i = 0; i < (os08a20_mode_MAX + 1); i++) {
		if (fmt->width == os08a20_mode_info_data[rate][i].width &&
			fmt->height == os08a20_mode_info_data[rate][i].height) {
			info = &os08a20_mode_info_data[rate][i];
			break;
		}
	}
	if (i == os08a20_mode_MAX + 1)
		info = NULL;

	return info;
}

static void try_to_find_resolution(struct os08a20 *sensor,
				   struct v4l2_mbus_framefmt *mf)
{
	enum os08a20_mode mode = sensor->streamcap.capturemode;
	struct v4l2_fract *timeperframe = &sensor->streamcap.timeperframe;
	enum os08a20_frame_rate frame_rate = to_os08a20_frame_rate(timeperframe);
	struct device *dev = &sensor->i2c_client->dev;
	struct os08a20_mode_info *info;
	bool found = false;

	pr_debug("enter %s\n", __func__);
	if ((mf->width == os08a20_mode_info_data[frame_rate][mode].width) &&
		(mf->height == os08a20_mode_info_data[frame_rate][mode].height)) {
		info = &os08a20_mode_info_data[frame_rate][mode];
		found = true;
	} else {
		/* get mode info according to frame user's width and height */
		info = match(mf, frame_rate);
		if (info == NULL) {
			frame_rate ^= 0x1;
			info = match(mf, frame_rate);
			if (info) {
				sensor->streamcap.capturemode = -1;
				dev_err(dev, "%s %dx%d only support %s(fps)\n",
					__func__, info->width, info->height,
					(frame_rate == 0) ? "15fps" : "30fps");
				return;
			}
			goto max_resolution;
		}
		found = true;
	}

	/* get max resolution to resize */
max_resolution:
	if (!found) {
		frame_rate ^= 0x1;
		info = get_max_resolution(frame_rate);
	}

	sensor->streamcap.capturemode = info->mode;
	sensor->streamcap.timeperframe.denominator = (frame_rate) ? 30 : 15;
	sensor->pix.width = info->width;
	sensor->pix.height = info->height;
}
#endif

static int os08a20_set_fmt(struct v4l2_subdev *sd,
			  struct v4l2_subdev_pad_config *cfg,
			  struct v4l2_subdev_format *format)
{
	struct v4l2_mbus_framefmt *mf = &format->format;
	const struct os08a20_datafmt *fmt = os08a20_find_datafmt(mf->code);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct os08a20 *sensor = client_to_os08a20(client);
	unsigned int i;
	struct vvsensor_reg_value_t *mode_setting = NULL;
	int array_size = 0;

	pr_debug("enter %s\n", __func__);
	if (format->pad) {
		return -EINVAL;
	}

	if (!fmt) {
		mf->code = os08a20_colour_fmts[0].code;
		mf->colorspace = os08a20_colour_fmts[0].colorspace;
	}

	mf->field = V4L2_FIELD_NONE;
	/*	old search method,	vsi need change to
		search resolution by width/height */
	/*	try_to_find_resolution(sensor, mf); */
	if (format->which == V4L2_SUBDEV_FORMAT_TRY)
		return 0;

	for (i=0; i<ARRAY_SIZE(pos08a20_mode_info); i++)
	{
		if (sensor->hdr)
		{
			if (mf->width == pos08a20_mode_info[i].width &&
				mf->height == pos08a20_mode_info[i].height &&
				pos08a20_mode_info[i].hdr_mode != SENSOR_MODE_LINEAR)
			{
				memcpy(&(sensor->cur_mode), &pos08a20_mode_info[i], sizeof(struct vvcam_mode_info));
				mode_setting = pos08a20_mode_info[i].preg_data;
				array_size = pos08a20_mode_info[i].reg_data_count;
				return os08a20_download_firmware(sensor, mode_setting, array_size);
			}
		}else
		{
			if (mf->width == pos08a20_mode_info[i].width &&
				mf->height == pos08a20_mode_info[i].height &&
				pos08a20_mode_info[i].hdr_mode == SENSOR_MODE_LINEAR)
			{
				memcpy(&(sensor->cur_mode), &pos08a20_mode_info[i], sizeof(struct vvcam_mode_info));
				mode_setting = pos08a20_mode_info[i].preg_data;
				array_size = pos08a20_mode_info[i].reg_data_count;
				return os08a20_download_firmware(sensor, mode_setting, array_size);
			}
		}
	}

	pr_err("%s search error: %d %d\n", __func__, mf->width, mf->height);
	return -EINVAL;;
}

static int os08a20_get_fmt(struct v4l2_subdev *sd,
			  struct v4l2_subdev_pad_config *cfg,
			  struct v4l2_subdev_format *format)
{
	struct v4l2_mbus_framefmt *mf = &format->format;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct os08a20 *sensor = client_to_os08a20(client);

	pr_debug("enter %s\n", __func__);
	if (format->pad)
		return -EINVAL;

	memset(mf, 0, sizeof(struct v4l2_mbus_framefmt));

	mf->code = os08a20_colour_fmts[0].code;
	mf->colorspace = os08a20_colour_fmts[0].colorspace;
	mf->width = sensor->pix.width;
	mf->height = sensor->pix.height;
	mf->field = V4L2_FIELD_NONE;
	mf->reserved[1] = find_hs_configure(sensor);

	dev_dbg(&client->dev,
		"%s code=0x%x, w/h=(%d,%d), colorspace=%d, field=%d\n",
		__func__, mf->code, mf->width, mf->height, mf->colorspace,
		mf->field);

	return 0;
}

static int os08a20_enum_code(struct v4l2_subdev *sd,
				struct v4l2_subdev_pad_config *cfg,
				struct v4l2_subdev_mbus_code_enum *code)
{
	pr_debug("enter %s\n", __func__);
	if (code->pad || code->index >= ARRAY_SIZE(os08a20_colour_fmts))
		return -EINVAL;

	code->code = os08a20_colour_fmts[code->index].code;
	return 0;
}

/*!
 * os08a20_enum_framesizes - V4L2 sensor interface handler for
 *			   VIDIOC_ENUM_FRAMESIZES ioctl
 * @s: pointer to standard V4L2 device structure
 * @fsize: standard V4L2 VIDIOC_ENUM_FRAMESIZES ioctl structure
 *
 * Return 0 if successful, otherwise -EINVAL.
 */
static int os08a20_enum_framesizes(struct v4l2_subdev *sd,
				  struct v4l2_subdev_pad_config *cfg,
				  struct v4l2_subdev_frame_size_enum *fse)
{
	pr_debug("enter %s\n", __func__);
	if (fse->index >  ARRAY_SIZE(pos08a20_mode_info))
		return -EINVAL;

	fse->min_width = pos08a20_mode_info[fse->index].width;
	fse->max_width = fse->min_width;
	fse->min_height = pos08a20_mode_info[fse->index].height;
	fse->max_height = fse->min_height;

	return 0;
}

/*!
 * os08a20_enum_frameintervals - V4L2 sensor interface handler for
 *				   VIDIOC_ENUM_FRAMEINTERVALS ioctl
 * @s: pointer to standard V4L2 device structure
 * @fival: standard V4L2 VIDIOC_ENUM_FRAMEINTERVALS ioctl structure
 *
 * Return 0 if successful, otherwise -EINVAL.
 */
static int os08a20_enum_frameintervals(struct v4l2_subdev *sd,
					  struct v4l2_subdev_pad_config *cfg,
					  struct v4l2_subdev_frame_interval_enum
					  *fie)
{

	pr_debug("enter %s\n", __func__);
	if (fie->index < 0 || fie->index > os08a20_mode_MAX)
		return -EINVAL;

	if (fie->width == 0 || fie->height == 0 || fie->code == 0) {
		pr_warn("Please assign pixel format, width and height.\n");
		return -EINVAL;
	}

	fie->interval.numerator = 1;
	fie->interval.denominator = pos08a20_mode_info[fie->index].fps;

	return 0;
}

static int os08a20_link_setup(struct media_entity *entity,
				 const struct media_pad *local,
				 const struct media_pad *remote, u32 flags)
{
	return 0;
}

static struct v4l2_subdev_video_ops os08a20_subdev_video_ops = {
	.g_parm = os08a20_g_parm,
	.s_parm = os08a20_s_parm,
	.s_stream = os08a20_s_stream,
};

static const struct v4l2_subdev_pad_ops os08a20_subdev_pad_ops = {
	.enum_frame_size = os08a20_enum_framesizes,
	.enum_frame_interval = os08a20_enum_frameintervals,
	.enum_mbus_code = os08a20_enum_code,
	.set_fmt = os08a20_set_fmt,
	.get_fmt = os08a20_get_fmt,
};

static struct v4l2_subdev_core_ops os08a20_subdev_core_ops = {
	.s_power = os08a20_s_power,
	.ioctl = os08a20_priv_ioctl,
};

static struct v4l2_subdev_ops os08a20_subdev_ops = {
	.core = &os08a20_subdev_core_ops,
	.video = &os08a20_subdev_video_ops,
	.pad = &os08a20_subdev_pad_ops,
};

static const struct media_entity_operations os08a20_sd_media_ops = {
	.link_setup = os08a20_link_setup,
};

/*!
 * os08a20 I2C probe function
 *
 * @param adapter struct i2c_adapter *
 * @return	Error code indicating success or failure
 */

static int os08a20_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct pinctrl *pinctrl;
	struct device *dev = &client->dev;
	struct v4l2_subdev *sd;
	int retval;
	u8	reg_val = 0;
	u16 chip_id = 0;
	struct os08a20 *sensor;

	pr_debug("enter %s\n", __func__);
	sensor = devm_kmalloc(dev, sizeof(*sensor), GFP_KERNEL);
	if (!sensor)
		return -ENOMEM;
	/* Set initial values for the sensor struct. */
	memset(sensor, 0, sizeof(*sensor));
	sensor->i2c_client = client;

	/* os08a20 pinctrl */
	pinctrl = devm_pinctrl_get_select_default(dev);
	if (IS_ERR(pinctrl)) {
		dev_err(dev, "setup pinctrl failed\n");
		return PTR_ERR(pinctrl);
	}

	/* request power down pin */
	sensor->pwn_gpio = of_get_named_gpio(dev->of_node, "pwn-gpios", 0);
	if (!gpio_is_valid(sensor->pwn_gpio))
		dev_warn(dev, "No sensor pwdn pin available");
	else {
		retval = devm_gpio_request_one(dev, sensor->pwn_gpio,
						   GPIOF_OUT_INIT_HIGH,
						   "os08a20_mipi_pwdn");
		if (retval < 0) {
			dev_warn(dev, "Failed to set power pin\n");
			dev_warn(dev, "retval=%d\n", retval);
			return retval;
		}
	}

	/* request reset pin */
	sensor->rst_gpio = of_get_named_gpio(dev->of_node, "rst-gpios", 0);
	if (!gpio_is_valid(sensor->rst_gpio))
		dev_warn(dev, "No sensor reset pin available");
	else {
		retval = devm_gpio_request_one(dev, sensor->rst_gpio,
						   GPIOF_OUT_INIT_HIGH,
						   "os08a20_mipi_reset");
		if (retval < 0) {
			dev_warn(dev, "Failed to set reset pin\n");
			return retval;
		}
	}

	/* Set initial values for the sensor struct. */
	sensor->sensor_clk = devm_clk_get(dev, "csi_mclk");
	if (IS_ERR(sensor->sensor_clk)) {
		/* assuming clock enabled by default */
		sensor->sensor_clk = NULL;
		dev_err(dev, "clock-frequency missing or invalid\n");
		return PTR_ERR(sensor->sensor_clk);
	}

	retval = of_property_read_u32(dev->of_node, "mclk", &(sensor->mclk));
	if (retval) {
		dev_err(dev, "mclk missing or invalid\n");
		return retval;
	}

	retval = of_property_read_u32(dev->of_node, "mclk_source",
					  (u32 *)&(sensor->mclk_source));
	if (retval) {
		dev_err(dev, "mclk_source missing or invalid\n");
		return retval;
	}

	retval = of_property_read_u32(dev->of_node, "csi_id", &(sensor->csi));
	if (retval) {
		dev_err(dev, "csi id missing or invalid\n");
		return retval;
	}
	sensor->io_regulator = devm_regulator_get(dev, "DOVDD");
	if (IS_ERR(sensor->io_regulator))
	{
		dev_err(dev, "cannot get io regulator\n");
		return PTR_ERR(sensor->io_regulator);
	}

	sensor->core_regulator = devm_regulator_get(dev, "DVDD");
	if (IS_ERR(sensor->core_regulator))
	{
		dev_err(dev, "cannot get core regulator\n");
		return PTR_ERR(sensor->core_regulator);
	}

	sensor->analog_regulator = devm_regulator_get(dev, "AVDD");
	if (IS_ERR(sensor->analog_regulator))
	{
		dev_err(dev, "cannot get analog  regulator\n");
		return PTR_ERR(sensor->analog_regulator);
	}

	retval = os08a20_regulator_enable(sensor);
	if (retval) {
		dev_err(dev, "regulator enable failed\n");
		return retval;
	}

	/* Set mclk rate before clk on */
	os08a20_set_clk_rate(sensor);

	retval = clk_prepare_enable(sensor->sensor_clk);
	if (retval < 0) {
		dev_err(dev, "%s: enable sensor clk fail\n", __func__);
		goto probe_err_regulator_disable;
	}
	os08a20_power_up(sensor);
	os08a20_reset(sensor);

	sensor->io_init = os08a20_reset;

	sensor->pix.pixelformat = V4L2_PIX_FMT_UYVY;
	sensor->pix.width =pos08a20_mode_info[0].width;
	sensor->pix.height = pos08a20_mode_info[0].height;
	sensor->streamcap.capability = V4L2_MODE_HIGHQUALITY |
		V4L2_CAP_TIMEPERFRAME;
	sensor->streamcap.capturemode = 0;
	sensor->streamcap.timeperframe.denominator = pos08a20_mode_info[0].fps;
	sensor->streamcap.timeperframe.numerator = 1;

	chip_id = 0;
	os08a20_read_reg(sensor, OS08a20_CHIP_ID_HIGH_BYTE,&reg_val);
	chip_id |= reg_val << 8;
	os08a20_read_reg(sensor, OS08a20_CHIP_ID_LOW_BYTE, &reg_val);
	chip_id |= reg_val;
	if (chip_id != 0x5308) {
		pr_warn("camera os08a20 is not found\n");
		goto probe_err_power_down;
	}

	sd = &sensor->subdev;
	v4l2_i2c_subdev_init(sd, client, &os08a20_subdev_ops);
	sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	sd->entity.function = MEDIA_ENT_F_CAM_SENSOR;
	sensor->pads[OS08a20_SENS_PAD_SOURCE].flags = MEDIA_PAD_FL_SOURCE;

	retval = media_entity_pads_init(&sd->entity, OS08a20_SENS_PADS_NUM,
					sensor->pads);
	sd->entity.ops = &os08a20_sd_media_ops;
	if (retval < 0)
		goto probe_err_power_down;

	retval = v4l2_async_register_subdev_sensor_common(sd);
	if (retval < 0) {
		dev_err(&client->dev,"%s--Async register failed, ret=%d\n", __func__,retval);
		goto probe_err_entity_cleanup;
	}
	mutex_init(&sensor->lock);

	pr_info("%s camera mipi os08a20, is found\n", __func__);
	return 0;

probe_err_entity_cleanup:
	media_entity_cleanup(&sd->entity);
probe_err_power_down:
	os08a20_power_down(sensor);
	clk_disable_unprepare(sensor->sensor_clk);
probe_err_regulator_disable:
	os08a20_regulator_disable(sensor);
	return retval;
}

/*!
 * os08a20 I2C detach function
 *
 * @param client struct i2c_client *
 * @return	Error code indicating success or failure
 */
static int os08a20_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct os08a20 *sensor = client_to_os08a20(client);

	pr_info("enter %s\n", __func__);

	v4l2_async_unregister_subdev(sd);
	media_entity_cleanup(&sd->entity);
	os08a20_power_down(sensor);
	clk_disable_unprepare(sensor->sensor_clk);
	os08a20_regulator_disable(sensor);
	mutex_destroy(&sensor->lock);

	return 0;
}

module_i2c_driver(os08a20_i2c_driver);
MODULE_DESCRIPTION("OS08a20 MIPI Camera Subdev Driver");
MODULE_LICENSE("GPL");

int sensor_calc_gain(__u32 total_gain, __u32 *pagain, __u32 *pdgain)
{
	if (total_gain < 1024)
	{
		total_gain = 1024;
	}

	if(total_gain < (0x7c0 << 3))
	{
		*pdgain = 0x400;
		*pagain = (total_gain << 7) / *pdgain;//(128 * 1024)/1024 = 128

	}else
	{
		*pagain = 0x7c0;
		*pdgain = (total_gain << 7) / 0x7c0;
	}
	return 0;
}

int os08a20_s_long_gain(struct os08a20 *sensor, __u32 new_gain)
{
	__u32 again = 0;
	__u32 dgain = 0;
	int ret = 0;

	sensor_calc_gain(new_gain, &again, &dgain);

	ret |= os08a20_write_reg(sensor, 0x3508, (again >> 8) & 0xff);
	ret |= os08a20_write_reg(sensor, 0x3509, again & 0xff);

	ret |= os08a20_write_reg(sensor, 0x350a, (dgain >> 8) & 0xff);
	ret |= os08a20_write_reg(sensor, 0x350b, dgain & 0x00FF);

	return ret;
}

int os08a20_s_short_gain(struct os08a20 *sensor, __u32 new_gain)
{
	__u32 again = 0;
	__u32 dgain = 0;
	int ret = 0;

	sensor_calc_gain(new_gain, &again, &dgain);

	ret |= os08a20_write_reg(sensor, 0x350c, (again >> 8) & 0xff);
	ret |= os08a20_write_reg(sensor, 0x350d, again & 0xff);

	ret |= os08a20_write_reg(sensor, 0x350e, (dgain >> 8) & 0xff);
	ret |= os08a20_write_reg(sensor, 0x350f, dgain & 0x00FF);

	return ret;
}

int os08a20_s_long_exp(struct os08a20 *sensor, __u32 exp)
{
	int ret = 0;
	ret |= os08a20_write_reg(sensor, 0x3501, (exp >> 8) & 0xff);
	ret |= os08a20_write_reg(sensor, 0x3502, exp & 0xff);

	return ret;
}

int os08a20_s_short_exp(struct os08a20 *sensor, __u32 exp)
{
	int ret = 0;
	ret |= os08a20_write_reg(sensor, 0x3511, (exp >> 8) & 0xff);
	ret |= os08a20_write_reg(sensor, 0x3512, exp & 0xff);

	return ret;
}

int os08a20_g_gain(struct os08a20 *sensor, struct vvsensor_gain_context *gain)
{
	return 0;
}

int os08a20_g_version(struct os08a20 *sensor, __u32 *version)
{
	__u8 val = 0;

	os08a20_read_reg(sensor, 0x300a, &val);
	*version = val << 8;
	os08a20_read_reg(sensor, 0x300b, &val);
	*version |= val;
	return 0;
}


int os08a20_s_hdr(struct os08a20 *sensor, bool enable)
{
	pr_debug("%s: %d\n", __func__, enable);
	sensor->hdr = enable;
	return 0;
}

int os08a20_s_clk(struct os08a20 *sensor, __u32 clk)
{
	pr_debug("%s: %d\n", __func__, clk);
	os08a20_write_reg(sensor, 0x3005, clk);
	return 0;
}

int os08a20_g_clk(struct os08a20 *sensor, __u32 *clk)
{
	u8 val;
	os08a20_read_reg(sensor, 0x3005, &val);
	*clk = val;
	return 0;
}

int os08a20_s_fps(struct os08a20 *sensor, __u32 fps)
{
	u32 vts;
	pr_debug("%s: %d\n", __func__, fps);
	if (fps < 5) {
		fps = 5;
	}
	if (fps > sensor->cur_mode.fps){
		fps = sensor->cur_mode.fps;
	}
	sensor->fps = fps;

	vts =  sensor->cur_mode.fps * sensor->cur_mode.ae_info.DefaultFrameLengthLines / sensor->fps;

	os08a20_write_reg(sensor, 0x380e, (vts >> 8)&0xff);
	os08a20_write_reg(sensor, 0x380f, vts&0xff);

	if (sensor->hdr)
	{
		sensor->cur_mode.ae_info.cur_fps = sensor->fps;
		sensor->cur_mode.ae_info.CurFrameLengthLines = vts;
		sensor->cur_mode.ae_info.max_integration_time = vts - 64 - 4;
	}else
	{
		sensor->cur_mode.ae_info.cur_fps = sensor->fps;
		sensor->cur_mode.ae_info.CurFrameLengthLines = vts;
		sensor->cur_mode.ae_info.max_integration_time = vts - 8;
	}

	return 0;
}

int os08a20_g_fps(struct os08a20 *sensor,  __u32 *fps)
{
	*fps = sensor->fps;
	return 0;
}

int os08a20_g_chipid(struct os08a20 *sensor, __u32 *chip_id)
{
	int ret = 0;
	__u8 chip_id_high = 0;
	__u8 chip_id_low = 0;
	ret = os08a20_read_reg(sensor, 0x300a, &chip_id_high);
	ret |= os08a20_read_reg(sensor, 0x300b, &chip_id_low);

	*chip_id = ((chip_id_high & 0xff)<<8) | (chip_id_low & 0xff);
	return ret;
}

int os08a20_ioc_qcap(struct os08a20 *sensor, void *args)
{
	struct v4l2_capability *cap = (struct v4l2_capability *)args;

	strcpy((char *)cap->driver, "os08a20");
	sprintf((char *)cap->bus_info, "csi%d",sensor->csi);
	if(sensor->i2c_client->adapter)
	{//bus_info[8]-i2c bus dev number
		cap->bus_info[VVCAM_CAP_BUS_INFO_I2C_ADAPTER_NR_POS] = (__u8)sensor->i2c_client->adapter->nr;
	}
	else
	{
		cap->bus_info[VVCAM_CAP_BUS_INFO_I2C_ADAPTER_NR_POS] = 0xFF;
	}

	return 0;
}

int os08a20_ioc_query_mode(struct os08a20 *sensor, struct vvcam_mode_info_array *array)
{
	array->count = ARRAY_SIZE(pos08a20_mode_info);
#ifdef CONFIG_HARDENED_USERCOPY
	unsigned long copy_ret = 0;
	pr_debug("sensor %p\n", sensor);
	copy_ret = copy_to_user(&array->modes,pos08a20_mode_info,sizeof(pos08a20_mode_info));
#else
	memcpy(&array->modes,pos08a20_mode_info,sizeof(pos08a20_mode_info));
#endif
	return 0;
}

int os08a20_g_mode(struct os08a20 *sensor, struct vvcam_mode_info *pmode)
{
	int i = 0;
	struct vvcam_mode_info *pcur_mode = NULL;

	if (sensor->cur_mode.index == pmode->index &&
		sensor->cur_mode.width != 0 &&
		sensor->cur_mode.height != 0)
	{
		pcur_mode = &(sensor->cur_mode);
		memcpy(pmode,pcur_mode,sizeof(struct vvcam_mode_info));
		return 0;
	}

	for(i=0; i < ARRAY_SIZE(pos08a20_mode_info); i++)
	{
		if (pmode->index == pos08a20_mode_info[i].index)
		{
			pcur_mode = &pos08a20_mode_info[i];
			break;
		}
	}

	if (pcur_mode == NULL)
	{
		return -1;
	}

	memcpy(pmode,pcur_mode,sizeof(struct vvcam_mode_info));
	return 0;
}


/*
Use USER_TO_KERNEL/KERNEL_TO_USER to fix "uaccess" exception on run time.
Also, use "copy_ret" to fix the build issue as below.
error: ignoring return value of function declared with 'warn_unused_result' attribute.
*/

#ifdef CONFIG_HARDENED_USERCOPY
#define USER_TO_KERNEL(TYPE) \
	do {\
		TYPE tmp; \
		unsigned long copy_ret; \
		arg = (void *)(&tmp); \
		copy_ret = copy_from_user(arg, arg_user, sizeof(TYPE));\
	} while (0)

#define KERNEL_TO_USER(TYPE) \
	do {\
		unsigned long copy_ret; \
		copy_ret = copy_to_user(arg_user, arg, sizeof(TYPE));\
	} while (0)
#else
#define USER_TO_KERNEL(TYPE)
#define KERNEL_TO_USER(TYPE)
#endif

long os08a20_priv_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg_user)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct os08a20 *sensor = client_to_os08a20(client);
	struct vvcam_sccb_data reg;
	int ret = 0;
	void *arg = arg_user;

	/* pr_info("enter %s\n", __func__); */
	mutex_lock(&sensor->lock);
	switch (cmd) {
	case VVSENSORIOC_WRITE_REG: {
		USER_TO_KERNEL(struct vvcam_sccb_data);
		reg = *(struct vvcam_sccb_data *)arg;
		ret = os08a20_write_reg(sensor, reg.addr, (u8)reg.data) < 0;
		break;
	}
	case VVSENSORIOC_READ_REG: {
		struct vvcam_sccb_data *preg;
		u8 val;
		USER_TO_KERNEL(struct vvcam_sccb_data);
		preg = (struct vvcam_sccb_data *)arg;
		ret = os08a20_read_reg(sensor, (u16) preg->addr, &val) < 0;
		preg->data = val;
		KERNEL_TO_USER(struct vvcam_sccb_data);
		break;
	}
	case VVSENSORIOC_S_STREAM: {
		USER_TO_KERNEL(__u32);
		ret = os08a20_s_stream(sd, *(__u32 *)arg);
		break;
	}
	case VVSENSORIOC_S_EXP: {
		USER_TO_KERNEL(__u32);
		ret = os08a20_s_long_exp(sensor, *(__u32 *)arg);
		break;
	}
	case VVSENSORIOC_S_VSEXP: {
		USER_TO_KERNEL(__u32);
		ret = os08a20_s_short_exp(sensor, *(__u32 *)arg);
		break;
	}
	case VVSENSORIOC_S_GAIN: {
		USER_TO_KERNEL(__u32);
		ret = os08a20_s_long_gain(sensor, *(__u32 *)arg);
		break;
	}
	case VVSENSORIOC_S_VSGAIN: {
		USER_TO_KERNEL(__u32);
		ret = os08a20_s_short_gain(sensor, *(__u32 *)arg);
		break;
	}
	case VVSENSORIOC_S_FPS: {
		USER_TO_KERNEL(__u32);
		ret = os08a20_s_fps(sensor, *(__u32 *)arg);
		break;
	}
	case VVSENSORIOC_G_FPS: {
		USER_TO_KERNEL(__u32);
		ret = os08a20_g_fps(sensor, (__u32 *)arg);
		KERNEL_TO_USER(__u32);
		break;
	}
	case VVSENSORIOC_S_CLK: {
		USER_TO_KERNEL(__u32);
		//ret = os08a20_s_clk(sensor, *(__u32 *)arg);
		break;
	}
	case VVSENSORIOC_G_CLK: {
		USER_TO_KERNEL(__u32);
		ret = os08a20_g_clk(sensor, (__u32 *)arg);
		KERNEL_TO_USER(__u32);
		break;
	}
	case VIDIOC_QUERYCAP:
		ret = os08a20_ioc_qcap(sensor, arg);
		break;
	case VVSENSORIOC_G_CHIP_ID: {
		USER_TO_KERNEL(__u32);
		ret = os08a20_g_chipid(sensor, (__u32 *)arg);
		ret = (ret < 0) ? -1 : 0;
		KERNEL_TO_USER(__u32);
		break;
	}
	case VVSENSORIOC_G_RESERVE_ID: {
		__u32  correct_id = 0x5308;
		ret = copy_to_user(arg_user, &correct_id, sizeof(__u32));
		ret = ret? -1 : 0;
		break;
	}
	case VVSENSORIOC_S_HDR_MODE: {
		USER_TO_KERNEL(bool);
		ret = os08a20_s_hdr(sensor, *(bool *)arg);
		break;
	}
	case VVSENSORIOC_QUERY: {
		//USER_TO_KERNEL(struct vvcam_mode_info_array);
		os08a20_ioc_query_mode(sensor, arg);
		//KERNEL_TO_USER(struct vvcam_mode_info_array);
		break;
	}
	case VVSENSORIOC_G_SENSOR_MODE: {
		USER_TO_KERNEL(struct vvcam_mode_info);
		os08a20_g_mode(sensor, arg);
		KERNEL_TO_USER(struct vvcam_mode_info);
		break;
	}
	default:
		/* pr_err("unsupported os08a20 command %d.", cmd); */
		break;
	}
	mutex_unlock(&sensor->lock);

	return ret;
}
