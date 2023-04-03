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
#include <linux/of_graph.h>
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
#include <media/v4l2-fwnode.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include "vvsensor.h"
#include "ar1335_regs_1080p.h"
#include "ar1335_regs_1080p60.h"
#include "ar1335_regs_12MP.h"


#define AR1335_VOLTAGE_ANALOG			2800000
#define AR1335_VOLTAGE_DIGITAL_CORE		1500000
#define AR1335_VOLTAGE_DIGITAL_IO		1800000

#define AR1335_XCLK_MIN 6000000
#define AR1335_XCLK_MAX 48000000

#define AR1335_CHIP_ID                  0x153
#define AR1335_CHIP_VERSION_REG 		0x3000
#define AR1335_RESET_REG                0x301A

#define AR1335_SENS_PAD_SOURCE	0
#define AR1335_SENS_PADS_NUM	1

#define AR1335_RESERVE_ID 0X2770
#define DCG_CONVERSION_GAIN 11

#define client_to_ar1335(client)\
	container_of(i2c_get_clientdata(client), struct ar1335, subdev)

struct ar1335_capture_properties {
	__u64 max_lane_frequency;
	__u64 max_pixel_frequency;
	__u64 max_data_rate;
};

struct ar1335 {
	struct i2c_client *i2c_client;
	struct regulator *io_regulator;
	struct regulator *core_regulator;
	struct regulator *analog_regulator;
	unsigned int pwn_gpio;
	unsigned int rst_gpio;
	unsigned int mclk;
	unsigned int mclk_source;
	struct clk *sensor_clk;
	unsigned int csi_id;
	struct ar1335_capture_properties ocp;

	struct v4l2_subdev subdev;
	struct media_pad pads[AR1335_SENS_PADS_NUM];

	struct v4l2_mbus_framefmt format;
	vvcam_mode_info_t cur_mode;
	sensor_blc_t blc;
	sensor_white_balance_t wb;
	struct mutex lock;
	u32 stream_status;
	u32 resume_status;
	vvcam_lens_t focus_lens;
};

static struct vvcam_mode_info_s par1335_mode_info[] = {
	{
		.index          = 0,
		.size           = {
			.bounds_width  = 1920,
			.bounds_height = 1080,
			.top           = 0,
			.left          = 0,
			.width         = 1920,
			.height        = 1080,
		},
		.hdr_mode       = SENSOR_MODE_LINEAR,
		.bit_width      = 10,
		.data_compress  = {
			.enable = 0,
		},
		.bayer_pattern = BAYER_GRBG,
		.ae_info = {
			.def_frm_len_lines     = 0xC4E,
			.curr_frm_len_lines    = 0xC4E - 1,
			.one_line_exp_time_ns  = 10601,

			.max_integration_line  = 0xC4E - 1,
			.min_integration_line  = 8,

			.max_again             = 7.75 * (1 << SENSOR_FIX_FRACBITS),
			.min_again             = 1 * (1 << SENSOR_FIX_FRACBITS),
			.max_dgain             = 3.09 * (1 << SENSOR_FIX_FRACBITS),
			.min_dgain             = 1 * (1 << SENSOR_FIX_FRACBITS),
			.gain_step             = 2,

			.start_exposure        = 3 * 100 * (1 << SENSOR_FIX_FRACBITS),
			.cur_fps               = 30 * (1 << SENSOR_FIX_FRACBITS),
			.max_fps               = 30 * (1 << SENSOR_FIX_FRACBITS),
			.min_fps               = 5 * (1 << SENSOR_FIX_FRACBITS),
			.min_afps              = 5 * (1 << SENSOR_FIX_FRACBITS),
			.int_update_delay_frm  = 1,
			.gain_update_delay_frm = 1,
		},
		.mipi_info = {
			.mipi_lane = 4,
		},
		.preg_data      = ar1335_init_setting_1080p,
		.reg_data_count = ARRAY_SIZE(ar1335_init_setting_1080p),
	},
	{
		.index          = 1,
		.size           = {
			.bounds_width  = 1920,
			.bounds_height = 1080,
			.top           = 0,
			.left          = 0,
			.width         = 1920,
			.height        = 1080,
		},
		.hdr_mode       = SENSOR_MODE_LINEAR,
		.bit_width      = 10,
		.data_compress  = {
			.enable = 0,
		},
		.bayer_pattern = BAYER_GRBG,
		.ae_info = {
			.def_frm_len_lines     = 0x0625,
			.curr_frm_len_lines    = 0x0625 - 1,
			.one_line_exp_time_ns  = 10601,

			.max_integration_line  = 0x0625 - 1,
			.min_integration_line  = 8,

			.max_again             = 7.75 * (1 << SENSOR_FIX_FRACBITS),
			.min_again             = 1 * (1 << SENSOR_FIX_FRACBITS),
			.max_dgain             = 3.09 * (1 << SENSOR_FIX_FRACBITS),
			.min_dgain             = 1 * (1 << SENSOR_FIX_FRACBITS),
			.gain_step             = 2,

			.start_exposure        = 3 * 100 * (1 << SENSOR_FIX_FRACBITS),
			.cur_fps               = 60 * (1 << SENSOR_FIX_FRACBITS),
			.max_fps               = 60 * (1 << SENSOR_FIX_FRACBITS),
			.min_fps               = 5 * (1 << SENSOR_FIX_FRACBITS),
			.min_afps              = 5 * (1 << SENSOR_FIX_FRACBITS),
			.int_update_delay_frm  = 1,
			.gain_update_delay_frm = 1,
		},
		.mipi_info = {
			.mipi_lane = 4,
		},
		.preg_data      = ar1335_init_setting_1080p60,
		.reg_data_count = ARRAY_SIZE(ar1335_init_setting_1080p60),
	},
    {
		.index          = 2,
		.size           = {
			.bounds_width  = 4096,
			.bounds_height = 3072,
			.top           = 0,
			.left          = 0,
			.width         = 4096,
			.height        = 3072,
		},
		.hdr_mode       = SENSOR_MODE_LINEAR,
		.bit_width      = 10,
		.data_compress  = {
			.enable = 0,
		},
		.bayer_pattern = BAYER_GRBG,
		.ae_info = {
			.def_frm_len_lines     = 0xC90,
			.curr_frm_len_lines    = 0xC90 - 1,
			.one_line_exp_time_ns  = 12435,

			.max_integration_line  = 0xC90 - 1,
			.min_integration_line  = 8,

			.max_again             = 7.75 * (1 << SENSOR_FIX_FRACBITS),
			.min_again             = 1 * (1 << SENSOR_FIX_FRACBITS),
			.max_dgain             = 3.09 * (1 << SENSOR_FIX_FRACBITS),
			.min_dgain             = 1 * (1 << SENSOR_FIX_FRACBITS),
			.gain_step             = 2,

			.start_exposure        = 3 * 100 * (1 << SENSOR_FIX_FRACBITS),
			.cur_fps               = 25 * (1 << SENSOR_FIX_FRACBITS),
			.max_fps               = 25 * (1 << SENSOR_FIX_FRACBITS),
			.min_fps               = 5 * (1 << SENSOR_FIX_FRACBITS),
			.min_afps              = 5 * (1 << SENSOR_FIX_FRACBITS),
			.int_update_delay_frm  = 1,
			.gain_update_delay_frm = 1,
		},
		.mipi_info = {
			.mipi_lane = 4,
		},
		.preg_data      = ar1335_init_setting_12MP,
		.reg_data_count = ARRAY_SIZE(ar1335_init_setting_12MP),
	},

};

int ar1335_get_clk(struct ar1335 *sensor, void *clk)
{
	struct vvcam_clk_s vvcam_clk;
	int ret = 0;
	vvcam_clk.sensor_mclk = clk_get_rate(sensor->sensor_clk);
	vvcam_clk.csi_max_pixel_clk = sensor->ocp.max_pixel_frequency;
	ret = copy_to_user(clk, &vvcam_clk, sizeof(struct vvcam_clk_s));
	if (ret != 0)
		ret = -EINVAL;
	return ret;
}

static int ar1335_power_on(struct ar1335 *sensor)
{
	int ret;
	pr_debug("enter %s\n", __func__);

	if (gpio_is_valid(sensor->pwn_gpio))
		gpio_set_value_cansleep(sensor->pwn_gpio, 1);

	ret = clk_prepare_enable(sensor->sensor_clk);
	if (ret < 0)
		pr_err("%s: enable sensor clk fail\n", __func__);

	return ret;
}

static int ar1335_power_off(struct ar1335 *sensor)
{
	pr_debug("enter %s\n", __func__);
	if (gpio_is_valid(sensor->pwn_gpio))
		gpio_set_value_cansleep(sensor->pwn_gpio, 0);
	clk_disable_unprepare(sensor->sensor_clk);

	return 0;
}

static int ar1335_s_power(struct v4l2_subdev *sd, int on)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ar1335 *sensor = client_to_ar1335(client);

	pr_debug("enter %s\n", __func__);
	if (on)
		ar1335_power_on(sensor);
	else
		ar1335_power_off(sensor);

	return 0;
}

static s32 ar1335_write_reg(struct ar1335 *sensor, u16 reg, u16 val)
{
	struct device *dev = &sensor->i2c_client->dev;
	u8 au8Buf[4] = { reg >> 8, reg & 0xff, val >> 8, val & 0xff };

	if (i2c_master_send(sensor->i2c_client, au8Buf, 4) < 0) {
		dev_err(dev, "Write reg error: reg=%x, val=%x\n", reg, val);
		return -1;
	}

	return 0;
}


static s32 ar1335_read_reg(struct ar1335 *sensor, u16 reg, u16 *val)
{
	struct device *dev = &sensor->i2c_client->dev;
	u8 au8RegBuf[2] = { reg >> 8, reg & 0xff };
	u8 au8RdVal[2] = {0};

	if (i2c_master_send(sensor->i2c_client, au8RegBuf, 2) != 2) {
		dev_err(dev, "Read reg error: reg=%x\n", reg);
		return -1;
	}

	if (i2c_master_recv(sensor->i2c_client, au8RdVal, 2) != 2) {
		dev_err(dev, "Read reg error: reg=%x, val=%x %x\n",
                        reg, au8RdVal[0], au8RdVal[1]);
		return -1;
	}

	*val = ((u16)au8RdVal[0] << 8) | (u16)au8RdVal[1];

	return 0;
}


static int ar1335_stream_on(struct ar1335 *sensor)
{
	int ret;
	u16 val = 0;

	ret = ar1335_read_reg(sensor, AR1335_RESET_REG, &val);
	if (ret < 0)
		return ret;
	val |= 0x0004;
	ret = ar1335_write_reg(sensor, AR1335_RESET_REG, val);

	return ret;
}

static int ar1335_stream_off(struct ar1335 *sensor)
{
	int ret;
	u16 val = 0;

	ret = ar1335_read_reg(sensor, AR1335_RESET_REG, &val);
	if (ret < 0)
		return ret;
	val &= (~0x0004);
	ret = ar1335_write_reg(sensor, AR1335_RESET_REG, val);

	return ret;
}

static int ar1335_write_reg_arry(struct ar1335 *sensor,
				    struct vvcam_sccb_data_s *mode_setting,
				    s32 size)
{
	register u16 reg_addr = 0;
	register u16 data = 0;
	int i, retval = 0;

	for (i = 0; i < size; ++i, ++mode_setting) {

		reg_addr = mode_setting->addr;
		data = mode_setting->data;

		retval = ar1335_write_reg(sensor, reg_addr, data);
		if (retval < 0)
			break;
		if (reg_addr == 0x301A  && i == 0)
			msleep(100);
	}

    ar1335_stream_off(sensor);

	return retval;
}

static int ar1335_query_capability(struct ar1335 *sensor, void *arg)
{
	struct v4l2_capability *pcap = (struct v4l2_capability *)arg;

	strcpy((char *)pcap->driver, "ar1335");
	sprintf((char *)pcap->bus_info, "csi%d",sensor->csi_id);
	if(sensor->i2c_client->adapter) {
		pcap->bus_info[VVCAM_CAP_BUS_INFO_I2C_ADAPTER_NR_POS] =
			(__u8)sensor->i2c_client->adapter->nr;
	} else {
		pcap->bus_info[VVCAM_CAP_BUS_INFO_I2C_ADAPTER_NR_POS] = 0xFF;
	}
	return 0;
}

static int ar1335_query_supports(struct ar1335 *sensor, void* parry)
{
	int ret = 0;
	struct vvcam_mode_info_array_s *psensor_mode_arry = parry;
	uint32_t support_counts = ARRAY_SIZE(par1335_mode_info);

	ret = copy_to_user(&psensor_mode_arry->count, &support_counts, sizeof(support_counts));
	ret |= copy_to_user(&psensor_mode_arry->modes, par1335_mode_info,
			   sizeof(par1335_mode_info));
	if (ret != 0)
		ret = -ENOMEM;
	return ret;
}

static int ar1335_get_sensor_id(struct ar1335 *sensor, void* pchip_id)
{
	int ret = 0;
	u16 chip_id;

	ret = ar1335_read_reg(sensor, 0x3000, &chip_id);
	ret = copy_to_user(pchip_id, &chip_id, sizeof(u16));
	if (ret != 0)
		ret = -ENOMEM;
	return ret;
}

static int ar1335_get_reserve_id(struct ar1335 *sensor, void* preserve_id)
{
	int ret = 0;
	u16 reserve_id = 0x2770;
	ret = copy_to_user(preserve_id, &reserve_id, sizeof(u16));
	if (ret != 0)
		ret = -ENOMEM;
	return ret;
}

static int ar1335_get_sensor_mode(struct ar1335 *sensor, void* pmode)
{
	int ret = 0;
	ret = copy_to_user(pmode, &sensor->cur_mode,
		sizeof(struct vvcam_mode_info_s));
	if (ret != 0)
		ret = -ENOMEM;
	return ret;
}

static int ar1335_set_sensor_mode(struct ar1335 *sensor, void* pmode)
{
	int ret = 0;
	int i = 0;
	struct vvcam_mode_info_s sensor_mode;

	ret = copy_from_user(&sensor_mode, pmode,
		sizeof(struct vvcam_mode_info_s));
	if (ret != 0)
		return -ENOMEM;
	for (i = 0; i < ARRAY_SIZE(par1335_mode_info); i++) 
    {
		if (par1335_mode_info[i].index == sensor_mode.index) 
        {
			memcpy(&sensor->cur_mode, &par1335_mode_info[i],sizeof(struct vvcam_mode_info_s));
			return 0;
		}
	}
	return -ENXIO;
}

static int ar1335_set_exp(struct ar1335 *sensor, u32 exp)
{
	int ret = 0;
	ret |= ar1335_write_reg(sensor, 0x0202, exp);

	return ret;
}

static int ar1335_set_gain(struct ar1335 *sensor, u32 gain)
{
	int ret = 0;
	u16 new_gain = 0;
	u32 div = 0;

	if (gain < (1 << SENSOR_FIX_FRACBITS)) {
		new_gain = 0x2010;
	} else if (gain < (8 * (1 << SENSOR_FIX_FRACBITS))) {
		div = gain / 0x400;
		if (div < 2) {
			new_gain |= 0x2010;
			new_gain += 2 * ((gain - 0x400) / 125);
		} else if (div < 4) {
			new_gain |= 0x2020;
			new_gain += 1 * ((gain - 0x800) / 125);
		} else {
			new_gain |= 0x2030;
			new_gain += (gain - 0x1000) / 250;
		}
	} else if (gain < (24 * (1 << SENSOR_FIX_FRACBITS))) {
		new_gain |= 0x21BF;
		new_gain += (0x100 * (gain - 0x2000) / 250) & 0xFF00;
	} else {
		new_gain = 0x633F;
	}

	ret = ar1335_write_reg(sensor, 0x305E, new_gain);
    return ret;
}

static int ar1335_set_fps(struct ar1335 *sensor, u32 fps)
{
	u32 vts;
	int ret = 0;

	if (fps > sensor->cur_mode.ae_info.max_fps) {
		fps = sensor->cur_mode.ae_info.max_fps;
	}
	else if (fps < sensor->cur_mode.ae_info.min_fps) {
		fps = sensor->cur_mode.ae_info.min_fps;
	}
	vts = sensor->cur_mode.ae_info.max_fps *
	      sensor->cur_mode.ae_info.def_frm_len_lines / fps;

	ret |= ar1335_write_reg(sensor, 0x3012, vts);
	sensor->cur_mode.ae_info.cur_fps = fps;

	if (sensor->cur_mode.hdr_mode == SENSOR_MODE_LINEAR) {
		sensor->cur_mode.ae_info.max_integration_line = vts - 1;
	} else {
		if (sensor->cur_mode.stitching_mode ==
		    SENSOR_STITCHING_DUAL_DCG){
			sensor->cur_mode.ae_info.max_vsintegration_line = 44;
			sensor->cur_mode.ae_info.max_integration_line = vts -
				4 - sensor->cur_mode.ae_info.max_vsintegration_line;
		} else {
			sensor->cur_mode.ae_info.max_integration_line = vts - 1;
		}
	}
	sensor->cur_mode.ae_info.curr_frm_len_lines = vts;
	return ret;
}

static int ar1335_get_fps(struct ar1335 *sensor, u32 *pfps)
{
	*pfps = sensor->cur_mode.ae_info.cur_fps;
	return 0;

}

static int ar1335_set_test_pattern(struct ar1335 *sensor, void * arg)
{

	int ret;
	struct sensor_test_pattern_s test_pattern;

	ret = copy_from_user(&test_pattern, arg, sizeof(test_pattern));
	if (ret != 0)
		return -ENOMEM;
	if (test_pattern.enable) {
		switch (test_pattern.pattern) {
		case 0:
			ret |= ar1335_write_reg(sensor, 0x0600, 0x0001);
			break;
		case 1:
			ret |= ar1335_write_reg(sensor, 0x0600, 0x0002);
			break;
		case 2:
			ret |= ar1335_write_reg(sensor, 0x0600, 0x0003);
			break;
		default:
			ret = -1;
			break;
		}
	}
	return ret;
}

static int ar1335_get_lens(struct ar1335 *sensor, void * arg) {

	vvcam_lens_t *pfocus_lens = (vvcam_lens_t *)arg;

	if (!arg)
		return -ENOMEM;

	if (strlen(sensor->focus_lens.name) == 0)
		return -1;

	return copy_to_user(pfocus_lens, &sensor->focus_lens, sizeof(vvcam_lens_t));
}

static int ar1335_get_format_code(struct ar1335 *sensor, u32 *code)
{
	switch (sensor->cur_mode.bayer_pattern) {
	case BAYER_RGGB:
		if (sensor->cur_mode.bit_width == 8) {
			*code = MEDIA_BUS_FMT_SRGGB8_1X8;
		} else if (sensor->cur_mode.bit_width == 10) {
			*code = MEDIA_BUS_FMT_SRGGB10_1X10;
		} else {
			*code = MEDIA_BUS_FMT_SRGGB12_1X12;
		}
		break;
	case BAYER_GRBG:
		if (sensor->cur_mode.bit_width == 8) {
			*code = MEDIA_BUS_FMT_SGRBG8_1X8;
		} else if (sensor->cur_mode.bit_width == 10) {
			*code = MEDIA_BUS_FMT_SGRBG10_1X10;
		} else {
			*code = MEDIA_BUS_FMT_SGRBG12_1X12;
		}
		break;
	case BAYER_GBRG:
		if (sensor->cur_mode.bit_width == 8) {
			*code = MEDIA_BUS_FMT_SGBRG8_1X8;
		} else if (sensor->cur_mode.bit_width == 10) {
			*code = MEDIA_BUS_FMT_SGBRG10_1X10;
		} else {
			*code = MEDIA_BUS_FMT_SGBRG12_1X12;
		}
		break;
	case BAYER_BGGR:
		if (sensor->cur_mode.bit_width == 8) {
			*code = MEDIA_BUS_FMT_SBGGR8_1X8;
		} else if (sensor->cur_mode.bit_width == 10) {
			*code = MEDIA_BUS_FMT_SBGGR10_1X10;
		} else {
			*code = MEDIA_BUS_FMT_SBGGR12_1X12;
		}
		break;
	default:
		/*nothing need to do*/
		break;
	}
	return 0;
}


static int ar1335_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ar1335 *sensor = client_to_ar1335(client);
    int ret;
	
	pr_debug("enter %s\n", __func__);
	if (enable) {
        ret = ar1335_stream_on(sensor);
        if (ret < 0)
            return ret;
    } else {
        ret = ar1335_stream_off(sensor);
        if (ret < 0)
            return ret;
    }

	return 0;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 12, 0)
static int ar1335_enum_mbus_code(struct v4l2_subdev *sd,
				 struct v4l2_subdev_state *state,
				 struct v4l2_subdev_mbus_code_enum *code)
#else
static int ar1335_enum_mbus_code(struct v4l2_subdev *sd,
			         struct v4l2_subdev_pad_config *cfg,
			         struct v4l2_subdev_mbus_code_enum *code)
#endif
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ar1335 *sensor = client_to_ar1335(client);

	u32 cur_code = MEDIA_BUS_FMT_SBGGR12_1X12;

	if (code->index > 0)
		return -EINVAL;
	ar1335_get_format_code(sensor,&cur_code);
	code->code = cur_code;

	return 0;
}


#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 12, 0)
static int ar1335_set_fmt(struct v4l2_subdev *sd,
			  struct v4l2_subdev_state *state,
			  struct v4l2_subdev_format *fmt)
#else
static int ar1335_set_fmt(struct v4l2_subdev *sd,
			  struct v4l2_subdev_pad_config *cfg,
			  struct v4l2_subdev_format *fmt)

#endif
{
	int ret = 0;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ar1335 *sensor = client_to_ar1335(client);

	mutex_lock(&sensor->lock);
	pr_info("enter %s\n", __func__);
	if ((fmt->format.width != sensor->cur_mode.size.bounds_width) ||
	    (fmt->format.height != sensor->cur_mode.size.bounds_height)) {
		pr_err("%s:set sensor format %dx%d error\n",
			__func__,fmt->format.width,fmt->format.height);
		mutex_unlock(&sensor->lock);
		return -EINVAL;
	}
	
	ret |= ar1335_write_reg_arry(sensor,
		sensor->cur_mode.preg_data,
		sensor->cur_mode.reg_data_count);
	
	if (ret < 0) {
		pr_err("%s:ar1335_write_reg_arry error\n",__func__);
		mutex_unlock(&sensor->lock);
		return -EINVAL;
	}
	ar1335_get_format_code(sensor, &fmt->format.code);
	fmt->format.field = V4L2_FIELD_NONE;
	sensor->format = fmt->format;
	mutex_unlock(&sensor->lock);
	return 0;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 12, 0)
static int ar1335_get_fmt(struct v4l2_subdev *sd,
			  struct v4l2_subdev_state *state,
			  struct v4l2_subdev_format *fmt)
#else
static int ar1335_get_fmt(struct v4l2_subdev *sd,
			  struct v4l2_subdev_pad_config *cfg,
			  struct v4l2_subdev_format *fmt)
#endif
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ar1335 *sensor = client_to_ar1335(client);

	mutex_lock(&sensor->lock);
	fmt->format = sensor->format;
	mutex_unlock(&sensor->lock);
	return 0;
}

static long ar1335_priv_ioctl(struct v4l2_subdev *sd,
                              unsigned int cmd,
                              void *arg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ar1335 *sensor = client_to_ar1335(client);
	long ret = 0;
	struct vvcam_sccb_data_s sensor_reg;
	uint32_t value = 0;

	mutex_lock(&sensor->lock);
	switch (cmd){
	case VVSENSORIOC_S_POWER:
		ret = 0;
		break;
	case VVSENSORIOC_S_CLK:
		ret = 0;
		break;
	case VVSENSORIOC_G_CLK:
		ret = ar1335_get_clk(sensor,arg);
		break;
	case VVSENSORIOC_RESET:
		ret = 0;
		break;
	case VIDIOC_QUERYCAP:
		ret = ar1335_query_capability(sensor, arg);
		break;
	case VVSENSORIOC_QUERY:
		ret = ar1335_query_supports(sensor, arg);
		break;
	case VVSENSORIOC_G_CHIP_ID:
		ret = ar1335_get_sensor_id(sensor, arg);
		break;
	case VVSENSORIOC_G_RESERVE_ID:
		ret = ar1335_get_reserve_id(sensor, arg);
		break;
	case VVSENSORIOC_G_SENSOR_MODE:
		ret = ar1335_get_sensor_mode(sensor, arg);
		break;
	case VVSENSORIOC_S_SENSOR_MODE:
		ret = ar1335_set_sensor_mode(sensor, arg);
		break;
	case VVSENSORIOC_S_STREAM:
		ret = copy_from_user(&value, arg, sizeof(value));
		ret |= ar1335_s_stream(&sensor->subdev, value);
		break;
	case VVSENSORIOC_WRITE_REG:
		ret = copy_from_user(&sensor_reg, arg,
			sizeof(struct vvcam_sccb_data_s));
		ret |= ar1335_write_reg(sensor, sensor_reg.addr,
			sensor_reg.data);
		break;
	case VVSENSORIOC_READ_REG:
		ret = copy_from_user(&sensor_reg, arg, sizeof(struct vvcam_sccb_data_s));
		ret |= ar1335_read_reg(sensor, (u16)sensor_reg.addr, (u16 *)&sensor_reg.data);
		ret |= copy_to_user(arg, &sensor_reg, sizeof(struct vvcam_sccb_data_s));
		break;
	case VVSENSORIOC_S_EXP:
		ret = copy_from_user(&value, arg, sizeof(value));
		ret |= ar1335_set_exp(sensor, value);
		break;
	case VVSENSORIOC_S_GAIN:
		ret = copy_from_user(&value, arg, sizeof(value));
		ret |= ar1335_set_gain(sensor, value);
		break;
	case VVSENSORIOC_S_FPS:
		ret = copy_from_user(&value, arg, sizeof(value));
		ret |= ar1335_set_fps(sensor, value);
		break;
	case VVSENSORIOC_G_FPS:
		ret = ar1335_get_fps(sensor, &value);
		ret |= copy_to_user(arg, &value, sizeof(value));
		break;
	case VVSENSORIOC_S_TEST_PATTERN:
		ret= ar1335_set_test_pattern(sensor, arg);
		break;
	case VVSENSORIOC_G_LENS:
		ret = ar1335_get_lens(sensor, arg);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	mutex_unlock(&sensor->lock);
	return ret;
}

static struct v4l2_subdev_video_ops ar1335_subdev_video_ops = {
	.s_stream = ar1335_s_stream,
};

static const struct v4l2_subdev_pad_ops ar1335_subdev_pad_ops = {
	.enum_mbus_code = ar1335_enum_mbus_code,
	.set_fmt = ar1335_set_fmt,
	.get_fmt = ar1335_get_fmt,
};

static struct v4l2_subdev_core_ops ar1335_subdev_core_ops = {
	.s_power = ar1335_s_power,
	.ioctl = ar1335_priv_ioctl,
};

static struct v4l2_subdev_ops ar1335_subdev_ops = {
	.core  = &ar1335_subdev_core_ops,
	.video = &ar1335_subdev_video_ops,
	.pad   = &ar1335_subdev_pad_ops,
};

static int ar1335_link_setup(struct media_entity *entity,
			     const struct media_pad *local,
			     const struct media_pad *remote, u32 flags)
{
	return 0;
}

static const struct media_entity_operations ar1335_sd_media_ops = {
	.link_setup = ar1335_link_setup,
};

static int ar1335_regulator_enable(struct ar1335 *sensor)
{
	int ret = 0;
	struct device *dev = &(sensor->i2c_client->dev);

	pr_debug("enter %s\n", __func__);

	if (sensor->io_regulator) {
		regulator_set_voltage(sensor->io_regulator,
				      AR1335_VOLTAGE_DIGITAL_IO,
				      AR1335_VOLTAGE_DIGITAL_IO);
		ret = regulator_enable(sensor->io_regulator);
		if (ret < 0) {
			dev_err(dev, "set io voltage failed\n");
			return ret;
		}
	}

	if (sensor->analog_regulator) {
		regulator_set_voltage(sensor->analog_regulator,
				      AR1335_VOLTAGE_ANALOG,
				      AR1335_VOLTAGE_ANALOG);
		ret = regulator_enable(sensor->analog_regulator);
		if (ret) {
			dev_err(dev, "set analog voltage failed\n");
			goto err_disable_io;
		}

	}

	if (sensor->core_regulator) {
		regulator_set_voltage(sensor->core_regulator,
				      AR1335_VOLTAGE_DIGITAL_CORE,
				      AR1335_VOLTAGE_DIGITAL_CORE);
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

static void ar1335_regulator_disable(struct ar1335 *sensor)
{
	int ret = 0;
	struct device *dev = &(sensor->i2c_client->dev);

	if (sensor->core_regulator) {
		ret = regulator_disable(sensor->core_regulator);
		if (ret < 0)
			dev_err(dev, "core regulator disable failed\n");
	}

	if (sensor->analog_regulator) {
		ret = regulator_disable(sensor->analog_regulator);
		if (ret < 0)
			dev_err(dev, "analog regulator disable failed\n");
	}

	if (sensor->io_regulator) {
		ret = regulator_disable(sensor->io_regulator);
		if (ret < 0)
			dev_err(dev, "io regulator disable failed\n");
	}
	return ;
}

static int ar1335_set_clk_rate(struct ar1335 *sensor)
{
	int ret;
	unsigned int clk;

	clk = sensor->mclk;
	clk = min_t(u32, clk, (u32)AR1335_XCLK_MAX);
	clk = max_t(u32, clk, (u32)AR1335_XCLK_MIN);
	sensor->mclk = clk;

	pr_debug("   Setting mclk to %d MHz\n",sensor->mclk / 1000000);
	ret = clk_set_rate(sensor->sensor_clk, sensor->mclk);
	if (ret < 0)
		pr_debug("set rate filed, rate=%d\n", sensor->mclk);
	return ret;
}

static void ar1335_reset(struct ar1335 *sensor)
{
	pr_debug("enter %s\n", __func__);
	if (!gpio_is_valid(sensor->rst_gpio))
		return;

	gpio_set_value_cansleep(sensor->rst_gpio, 0);
	msleep(20);

	gpio_set_value_cansleep(sensor->rst_gpio, 1);
	msleep(20);

	return;
}
static int ar1335_retrieve_capture_properties(
			struct ar1335 *sensor,
			struct ar1335_capture_properties* ocp)
{
	struct device *dev = &sensor->i2c_client->dev;
	__u64 mlf = 0;
	__u64 mpf = 0;
	__u64 mdr = 0;

	struct device_node *ep;
	int ret;
	/*Collecting the information about limits of capture path
	* has been centralized to the sensor
	* * also into the sensor endpoint itself.
	*/

	ep = of_graph_get_next_endpoint(dev->of_node, NULL);
	if (!ep) {
		dev_err(dev, "missing endpoint node\n");
		return -ENODEV;
	}

	/*ret = fwnode_property_read_u64(of_fwnode_handle(ep),
		"max-lane-frequency", &mlf);
	if (ret || mlf == 0) {
		dev_dbg(dev, "no limit for max-lane-frequency\n");
	}*/
	ret = fwnode_property_read_u64(of_fwnode_handle(ep),
	        "max-pixel-frequency", &mpf);
	if (ret || mpf == 0) {
	        dev_dbg(dev, "no limit for max-pixel-frequency\n");
	}

	/*ret = fwnode_property_read_u64(of_fwnode_handle(ep),
	        "max-data-rate", &mdr);
	if (ret || mdr == 0) {
	        dev_dbg(dev, "no limit for max-data_rate\n");
	}*/

	ocp->max_lane_frequency = mlf;
	ocp->max_pixel_frequency = mpf;
	ocp->max_data_rate = mdr;

	return ret;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 3, 0)
static int ar1335_probe(struct i2c_client *client)
#else
static int ar1335_probe(struct i2c_client *client,
                        const struct i2c_device_id *id)
#endif
{
	int retval;
	struct device *dev = &client->dev;
	struct v4l2_subdev *sd;
	struct ar1335 *sensor;
    u16 chip_id;
	struct device_node *lens_node;

	pr_info("enter %s\n", __func__);

	sensor = devm_kmalloc(dev, sizeof(*sensor), GFP_KERNEL);
	if (!sensor)
		return -ENOMEM;
	memset(sensor, 0, sizeof(*sensor));

	sensor->i2c_client = client;

	sensor->pwn_gpio = of_get_named_gpio(dev->of_node, "pwn-gpios", 0);
	if (!gpio_is_valid(sensor->pwn_gpio))
		dev_warn(dev, "No sensor pwdn pin available");
	else {
		retval = devm_gpio_request_one(dev, sensor->pwn_gpio,
						GPIOF_OUT_INIT_HIGH,
						"ar1335_mipi_pwdn");
		if (retval < 0) {
			dev_warn(dev, "Failed to set power pin\n");
			dev_warn(dev, "retval=%d\n", retval);
			return retval;
		}
	}

	sensor->rst_gpio = of_get_named_gpio(dev->of_node, "rst-gpios", 0);
	if (!gpio_is_valid(sensor->rst_gpio))
		dev_warn(dev, "No sensor reset pin available");
	else {
		retval = devm_gpio_request_one(dev, sensor->rst_gpio,
						GPIOF_OUT_INIT_HIGH,
						"ar1335_mipi_reset");
		if (retval < 0) {
			dev_warn(dev, "Failed to set reset pin\n");
			return retval;
		}
	}

	sensor->sensor_clk = devm_clk_get(dev, "csi_mclk");
	if (IS_ERR(sensor->sensor_clk)) {
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

	retval = of_property_read_u32(dev->of_node, "csi_id", &(sensor->csi_id));
	if (retval) {
		dev_err(dev, "csi id missing or invalid\n");
		return retval;
	}

	lens_node = of_parse_phandle(dev->of_node, "lens-focus", 0);
	if (lens_node) {
		retval = of_property_read_u32(lens_node, "id", &sensor->focus_lens.id);
		if (retval) {
			dev_err(dev, "lens-focus id missing or invalid\n");
			return retval;
		}
		memcpy(sensor->focus_lens.name, lens_node->name, strlen(lens_node->name));
	}

	retval = ar1335_retrieve_capture_properties(sensor,&sensor->ocp);
	if (retval) {
		dev_warn(dev, "retrive capture properties error\n");
	}

	sensor->io_regulator = devm_regulator_get(dev, "DOVDD");
	if (IS_ERR(sensor->io_regulator)) {
		dev_err(dev, "cannot get io regulator\n");
		return PTR_ERR(sensor->io_regulator);
	}

	sensor->core_regulator = devm_regulator_get(dev, "DVDD");
	if (IS_ERR(sensor->core_regulator)) {
		dev_err(dev, "cannot get core regulator\n");
		return PTR_ERR(sensor->core_regulator);
	}

	sensor->analog_regulator = devm_regulator_get(dev, "AVDD");
	if (IS_ERR(sensor->analog_regulator)) {
		dev_err(dev, "cannot get analog  regulator\n");
		return PTR_ERR(sensor->analog_regulator);
	}

	retval = ar1335_regulator_enable(sensor);
	if (retval) {
		dev_err(dev, "regulator enable failed\n");
		return retval;
	}

	ar1335_set_clk_rate(sensor);
	retval = clk_prepare_enable(sensor->sensor_clk);
	if (retval < 0) {
		dev_err(dev, "%s: enable sensor clk fail\n", __func__);
		goto probe_err_regulator_disable;
	}
	mdelay(2);

	retval = ar1335_power_on(sensor);
	if (retval < 0) {
		dev_err(dev, "%s: sensor power on fail\n", __func__);
		goto probe_err_regulator_disable;
	}

	ar1335_reset(sensor);

    ar1335_read_reg(sensor, AR1335_CHIP_VERSION_REG, &chip_id);
	if (chip_id != AR1335_CHIP_ID) {
		dev_err(dev, "Sensor AR1335 is not found\n");
        goto probe_err_power_off;
    }

	sd = &sensor->subdev;
	v4l2_i2c_subdev_init(sd, client, &ar1335_subdev_ops);
	sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	sd->dev = &client->dev;
	sd->entity.ops = &ar1335_sd_media_ops;
	sd->entity.function = MEDIA_ENT_F_CAM_SENSOR;
	sensor->pads[AR1335_SENS_PAD_SOURCE].flags = MEDIA_PAD_FL_SOURCE;
	retval = media_entity_pads_init(&sd->entity,
				AR1335_SENS_PADS_NUM,
				sensor->pads);
	if (retval < 0)
		goto probe_err_power_off;

#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 12, 0)
	retval = v4l2_async_register_subdev_sensor(sd);
#else
	retval = v4l2_async_register_subdev_sensor_common(sd);
#endif
	if (retval < 0) {
		dev_err(&client->dev,"%s--Async register failed, ret=%d\n",
			__func__,retval);
		goto probe_err_free_entiny;
	}

	memcpy(&sensor->cur_mode, &par1335_mode_info[0],
			sizeof(struct vvcam_mode_info_s));

	mutex_init(&sensor->lock);

	pr_info("%s camera mipi ar1335, is found\n", __func__);

	return 0;

probe_err_free_entiny:
	media_entity_cleanup(&sd->entity);

probe_err_power_off:
	ar1335_power_off(sensor);

probe_err_regulator_disable:
	ar1335_regulator_disable(sensor);

	return retval;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
static int ar1335_remove(struct i2c_client *client)
#else
static void ar1335_remove(struct i2c_client *client)
#endif
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct ar1335 *sensor = client_to_ar1335(client);

	pr_info("enter %s\n", __func__);

	v4l2_async_unregister_subdev(sd);
	media_entity_cleanup(&sd->entity);
	ar1335_power_off(sensor);
	ar1335_regulator_disable(sensor);
	mutex_destroy(&sensor->lock);

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
	return 0;
#else
#endif
}

static int __maybe_unused ar1335_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct ar1335 *sensor = client_to_ar1335(client);

	sensor->resume_status = sensor->stream_status;
	if (sensor->resume_status) {
		ar1335_s_stream(&sensor->subdev,0);
	}

	return 0;
}

static int __maybe_unused ar1335_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct ar1335 *sensor = client_to_ar1335(client);

	if (sensor->resume_status) {
		ar1335_s_stream(&sensor->subdev,1);
	}

	return 0;
}

static const struct dev_pm_ops ar1335_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(ar1335_suspend, ar1335_resume)
};

static const struct i2c_device_id ar1335_id[] = {
	{"ar1335", 0},
	{},
};
MODULE_DEVICE_TABLE(i2c, ar1335_id);

static const struct of_device_id ar1335_of_match[] = {
	{ .compatible = "onsemi,ar1335" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, ar1335_of_match);

static struct i2c_driver ar1335_i2c_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name  = "ar1335",
		.pm = &ar1335_pm_ops,
		.of_match_table	= ar1335_of_match,
	},
	.probe  = ar1335_probe,
	.remove = ar1335_remove,
	.id_table = ar1335_id,
};


module_i2c_driver(ar1335_i2c_driver);
MODULE_DESCRIPTION("AR1335 MIPI Camera Subdev Driver");
MODULE_LICENSE("GPL");
