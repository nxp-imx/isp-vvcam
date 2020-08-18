/*
 * Copyright (C) 2012-2015 Freescale Semiconductor, Inc. All Rights Reserved.
 * Copyright 2018 NXP
 * Copyright (c) 2020 VeriSilicon Holdings Co., Ltd.
 */
/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

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

#include "ov2775_regs_1080p.h"
#include "ov2775_regs_1080p_hdr.h"
#include "ov2775_regs_720p.h"

#define OV2775_VOLTAGE_ANALOG			2800000
#define OV2775_VOLTAGE_DIGITAL_CORE		1500000
#define OV2775_VOLTAGE_DIGITAL_IO		1800000

#define MIN_FPS 15
#define MAX_FPS 30
#define DEFAULT_FPS 30

#define OV2775_XCLK_MIN 6000000
#define OV2775_XCLK_MAX 24000000

#define OV2775_CHIP_ID_HIGH_BYTE	0x300A
#define OV2775_CHIP_ID_LOW_BYTE		0x300B

#define OV2775_SENS_PAD_SOURCE	0
#define OV2775_SENS_PADS_NUM	1

enum ov2775_mode {
	ov2775_mode_MIN = 0,
	ov2775_mode_1080P_1920_1080 = 0,
	ov2775_mode_720P_1280_720 = 1,
	ov2775_mode_NTSC_720_480 = 2,
	ov2775_mode_VGA_640_480 = 3,
	ov2775_mode_QVGA_320_240 = 4,
	ov2775_mode_QSXGA_2592_1944 = 5,
	ov2775_mode_MAX,
	ov2775_mode_INIT = 0xff,	/* only for sensor init */
};

enum ov2775_frame_rate {
	ov2775_15_fps,
	ov2775_30_fps
};

struct ov2775_datafmt {
	u32 code;
	enum v4l2_colorspace colorspace;
};

/* image size under 1280 * 960 are SUBSAMPLING
 * image size upper 1280 * 960 are SCALING
 */
enum ov2775_downsize_mode {
	SUBSAMPLING,
	SCALING,
};

struct ov2775_mode_info {
	enum ov2775_mode mode;
	enum ov2775_downsize_mode dn_mode;
	u32 width;
	u32 height;
	struct vvsensor_reg_value_t *init_data_ptr;
	u32 init_data_size;
	u32 bit_width;
	u32 fps;
	bool is_default;
};

struct ov2775_pll_info {
	enum ov2775_mode mode;
	struct vvsensor_reg_value_t *init_data_ptr;
	u32 init_data_size;
};

struct ov2775_hs_info {
	u32 width;
	u32 height;
	u32 frame_rate;
	u32 val;
};

struct ov2775 {
	struct v4l2_subdev subdev;
	struct v4l2_device *v4l2_dev;
	struct i2c_client *i2c_client;
	struct v4l2_pix_format pix;
	const struct ov2775_datafmt *fmt;
	struct v4l2_captureparm streamcap;
	struct media_pad pads[OV2775_SENS_PADS_NUM];
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
	u32 sclk;
	struct clk *sensor_clk;
	int csi;

	void (*io_init) (struct ov2775 *);
	int pwn_gpio, rst_gpio;
	bool hdr;
	int fps;

	u8 curr_modeinfo_index;
};

#define client_to_ov2775(client)\
	container_of(i2c_get_clientdata(client), struct ov2775, subdev)

long ov2775_priv_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg);

static int ov2775_framerates[] = {
	[ov2775_15_fps] = 15,
	[ov2775_30_fps] = 30,
};

static struct ov2775_mode_info ov2775_mode_info_data[4][ov2775_mode_MAX + 1] = {
	{
		{
			ov2775_mode_1080P_1920_1080, -1, 0, 0, NULL, 0, 0, 0, false},
		},
	/* linear mode */
	{
		{
			ov2775_mode_1080P_1920_1080, SCALING, 1920, 1080,
			ov2775_init_setting_1080p,
			ARRAY_SIZE(ov2775_init_setting_1080p), 12, 30, true,
		},

		{
			ov2775_mode_720P_1280_720, SCALING, 1280, 720,
			ov2775_init_setting_720p,
			ARRAY_SIZE(ov2775_init_setting_720p), 12, 30, false,
		},
	},

	/* hdr settings */
	{
		{
			ov2775_mode_1080P_1920_1080, SCALING, 1920, 1080,
			ov2775_init_setting_1080p_hdr,
			ARRAY_SIZE(ov2775_init_setting_1080p_hdr), 12, 30, false,
		},
	},

	/* hdr settings native*/
	{
	},

};

static struct vvcam_mode_info pov2775_mode_info[] = {
	{
		.index     = 0,
		.width     = 1920,
		.height    = 1080,
		.fps       = 30,
		.hdr_mode  = SENSOR_MODE_LINEAR,
		.bit_width = 12,
		.bayer_pattern = BAYER_BGGR,
	},
	{
		.index     = 1,
		.width    = 1920,
		.height   = 1080,
		.fps      = 30,
		.hdr_mode = SENSOR_MODE_HDR_STITCH,
		.stitching_mode = SENSOR_STITCHING_DUAL_DCG,
		.bit_width = 12,
		.bayer_pattern = BAYER_BGGR,
	},
	{
		.index     = 2,
		.width    = 1280,
		.height   = 720,
		.fps      = 60,
		.hdr_mode = SENSOR_MODE_LINEAR,
		.bit_width = 12,
		.bayer_pattern = BAYER_BGGR,
	}
};


static struct ov2775_hs_info hs_setting[] = {
	{1920, 1080, 30, 0x0B},
	{1920, 1080, 15, 0x10},
	{1280, 720,  30, 0x11},
};

static struct regulator *io_regulator;
static struct regulator *core_regulator;
static struct regulator *analog_regulator;

static int ov2775_probe(struct i2c_client *adapter,
			const struct i2c_device_id *device_id);
static int ov2775_remove(struct i2c_client *client);

static void ov2775_stop(struct ov2775 *sensor);

static const struct i2c_device_id ov2775_id[] = {
	{"ov2775", 0},
	{},
};

MODULE_DEVICE_TABLE(i2c, ov2775_id);

static const struct of_device_id ov2775_dt_ids[] = {
	{ .compatible = "ovti,ov2775" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, ov2775_dt_ids);

static struct i2c_driver ov2775_i2c_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name  = "ov2775",
		.of_match_table	= ov2775_dt_ids,
	},
	.probe  = ov2775_probe,
	.remove = ov2775_remove,
	.id_table = ov2775_id,
};

#if 0
static const struct ov2775_datafmt ov2775_colour_fmts[] = {
	{MEDIA_BUS_FMT_YVYU8_2X8, V4L2_COLORSPACE_JPEG},
	{MEDIA_BUS_FMT_UYVY8_2X8, V4L2_COLORSPACE_JPEG},
};
#else
static const struct ov2775_datafmt ov2775_colour_fmts[] = {
	{MEDIA_BUS_FMT_SBGGR12_1X12, V4L2_COLORSPACE_JPEG},
};
#endif

#if 0
static enum ov2775_frame_rate to_ov2775_frame_rate(struct v4l2_fract
						   *timeperframe)
{
	enum ov2775_frame_rate rate;
	u32 tgt_fps;		/* target frames per secound */

	pr_info("enter %s\n", __func__);
	tgt_fps = timeperframe->denominator / timeperframe->numerator;

	if (tgt_fps == 30)
		rate = ov2775_30_fps;
	else if (tgt_fps == 15)
		rate = ov2775_15_fps;
	else
		rate = -EINVAL;

	return rate;
}
#endif

static __u16 find_hs_configure(struct ov2775 *sensor)
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
static const struct ov2775_datafmt
*ov2775_find_datafmt(u32 code)
{
	int i;

	pr_debug("enter %s\n", __func__);
	for (i = 0; i < ARRAY_SIZE(ov2775_colour_fmts); i++)
		if (ov2775_colour_fmts[i].code == code)
			return ov2775_colour_fmts + i;

	return NULL;
}

static inline void ov2775_power_down(struct ov2775 *sensor, int enable)
{
	pr_debug("enter %s\n", __func__);
	if (!gpio_is_valid(sensor->pwn_gpio))
		return;

	gpio_set_value_cansleep(sensor->pwn_gpio, enable);
	udelay(2000);
}

static inline void ov2775_reset(struct ov2775 *sensor)
{
	pr_debug("enter %s\n", __func__);
	if (sensor->pwn_gpio < 0 || sensor->rst_gpio < 0)
		return;

	gpio_set_value_cansleep(sensor->pwn_gpio, 1);
	gpio_set_value_cansleep(sensor->rst_gpio, 0);
	udelay(5000);

	gpio_set_value_cansleep(sensor->pwn_gpio, 0);
	udelay(1000);

	gpio_set_value_cansleep(sensor->rst_gpio, 1);
	msleep(20);
}

static int ov2775_regulator_enable(struct device *dev)
{
	int ret = 0;

	pr_debug("enter %s\n", __func__);
	io_regulator = devm_regulator_get(dev, "DOVDD");
	if (!IS_ERR(io_regulator)) {
		regulator_set_voltage(io_regulator,
				      OV2775_VOLTAGE_DIGITAL_IO,
				      OV2775_VOLTAGE_DIGITAL_IO);
		ret = regulator_enable(io_regulator);
		if (ret) {
			dev_err(dev, "set io voltage failed\n");
			return ret;
		} else {
			dev_dbg(dev, "set io voltage ok\n");
		}
	} else {
		io_regulator = NULL;
		dev_warn(dev, "cannot get io voltage\n");
	}

	core_regulator = devm_regulator_get(dev, "DVDD");
	if (!IS_ERR(core_regulator)) {
		regulator_set_voltage(core_regulator,
				      OV2775_VOLTAGE_DIGITAL_CORE,
				      OV2775_VOLTAGE_DIGITAL_CORE);
		ret = regulator_enable(core_regulator);
		if (ret) {
			dev_err(dev, "set core voltage failed\n");
			return ret;
		} else {
			dev_dbg(dev, "set core voltage ok\n");
		}
	} else {
		core_regulator = NULL;
		dev_warn(dev, "cannot get core voltage\n");
	}

	analog_regulator = devm_regulator_get(dev, "AVDD");
	if (!IS_ERR(analog_regulator)) {
		regulator_set_voltage(analog_regulator,
				      OV2775_VOLTAGE_ANALOG,
				      OV2775_VOLTAGE_ANALOG);
		ret = regulator_enable(analog_regulator);
		if (ret)
			dev_err(dev, "set analog voltage failed\n");
		else
			dev_dbg(dev, "set analog voltage ok\n");
	} else {
		analog_regulator = NULL;
		dev_warn(dev, "cannot get analog voltage\n");
	}

	return ret;
}

s32 ov2775_write_reg(struct ov2775 *sensor, u16 reg, u8 val)
{
	struct device *dev = &sensor->i2c_client->dev;
	u8 au8Buf[3] = { 0 };

	au8Buf[0] = reg >> 8;
	au8Buf[1] = reg & 0xff;
	au8Buf[2] = val;

	if (i2c_master_send(sensor->i2c_client, au8Buf, 3) < 0) {
		dev_err(dev, "Write reg error: reg=%x, val=%x\n", reg, val);
		return -1;
	}

	return 0;
}

s32 ov2775_read_reg(struct ov2775 *sensor, u16 reg, u8 *val)
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

static int ov2775_set_clk_rate(struct ov2775 *sensor)
{
	u32 tgt_xclk;		/* target xclk */
	int ret;

	/* mclk */
	tgt_xclk = sensor->mclk;
	tgt_xclk = min_t(u32, tgt_xclk, (u32) OV2775_XCLK_MAX);
	tgt_xclk = max_t(u32, tgt_xclk, (u32) OV2775_XCLK_MIN);
	sensor->mclk = tgt_xclk;

	pr_debug("   Setting mclk to %d MHz\n", tgt_xclk / 1000000);
	ret = clk_set_rate(sensor->sensor_clk, sensor->mclk);
	if (ret < 0)
		pr_debug("set rate filed, rate=%d\n", sensor->mclk);
	return ret;
}

/* download ov2775 settings to sensor through i2c */
static int ov2775_download_firmware(struct ov2775 *sensor,
				    struct vvsensor_reg_value_t *mode_setting,
				    s32 size)
{
	register u32 delay_ms = 0;
	register u16 reg_addr = 0;
	register u8 mask = 0;
	register u8 val = 0;
	u8 reg_val = 0;
	int i, retval = 0;

	pr_debug("enter %s\n", __func__);
	for (i = 0; i < size; ++i, ++mode_setting) {
		delay_ms = mode_setting->delay;
		reg_addr = mode_setting->addr;
		val = mode_setting->val;
		mask = mode_setting->mask;

		if (mask) {
			retval = ov2775_read_reg(sensor, reg_addr, &reg_val);
			if (retval < 0)
				break;

			reg_val &= ~(u8)mask;
			val &= mask;
			val |= reg_val;
		}

		retval = ov2775_write_reg(sensor, reg_addr, val);
		if (retval < 0)
			break;

		if (delay_ms)
			msleep(delay_ms);
	}

	return retval;
}

static void ov2775_start(struct ov2775 *sensor)
{
	pr_debug("enter %s\n", __func__);
#if 1
	ov2775_write_reg(sensor, 0x3012, 0x01);
#else
	ov2775_write_reg(sensor, 0x3008, 0x02);
	ov2775_write_reg(sensor, 0x4202, 0x00);
#endif
	/* Color bar control */
	/* ov2775_write_reg(sensor, 0x503d, 0x80); */

	/* skip the first three frame for 30fps */
	msleep(100);
}

#if 0
static int ov2775_change_mode(struct ov2775 *sensor)
{
	struct reg_value *mode_setting = NULL;
	enum ov2775_mode mode = sensor->streamcap.capturemode;
	enum ov2775_frame_rate frame_rate =
	    to_ov2775_frame_rate(&sensor->streamcap.timeperframe);
	int ArySize = 0, retval = 0;

	pr_debug("enter %s\n", __func__);
	if (mode > ov2775_mode_MAX || mode < ov2775_mode_MIN) {
		pr_err("Wrong ov2775 mode detected!\n");
		return -1;
	}

	mode_setting = ov2775_mode_info_data[frame_rate][mode].init_data_ptr;
	ArySize = ov2775_mode_info_data[frame_rate][mode].init_data_size;

	sensor->pix.width = ov2775_mode_info_data[frame_rate][mode].width;
	sensor->pix.height = ov2775_mode_info_data[frame_rate][mode].height;

	if (sensor->pix.width == 0 || sensor->pix.height == 0 ||
	    mode_setting == NULL || ArySize == 0) {
		pr_err("Not support mode=%d %s\n", mode,
		       (frame_rate == 0) ? "15(fps)" : "30(fps)");
		return -EINVAL;
	}

	retval = ov2775_download_firmware(sensor, mode_setting, ArySize);

	return retval;
}
#endif

static void ov2775_stop(struct ov2775 *sensor)
{
	pr_debug("enter %s\n", __func__);
#if 1
	ov2775_write_reg(sensor, 0x3012, 0x00);
#else
	ov2775_write_reg(sensor, 0x4202, 0x0f);
	ov2775_write_reg(sensor, 0x3008, 0x42);
	ov2775_write_reg(sensor, 0x4800, 0x24);
#endif
}

/*!
 * ov2775_s_power - V4L2 sensor interface handler for VIDIOC_S_POWER ioctl
 * @s: pointer to standard V4L2 device structure
 * @on: indicates power mode (on or off)
 *
 * Turns the power on or off, depending on the value of on and returns the
 * appropriate error code.
 */
static int ov2775_s_power(struct v4l2_subdev *sd, int on)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov2775 *sensor = client_to_ov2775(client);

	pr_debug("enter %s\n", __func__);
	if (on)
		clk_prepare_enable(sensor->sensor_clk);
	else
		clk_disable_unprepare(sensor->sensor_clk);

	sensor->on = on;
	return 0;
}

/*!
 * ov2775_g_parm - V4L2 sensor interface handler for VIDIOC_G_PARM ioctl
 * @s: pointer to standard V4L2 sub device structure
 * @a: pointer to standard V4L2 VIDIOC_G_PARM ioctl structure
 *
 * Returns the sensor's video CAPTURE parameters.
 */
static int ov2775_g_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *a)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov2775 *sensor = client_to_ov2775(client);
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
		pr_debug("   type is unknown - %d\n", a->type);
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
 * Configures the sensor to use the input parameters, if possible.  If
 * not possible, reverts to the old parameters and returns the
 * appropriate error code.
 */
static int ov2775_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *a)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov2775 *sensor = client_to_ov2775(client);
	struct v4l2_fract *timeperframe = &a->parm.capture.timeperframe;
	u32 tgt_fps;		/* target frames per secound */
	enum ov2775_mode mode = a->parm.capture.capturemode;
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

		if (mode > ov2775_mode_MAX || mode < ov2775_mode_MIN) {
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
		pr_debug("   type is not V4L2_BUF_TYPE_VIDEO_CAPTURE but %d\n",
			 a->type);
		ret = -EINVAL;
		break;

	default:
		pr_debug("   type is unknown - %d\n", a->type);
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int ov2775_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov2775 *sensor = client_to_ov2775(client);

	pr_debug("enter %s\n", __func__);
	if (enable)
		ov2775_start(sensor);
	else
		ov2775_stop(sensor);

	sensor->on = enable;
	return 0;
}

#if 0
static struct ov2775_mode_info *get_max_resolution(enum ov2775_frame_rate rate)
{
	u32 max_width;
	enum ov2775_mode mode;
	int i;

	pr_debug("enter %s\n", __func__);
	mode = 0;
	max_width = ov2775_mode_info_data[rate][0].width;

	for (i = 0; i < (ov2775_mode_MAX + 1); i++) {
		if (ov2775_mode_info_data[rate][i].width > max_width) {
			max_width = ov2775_mode_info_data[rate][i].width;
			mode = i;
		}
	}
	return &ov2775_mode_info_data[rate][mode];
}

static struct ov2775_mode_info *match(struct v4l2_mbus_framefmt *fmt,
				      enum ov2775_frame_rate rate)
{
	struct ov2775_mode_info *info;
	int i;

	pr_debug("enter %s\n", __func__);
	for (i = 0; i < (ov2775_mode_MAX + 1); i++) {
		if (fmt->width == ov2775_mode_info_data[rate][i].width &&
		    fmt->height == ov2775_mode_info_data[rate][i].height) {
			info = &ov2775_mode_info_data[rate][i];
			break;
		}
	}
	if (i == ov2775_mode_MAX + 1)
		info = NULL;

	return info;
}

static void try_to_find_resolution(struct ov2775 *sensor,
				   struct v4l2_mbus_framefmt *mf)
{
	enum ov2775_mode mode = sensor->streamcap.capturemode;
	struct v4l2_fract *timeperframe = &sensor->streamcap.timeperframe;
	enum ov2775_frame_rate frame_rate = to_ov2775_frame_rate(timeperframe);
	struct device *dev = &sensor->i2c_client->dev;
	struct ov2775_mode_info *info;
	bool found = false;

	pr_debug("enter %s\n", __func__);
	if ((mf->width == ov2775_mode_info_data[frame_rate][mode].width) &&
	    (mf->height == ov2775_mode_info_data[frame_rate][mode].height)) {
		info = &ov2775_mode_info_data[frame_rate][mode];
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

static int ov2775_set_fmt(struct v4l2_subdev *sd,
			  struct v4l2_subdev_pad_config *cfg,
			  struct v4l2_subdev_format *format)
{
	struct v4l2_mbus_framefmt *mf = &format->format;
	const struct ov2775_datafmt *fmt = ov2775_find_datafmt(mf->code);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov2775 *sensor = client_to_ov2775(client);
	unsigned int i;
	struct vvsensor_reg_value_t *mode_setting = NULL;
	int array_size = 0;

	pr_debug("enter %s\n", __func__);
	if (format->pad) {
		return -EINVAL;
	}

	if (!fmt) {
		mf->code = ov2775_colour_fmts[0].code;
		mf->colorspace = ov2775_colour_fmts[0].colorspace;
	}

	mf->field = V4L2_FIELD_NONE;
	/*  old search method,  vsi need change to
	    search resolution by width/height */
	/*  try_to_find_resolution(sensor, mf); */
	if (format->which == V4L2_SUBDEV_FORMAT_TRY)
		return 0;

	if (sensor->hdr) {
		for (i = 0; i < ARRAY_SIZE(ov2775_mode_info_data[2]); i++) {
		if (ov2775_mode_info_data[2][i].width == mf->width &&
			ov2775_mode_info_data[2][i].height == mf->height) {
				mode_setting = ov2775_mode_info_data[2][i].init_data_ptr;
				array_size = ov2775_mode_info_data[2][i].init_data_size;
				return ov2775_download_firmware(sensor, mode_setting, array_size);
			}
		}
	} else {
		for (i = 0; i < ARRAY_SIZE(ov2775_mode_info_data[1]); i++) {
		if (ov2775_mode_info_data[1][i].width == mf->width &&
			ov2775_mode_info_data[1][i].height == mf->height) {
				mode_setting = ov2775_mode_info_data[1][i].init_data_ptr;
				array_size = ov2775_mode_info_data[1][i].init_data_size;
				return ov2775_download_firmware(sensor, mode_setting, array_size);
			}
		}
	}

	pr_err("%s search error: %d %d\n", __func__, mf->width, mf->height);
	return -EINVAL;;
}

static int ov2775_get_fmt(struct v4l2_subdev *sd,
			  struct v4l2_subdev_pad_config *cfg,
			  struct v4l2_subdev_format *format)
{
	struct v4l2_mbus_framefmt *mf = &format->format;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov2775 *sensor = client_to_ov2775(client);

	pr_debug("enter %s\n", __func__);
	if (format->pad)
		return -EINVAL;

	memset(mf, 0, sizeof(struct v4l2_mbus_framefmt));

	mf->code = ov2775_colour_fmts[0].code;
	mf->colorspace = ov2775_colour_fmts[0].colorspace;
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

static int ov2775_enum_code(struct v4l2_subdev *sd,
			    struct v4l2_subdev_pad_config *cfg,
			    struct v4l2_subdev_mbus_code_enum *code)
{
	pr_debug("enter %s\n", __func__);
	if (code->pad || code->index >= ARRAY_SIZE(ov2775_colour_fmts))
		return -EINVAL;

	code->code = ov2775_colour_fmts[code->index].code;
	return 0;
}

/*!
 * ov2775_enum_framesizes - V4L2 sensor interface handler for
 *			   VIDIOC_ENUM_FRAMESIZES ioctl
 * @s: pointer to standard V4L2 device structure
 * @fsize: standard V4L2 VIDIOC_ENUM_FRAMESIZES ioctl structure
 *
 * Return 0 if successful, otherwise -EINVAL.
 */
static int ov2775_enum_framesizes(struct v4l2_subdev *sd,
				  struct v4l2_subdev_pad_config *cfg,
				  struct v4l2_subdev_frame_size_enum *fse)
{
	pr_debug("enter %s\n", __func__);
	if (fse->index > ov2775_mode_MAX)
		return -EINVAL;

	fse->max_width =
	    max(ov2775_mode_info_data[0][fse->index].width,
		ov2775_mode_info_data[1][fse->index].width);
	fse->min_width = fse->max_width;

	fse->max_height =
	    max(ov2775_mode_info_data[0][fse->index].height,
		ov2775_mode_info_data[1][fse->index].height);
	fse->min_height = fse->max_height;

	return 0;
}

/*!
 * ov2775_enum_frameintervals - V4L2 sensor interface handler for
 *				   VIDIOC_ENUM_FRAMEINTERVALS ioctl
 * @s: pointer to standard V4L2 device structure
 * @fival: standard V4L2 VIDIOC_ENUM_FRAMEINTERVALS ioctl structure
 *
 * Return 0 if successful, otherwise -EINVAL.
 */
static int ov2775_enum_frameintervals(struct v4l2_subdev *sd,
				      struct v4l2_subdev_pad_config *cfg,
				      struct v4l2_subdev_frame_interval_enum
				      *fie)
{
	int i, j, count;

	pr_debug("enter %s\n", __func__);
	if (fie->index < 0 || fie->index > ov2775_mode_MAX)
		return -EINVAL;

	if (fie->width == 0 || fie->height == 0 || fie->code == 0) {
		pr_warn("Please assign pixel format, width and height.\n");
		return -EINVAL;
	}

	fie->interval.numerator = 1;

	count = 0;
	for (i = 0; i < ARRAY_SIZE(ov2775_framerates); i++) {
		for (j = 0; j < (ov2775_mode_MAX + 1); j++) {
			if (fie->width == ov2775_mode_info_data[i][j].width
			    && fie->height == ov2775_mode_info_data[i][j].height
			    && ov2775_mode_info_data[i][j].init_data_ptr !=
			    NULL) {
				count++;
			}
			if (fie->index == (count - 1)) {
				fie->interval.denominator =
				    ov2775_framerates[i];
				return 0;
			}
		}
	}

	return -EINVAL;
}

static int ov2775_link_setup(struct media_entity *entity,
			     const struct media_pad *local,
			     const struct media_pad *remote, u32 flags)
{
	return 0;
}

static struct v4l2_subdev_video_ops ov2775_subdev_video_ops = {
	.g_parm = ov2775_g_parm,
	.s_parm = ov2775_s_parm,
	.s_stream = ov2775_s_stream,
};

static const struct v4l2_subdev_pad_ops ov2775_subdev_pad_ops = {
	.enum_frame_size = ov2775_enum_framesizes,
	.enum_frame_interval = ov2775_enum_frameintervals,
	.enum_mbus_code = ov2775_enum_code,
	.set_fmt = ov2775_set_fmt,
	.get_fmt = ov2775_get_fmt,
};

static struct v4l2_subdev_core_ops ov2775_subdev_core_ops = {
	.s_power = ov2775_s_power,
	.ioctl = ov2775_priv_ioctl,
};

static struct v4l2_subdev_ops ov2775_subdev_ops = {
	.core = &ov2775_subdev_core_ops,
	.video = &ov2775_subdev_video_ops,
	.pad = &ov2775_subdev_pad_ops,
};

static const struct media_entity_operations ov2775_sd_media_ops = {
	.link_setup = ov2775_link_setup,
};

/*!
 * ov2775 I2C probe function
 *
 * @param adapter struct i2c_adapter *
 * @return  Error code indicating success or failure
 */

static int ov2775_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	struct v4l2_subdev *sd;
	int retval;

	u8 chip_id_high, chip_id_low;
	struct ov2775 *sensor;

	pr_debug("enter %s\n", __func__);
	sensor = devm_kmalloc(dev, sizeof(*sensor), GFP_KERNEL);
	if (!sensor)
		return -ENOMEM;
	/* Set initial values for the sensor struct. */
	memset(sensor, 0, sizeof(*sensor));

	/* request power down pin */
	sensor->pwn_gpio = of_get_named_gpio(dev->of_node, "pwn-gpios", 0);
	if (!gpio_is_valid(sensor->pwn_gpio))
		dev_warn(dev, "No sensor pwdn pin available");
	else {
		retval = devm_gpio_request_one(dev, sensor->pwn_gpio,
					       GPIOF_OUT_INIT_HIGH,
					       "ov2775_mipi_pwdn");
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
					       "ov2775_mipi_reset");
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
	mdelay(2);

	if (gpio_is_valid(sensor->rst_gpio)) {
		gpio_set_value_cansleep(sensor->rst_gpio, 0);
		mdelay(2);
	}

	/* Set mclk rate before clk on */
	ov2775_set_clk_rate(sensor);

	retval = clk_prepare_enable(sensor->sensor_clk);
	if (retval < 0) {
		dev_err(dev, "%s: enable sensor clk fail\n", __func__);
		return -EINVAL;
	}
	mdelay(2);

	if (gpio_is_valid(sensor->rst_gpio)) {
		gpio_set_value_cansleep(sensor->rst_gpio, 1);
		msleep(20);
	}

	sensor->io_init = ov2775_reset;
	sensor->i2c_client = client;

	sensor->pix.pixelformat = V4L2_PIX_FMT_UYVY;
	sensor->pix.width = ov2775_mode_info_data[1][0].width;
	sensor->pix.height = ov2775_mode_info_data[1][0].height;
	sensor->streamcap.capability = V4L2_MODE_HIGHQUALITY |
	    V4L2_CAP_TIMEPERFRAME;
	sensor->streamcap.capturemode = 0;
	sensor->streamcap.timeperframe.denominator = DEFAULT_FPS;
	sensor->streamcap.timeperframe.numerator = 1;

	ov2775_regulator_enable(&client->dev);

	ov2775_power_down(sensor, 1);

	retval = ov2775_read_reg(sensor, OV2775_CHIP_ID_HIGH_BYTE,
				 &chip_id_high);

	if (retval < 0 || chip_id_high != 0x27) {
		clk_disable_unprepare(sensor->sensor_clk);
		pr_warn("camera ov2775 is not found\n");
		return -ENODEV;
	}
	retval = ov2775_read_reg(sensor, OV2775_CHIP_ID_LOW_BYTE, &chip_id_low);
	if (retval < 0 || chip_id_low != 112) {
		clk_disable_unprepare(sensor->sensor_clk);
		pr_warn("camera ov2775 is not found\n");
		return -ENODEV;
	}

	sd = &sensor->subdev;
	v4l2_i2c_subdev_init(sd, client, &ov2775_subdev_ops);
	sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	sd->entity.function = MEDIA_ENT_F_CAM_SENSOR;
	sensor->pads[OV2775_SENS_PAD_SOURCE].flags = MEDIA_PAD_FL_SOURCE;

	retval = media_entity_pads_init(&sd->entity, OV2775_SENS_PADS_NUM,
					sensor->pads);
	sd->entity.ops = &ov2775_sd_media_ops;
	if (retval < 0)
		return retval;

	retval = v4l2_async_register_subdev_sensor_common(sd);
	if (retval < 0) {
		dev_err(&client->dev,
			"%s--Async register failed, ret=%d\n", __func__,
			retval);
		media_entity_cleanup(&sd->entity);
	}

	pr_info("%s camera mipi ov2775, is found\n", __func__);

	return retval;
}

/*!
 * ov2775 I2C detach function
 *
 * @param client struct i2c_client *
 * @return  Error code indicating success or failure
 */
static int ov2775_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct ov2775 *sensor = client_to_ov2775(client);

	pr_info("enter %s\n", __func__);

	v4l2_async_unregister_subdev(sd);

	/* clk_unprepare(sensor->sensor_clk); */
	ov2775_power_down(sensor, 1);
	if (analog_regulator)
		regulator_disable(analog_regulator);

	if (core_regulator)
		regulator_disable(core_regulator);

	if (io_regulator)
		regulator_disable(io_regulator);

	return 0;
}

module_i2c_driver(ov2775_i2c_driver);
MODULE_DESCRIPTION("OV2775 MIPI Camera Subdev Driver");
MODULE_LICENSE("GPL");

int sensor_calc_gain(__u32 total_gain, __u32 *pagain, __u32 *pdgain, __u32 *phcg)
{
	if (total_gain <= 3072)
		total_gain = 3072;
	else if ((total_gain >= 22528) && (total_gain < 23552))
		total_gain = 22528;

	if (total_gain < 4480) {
		*pagain = 1;
		*phcg = 1;
	} else if (total_gain < 8960) {
		*pagain = 2;
		*phcg = 1;
	} else if (total_gain < 22528) {
		*pagain = 3;
		*phcg = 1;
	} else if (total_gain < 44990) {
		*pagain = 0;
		*phcg = 11;
	} else if (total_gain < 89320) {
		*pagain = 1;
		*phcg = 11;
	} else if (total_gain < 179498) {
		*pagain = 2;
		*phcg = 11;
	} else {
		*pagain = 3;
		*phcg = 11;
	}

	*pdgain = ((total_gain << 8) >> (*pagain)) / (1024 * (*phcg));
	return 0;
}

int ov2775_s_gain(struct ov2775 *sensor, __u32 new_gain)
{
	__u32 again = 0;
	__u32 dgain, hcg;
	__u8 reg_val;

	__u32 hdr_radio = 16;
	__u32 hcg_gain;
	__u32 lcg_gain;

	__u32 hcg_again = 0;
	__u32 hcg_dgain = 0;
	__u32 lcg_again = 0;
	__u32 lcg_dgain = 0;

	int ret = 0;



	if (sensor->hdr == 0) {
		sensor_calc_gain(new_gain, &again, &dgain, &hcg);
		ret = ov2775_read_reg(sensor, 0x30bb, &reg_val);
		if (hcg == 1) {
			reg_val &= ~(1 << 6);
		} else {
			reg_val |= (1 << 6);
		}
		reg_val &= ~0x03;
		reg_val |= again;
		ret = ov2775_write_reg(sensor, 0x3467, 0x00);
		ret |= ov2775_write_reg(sensor, 0x3464, 0x04);
		ret |= ov2775_write_reg(sensor, 0x315c, (dgain >> 8) & 0xff);
		ret |= ov2775_write_reg(sensor, 0x315d, dgain & 0xff);
		ret |= ov2775_write_reg(sensor, 0x30bb, reg_val);
		ret |= ov2775_write_reg(sensor, 0x3464, 0x14);
		ret |= ov2775_write_reg(sensor, 0x3467, 0x01);
	}else {
		hcg_gain = new_gain * hdr_radio / 11;
		sensor_calc_gain(hcg_gain, &hcg_again, &hcg_dgain, &hcg);
		lcg_gain = new_gain;
		sensor_calc_gain(lcg_gain, &lcg_again, &lcg_dgain, &hcg);

		ret = ov2775_read_reg(sensor, 0x30bb, &reg_val);
		reg_val &= ~0x0f;
		reg_val |= (lcg_again & 0x03) << 2;
		reg_val |= hcg_again  & 0x03;

		ret  = ov2775_write_reg(sensor, 0x3467, 0x00);
		ret |= ov2775_write_reg(sensor, 0x3464, 0x04);

		ret |= ov2775_write_reg(sensor, 0x315a, (hcg_dgain>>8) & 0xff);
		ret |= ov2775_write_reg(sensor, 0x315b, hcg_dgain & 0xff);

		ret |= ov2775_write_reg(sensor, 0x315c, (lcg_dgain>>8) & 0xff);
		ret |= ov2775_write_reg(sensor, 0x315d, lcg_dgain & 0xff);

		ret |= ov2775_write_reg(sensor, 0x30bb, reg_val);
		ret |= ov2775_write_reg(sensor, 0x3464, 0x14);
		ret |= ov2775_write_reg(sensor, 0x3467, 0x01);
	}
	return ret;
}

int ov2775_s_vsgain(struct ov2775 *sensor, __u32 new_gain)
{
	__u32 again = 0;
	__u32 dgain, hcg;
	__u8 reg_val;

	sensor_calc_gain(new_gain, &again, &dgain, &hcg);

	ov2775_read_reg(sensor, 0x30bb, &reg_val);
	reg_val &= ~0x30;
	reg_val |= (again & 0x03) << 4;

	ov2775_write_reg(sensor, 0x3467, 0x00);
	ov2775_write_reg(sensor, 0x3464, 0x04);

	ov2775_write_reg(sensor, 0x315e, (dgain & 0xFF00) >> 8);
	ov2775_write_reg(sensor, 0x315f, dgain & (0x00FF));
	ov2775_write_reg(sensor, 0x30bb, reg_val);

	ov2775_write_reg(sensor, 0x3464, 0x14);
	ov2775_write_reg(sensor, 0x3467, 0x01);
	return 0;
}

int ov2775_s_exp(struct ov2775 *sensor, __u32 exp)
{
	/* pr_info("enter %s 0x%08x\n", __func__, exp); */
	ov2775_write_reg(sensor, 0x3467, 0x00);
	ov2775_write_reg(sensor, 0x3464, 0x04);

	ov2775_write_reg(sensor, 0x30b6, (exp & 0xFF00) >> 8);
	ov2775_write_reg(sensor, 0x30b7, exp & 0x00FF);

	ov2775_write_reg(sensor, 0x3464, 0x14);
	ov2775_write_reg(sensor, 0x3467, 0x01);
	return 0;
}

int ov2775_s_vsexp(struct ov2775 *sensor, __u32 exp)
{
	/* pr_info("enter %s 0x%08x\n", __func__, exp); */
	ov2775_write_reg(sensor, 0x3467, 0x00);
	ov2775_write_reg(sensor, 0x3464, 0x04);

	ov2775_write_reg(sensor, 0x30b8, (exp & 0xFF00) >> 8);
	ov2775_write_reg(sensor, 0x30b9, exp & 0x00FF);

	ov2775_write_reg(sensor, 0x3464, 0x14);
	ov2775_write_reg(sensor, 0x3467, 0x01);
	return 0;
}

int ov2775_g_gain(struct ov2775 *sensor, struct vvsensor_gain_context *gain)
{
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

int ov2775_g_version(struct ov2775 *sensor, __u32 *version)
{
	__u8 val = 0;

	ov2775_read_reg(sensor, 0x300a, &val);
	*version = val << 8;
	ov2775_read_reg(sensor, 0x300b, &val);
	*version |= val;
	return 0;
}


int ov2775_s_hdr(struct ov2775 *sensor, bool enable)
{
	pr_debug("%s: %d\n", __func__, enable);
	sensor->hdr = enable;
	return 0;
}

int ov2775_s_clk(struct ov2775 *sensor, __u32 clk)
{
	pr_debug("%s: %d\n", __func__, clk);
	ov2775_write_reg(sensor, 0x3005, clk);
	return 0;
}

int ov2775_g_clk(struct ov2775 *sensor, __u32 *clk)
{
	u8 val;
	ov2775_read_reg(sensor, 0x3005, &val);
	*clk = val;
	return 0;
}

int ov2775_g_sclk(struct ov2775 *sensor, __u32 *sclk_hz)
{
	int ret = 0;
	u8 pclk_pll_pre;//0x3004[2:0]
	u8 pclk_pll_pdiv;//0x3005[7:0]
	u8 pclk_pll_sdiv;//0x3006[0]
	u8 pclk_pll_post;//0x3007[3:0]
	u32 pclk_pll_m;
	u8  pclk_pll_pre_num, pclk_pll_pre_deno;
	ret = ov2775_read_reg(sensor, 0x3004, &pclk_pll_pre);
	ret |= ov2775_read_reg(sensor, 0x3005, &pclk_pll_pdiv);
	ret |= ov2775_read_reg(sensor, 0x3006, &pclk_pll_sdiv);
	ret |= ov2775_read_reg(sensor, 0x3007, &pclk_pll_post);
	switch(pclk_pll_pre&0x07)//0x3004[2:0]
	{
	case 0: pclk_pll_pre_num = 1; pclk_pll_pre_deno = 1; break;
	case 1: pclk_pll_pre_num = 2; pclk_pll_pre_deno = 3; break;
	case 2: pclk_pll_pre_num = 1; pclk_pll_pre_deno = 2; break;
	case 3: pclk_pll_pre_num = 2; pclk_pll_pre_deno = 5; break;
	case 4: pclk_pll_pre_num = 1; pclk_pll_pre_deno = 3; break;
	case 5: pclk_pll_pre_num = 1; pclk_pll_pre_deno = 4; break;
	case 6: pclk_pll_pre_num = 1; pclk_pll_pre_deno = 6; break;
	default: pclk_pll_pre_num = 1; pclk_pll_pre_deno = 8; break;
	}
	pclk_pll_post = (pclk_pll_post&0x0F) + 1;
	pclk_pll_m = 2*pclk_pll_pdiv + (pclk_pll_sdiv&0x01) + 4;
	*sclk_hz = (sensor->mclk/1000*pclk_pll_m*pclk_pll_pre_num/(pclk_pll_pre_deno*pclk_pll_post))*1000;
	sensor->sclk = *sclk_hz;
	return ret;
}

int ov2775_s_fps(struct ov2775 *sensor, __u32 fps)
{
	int ret = 0;
	u32 sclk;
	u8  val;
	u16 hts;
	u32 vts;
	pr_debug("%s: %d\n", __func__, fps);
	if (sensor->hdr)
		return 0;
	if (fps < 5) {
		fps = 5;
	}
	ov2775_g_sclk(sensor, &sclk);

	ret = ov2775_read_reg(sensor, 0x30B0, &val);
	hts = (u16)val;
	ret |= ov2775_read_reg(sensor, 0x30B1, &val);
	hts = (hts << 8) | val;
	if(ret >= 0)
	{
		vts = sclk/(fps*hts);
		ov2775_write_reg(sensor, 0x30B2, (u8)(vts >> 8));
		ov2775_write_reg(sensor, 0x30B3, (u8)(vts & 0xff));
		sensor->fps = fps;
	}

	return 0;
}

int ov2775_g_fps(struct ov2775 *sensor,  __u32 *fps)
{
	*fps = sensor->fps;
	return 0;
}

int ov2775_g_chipid(struct ov2775 *sensor, __u32 *chip_id)
{
	int ret = 0;
	__u8 chip_id_high = 0;
	__u8 chip_id_low = 0;
	ret = ov2775_read_reg(sensor, 0x300a, &chip_id_high);
	ret |= ov2775_read_reg(sensor, 0x300b, &chip_id_low);

	*chip_id = ((chip_id_high & 0xff)<<8) | (chip_id_low & 0xff);
	return ret;
}

int ov2775_ioc_qcap(void *dev, void *args)
{
	struct v4l2_capability *cap = (struct v4l2_capability *)args;

	strcpy((char *)cap->driver, "ov2775");
	return 0;
}

int ov2775_ioc_query_mode(struct ov2775 *sensor, struct vvcam_mode_info_arry *array)
{
	array->count = ARRAY_SIZE(pov2775_mode_info);
	memcpy(&array->modes,pov2775_mode_info,sizeof(pov2775_mode_info));

	return 0;
}

int ov2775_g_mode(struct ov2775 *sensor, struct vvcam_mode_info *pmode)
{
	int i = 0;
	struct vvcam_mode_info *pcur_mode = NULL;
	for(i=0; i < ARRAY_SIZE(pov2775_mode_info); i++)
	{
		if (pmode->index == pov2775_mode_info[i].index)
		{
			pcur_mode = &pov2775_mode_info[i];
			sensor->curr_modeinfo_index = i;
			sensor->fps = pov2775_mode_info[i].fps;
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

#ifdef CONFIG_HARDENED_USERCOPY
#define USER_TO_KERNEL(TYPE) \
	do {\
		TYPE tmp; \
		arg = (void *)(&tmp); \
		copy_from_user(arg, arg_user, sizeof(TYPE));\
	} while (0)

#define KERNEL_TO_USER(TYPE) \
		copy_to_user(arg_user, arg, sizeof(TYPE));
#else
#define USER_TO_KERNEL(TYPE)
#define KERNEL_TO_USER(TYPE)
#endif

long ov2775_priv_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg_user)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov2775 *sensor = client_to_ov2775(client);
	struct vvcam_sccb_data reg;
	int ret = 0;
	void *arg = arg_user;

	/* pr_info("enter %s\n", __func__); */
	switch (cmd) {
	case VVSENSORIOC_WRITE_REG: {
		USER_TO_KERNEL(struct vvcam_sccb_data);
		reg = *(struct vvcam_sccb_data *)arg;
		ret = ov2775_write_reg(sensor, reg.addr, (u8)reg.data) < 0;
		break;
	}
	case VVSENSORIOC_READ_REG: {
		struct vvcam_sccb_data *preg;
		u8 val;
		USER_TO_KERNEL(struct vvcam_sccb_data);
		preg = (struct vvcam_sccb_data *)arg;
		ret = ov2775_read_reg(sensor, (u16) preg->addr, &val) < 0;
		preg->data = val;
		KERNEL_TO_USER(struct vvcam_sccb_data);
		break;
	}
	case VVSENSORIOC_S_STREAM: {
		USER_TO_KERNEL(__u32);
		ret = ov2775_s_stream(sd, *(__u32 *)arg);
		break;
	}
	case VVSENSORIOC_S_EXP: {
		USER_TO_KERNEL(__u32);
		ret = ov2775_s_exp(sensor, *(__u32 *)arg);
		break;
	}
	case VVSENSORIOC_S_VSEXP: {
		USER_TO_KERNEL(__u32);
		ret = ov2775_s_vsexp(sensor, *(__u32 *)arg);
		break;
	}
	case VVSENSORIOC_S_GAIN: {
		USER_TO_KERNEL(__u32);
		ret = ov2775_s_gain(sensor, *(__u32 *)arg);
		break;
	}
	case VVSENSORIOC_S_VSGAIN: {
		USER_TO_KERNEL(__u32);
		ret = ov2775_s_vsgain(sensor, *(__u32 *)arg);
		break;
	}
	case VVSENSORIOC_S_FPS: {
		USER_TO_KERNEL(__u32);
		ret = ov2775_s_fps(sensor, *(__u32 *)arg);
		break;
	}
	case VVSENSORIOC_G_FPS: {
		USER_TO_KERNEL(__u32);
		ret = ov2775_g_fps(sensor, (__u32 *)arg);
		KERNEL_TO_USER(__u32);
		break;
	}
	case VVSENSORIOC_S_CLK: {
		USER_TO_KERNEL(__u32);
		ret = ov2775_s_clk(sensor, *(__u32 *)arg);
		break;
	}
	case VVSENSORIOC_G_CLK: {
		USER_TO_KERNEL(__u32);
		ret = ov2775_g_clk(sensor, (__u32 *)arg);
		KERNEL_TO_USER(__u32);
		break;
	}
	case VIDIOC_QUERYCAP:
		ret = ov2775_ioc_qcap(NULL, arg);
		break;
		
	case VVSENSORIOC_G_CHIP_ID: {
		USER_TO_KERNEL(__u32);
		ret = ov2775_g_chipid(sensor, (__u32 *)arg);
		ret = (ret < 0) ? -1 : 0;
		KERNEL_TO_USER(__u32);
		break;
	}

	case VVSENSORIOC_G_RESERVE_ID: {
		__u32  correct_id = 0x2770;
		ret = copy_to_user(arg_user, &correct_id, sizeof(__u32));
		ret = ret ? -1 : 0;
		break;
	}

	case VVSENSORIOC_S_HDR_MODE: {
		USER_TO_KERNEL(bool);
		ret = ov2775_s_hdr(sensor, *(bool *)arg);
		break;
	}
	case VVSENSORIOC_QUERY: {
		USER_TO_KERNEL(struct vvcam_mode_info_arry);
		ret = ov2775_ioc_query_mode(sensor, arg);
		KERNEL_TO_USER(struct vvcam_mode_info_arry);
		break;
	}
	case VVSENSORIOC_G_SENSOR_MODE:{
		USER_TO_KERNEL(struct vvcam_mode_info);
		ret = ov2775_g_mode(sensor, arg);
		KERNEL_TO_USER(struct vvcam_mode_info);
		break;
	}
	default:
		pr_err("unsupported ov2775 command %d.", cmd);
		ret = -1;
		break;
	}

	return ret;
}