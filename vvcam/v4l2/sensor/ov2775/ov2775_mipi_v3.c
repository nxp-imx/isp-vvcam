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

#include "ov2775_regs_1080p.h"
#include "ov2775_regs_1080p_hdr.h"
#include "ov2775_regs_1080p_hdr_low_freq.h"
#include "ov2775_regs_1080p_native_hdr.h"
#include "ov2775_regs_1080p_hdr_2dol.h"

#define OV2775_VOLTAGE_ANALOG			2800000
#define OV2775_VOLTAGE_DIGITAL_CORE		1500000
#define OV2775_VOLTAGE_DIGITAL_IO		1800000

#define OV2775_XCLK_MIN 6000000
#define OV2775_XCLK_MAX 24000000

#define OV2775_SENS_PAD_SOURCE	0
#define OV2775_SENS_PADS_NUM	1

#define OV2775_RESERVE_ID 0X2770
#define DCG_CONVERSION_GAIN 11

#define client_to_ov2775(client)\
	container_of(i2c_get_clientdata(client), struct ov2775, subdev)

struct ov2775_capture_properties {
	__u64 max_lane_frequency;
	__u64 max_pixel_frequency;
	__u64 max_data_rate;
};

struct ov2775 {
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
	struct ov2775_capture_properties ocp;

	struct v4l2_subdev subdev;
	struct media_pad pads[OV2775_SENS_PADS_NUM];

	struct v4l2_mbus_framefmt format;
	vvcam_mode_info_t cur_mode;
	sensor_blc_t blc;
	sensor_white_balance_t wb;
	struct mutex lock;
	u32 stream_status;
	u32 resume_status;
	u32 hcg_again;
	u32 hcg_dgain;
};

static struct vvcam_mode_info_s pov2775_mode_info[] = {
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
		.bit_width      = 12,
		.data_compress  = {
			.enable = 0,
		},
		.bayer_pattern = BAYER_BGGR,
		.ae_info = {
			.def_frm_len_lines     = 0x466,
			.curr_frm_len_lines    = 0x466,
			.one_line_exp_time_ns  = 29625,

			.max_integration_line  = 0x466 - 4,
			.min_integration_line  = 1,

			.max_again             = 8 * 1024,
			.min_again             = 2 * 1024,
			.max_dgain             = 4 * 1024,
			.min_dgain             = 1.5 * 1024,
			.gain_step             = 4,
			.start_exposure        = 3 * 400 * 1024,
			.cur_fps               = 30 * 1024,
			.max_fps               = 30 * 1024,
			.min_fps               = 5 * 1024,
			.min_afps              = 5 * 1024,
			.int_update_delay_frm  = 1,
			.gain_update_delay_frm = 1,
		},
		.mipi_info = {
			.mipi_lane = 4,
		},
		.preg_data      = ov2775_init_setting_1080p,
		.reg_data_count = ARRAY_SIZE(ov2775_init_setting_1080p),
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
		.hdr_mode       = SENSOR_MODE_HDR_STITCH,
		.stitching_mode = SENSOR_STITCHING_DUAL_DCG,
		.bit_width      = 12,
		.data_compress  = {
			.enable = 0,
		},
		.bayer_pattern  = BAYER_BGGR,
		.ae_info = {
			.def_frm_len_lines        = 0x466,
			.curr_frm_len_lines       = 0x466,
			.one_line_exp_time_ns     = 59167,

			.max_integration_line     = 0x400,
			.min_integration_line     = 16,

			.max_vsintegration_line   = 44,
			.min_vsintegration_line   = 1,

			.max_long_again = 8 * 1024 * DCG_CONVERSION_GAIN,
			.min_long_again = 1 * 1024 * DCG_CONVERSION_GAIN,
			.max_long_dgain = 3 * 1024,
			.min_long_dgain = 2 * 1024,

			.max_again      = 8 * 1024,
			.min_again      = 2 * 1024,
			.max_dgain      = 4 * 1024,
			.min_dgain      = 1.5 * 1024,

			.max_short_again = 8 * 1024,
			.min_short_again = 2 * 1024,
			.max_short_dgain = 4 * 1024,
			.min_short_dgain = 1.5 * 1024,
			.start_exposure  = 2 * 300 * 1024,

			.gain_step       = 4,
			.cur_fps         = 30 * 1024,
			.max_fps         = 30 * 1024,
			.min_fps         = 5 * 1024,
			.min_afps        = 5 * 1024,
			.hdr_ratio       = {
				.ratio_l_s = 8 * 1024,
				.ratio_s_vs = 8 * 1024,
				.accuracy = 1024,
			},
			.int_update_delay_frm = 1,
			.gain_update_delay_frm = 1,
		},
		.mipi_info = {
			.mipi_lane = 4,
		},
		.preg_data = ov2775_init_setting_1080p_hdr,
		.reg_data_count = ARRAY_SIZE(ov2775_init_setting_1080p_hdr),
	},
	{
		.index          = 2,
		.size           = {
			.bounds_width  = 1920,
			.bounds_height = 1080,
			.top           = 0,
			.left          = 0,
			.width         = 1920,
			.height        = 1080,
		},
		.hdr_mode       = SENSOR_MODE_HDR_NATIVE,
		.stitching_mode = SENSOR_STITCHING_DUAL_DCG_NOWAIT,
		.bit_width      = 12,
		.data_compress  = {
			.enable = 1,
			.x_bit  = 16,
			.y_bit  = 12,
		},
		.bayer_pattern  = BAYER_BGGR,
		.ae_info = {
			.def_frm_len_lines        = 0x466,
			.curr_frm_len_lines       = 0x466,
			.one_line_exp_time_ns     = 59167,

			.max_integration_line     = 0x466 - 4,
			.min_integration_line     = 1,

			.max_long_again = 8 * 1024 * DCG_CONVERSION_GAIN,
			.min_long_again = 1 * 1024 * DCG_CONVERSION_GAIN,
			.max_long_dgain = 3 * 1024,
			.min_long_dgain = 2 * 1024,

			.max_again      = 8 * 1024 ,
			.min_again      = 2 * 1024,
			.max_dgain      = 4 * 1024,
			.min_dgain      = 1.5 * 1024,

			.start_exposure = 3 * 400 * 1024,

			.gain_step       = 4,
			.cur_fps         = 30 * 1024,
			.max_fps         = 30 * 1024,
			.min_fps         = 5 * 1024,
			.min_afps        = 5 * 1024,
			.hdr_ratio       = {
				.ratio_l_s = 8 * 1024,
				.ratio_s_vs = 8 * 1024,
				.accuracy = 1024,
			},
			.int_update_delay_frm = 1,
			.gain_update_delay_frm = 1,
		},
		.mipi_info = {
			.mipi_lane = 4,
		},
		.preg_data = ov2775_1080p_native_hdr_regs,
		.reg_data_count = ARRAY_SIZE(ov2775_1080p_native_hdr_regs),
	},
	{
		.index          = 3,
		.size           = {
			.bounds_width  = 1920,
			.bounds_height = 1080,
			.top           = 16,
			.left          = 16,
			.width         = 1280,
			.height        = 720,
		},
		.hdr_mode       = SENSOR_MODE_LINEAR,
		.bit_width      = 12,
		.data_compress  = {
			.enable = 0,
		},
		.bayer_pattern = BAYER_BGGR,
		.ae_info = {
			.def_frm_len_lines     = 0x466,
			.curr_frm_len_lines    = 0x466,
			.one_line_exp_time_ns  = 29625,

			.max_integration_line  = 0x466 - 4,
			.min_integration_line  = 1,

			.max_again             = 8 * 1024,
			.min_again             = 2 * 1024,
			.max_dgain             = 4 * 1024,
			.min_dgain             = 1.5 * 1024,
			.gain_step             = 4,
			.start_exposure        = 3 * 400 * 1024,
			.cur_fps               = 30 * 1024,
			.max_fps               = 30 * 1024,
			.min_fps               = 5 * 1024,
			.min_afps              = 5 * 1024,
			.int_update_delay_frm  = 1,
			.gain_update_delay_frm = 1,
		},
		.mipi_info = {
			.mipi_lane = 4,
		},
		.preg_data      = ov2775_init_setting_1080p,
		.reg_data_count = ARRAY_SIZE(ov2775_init_setting_1080p),
	},
	{
		.index          = 4,
		.size           = {
			.bounds_width  = 1920,
			.bounds_height = 1080,
			.top           = 0,
			.left          = 0,
			.width         = 1920,
			.height        = 1080,
		},
		.hdr_mode       = SENSOR_MODE_HDR_STITCH,
		.stitching_mode = SENSOR_STITCHING_DUAL_DCG_NOWAIT,
		.bit_width      = 12,
		.data_compress  = {
			.enable = 0,
		},
		.bayer_pattern  = BAYER_BGGR,
		.ae_info = {
			.def_frm_len_lines        = 0x466,
			.curr_frm_len_lines       = 0x466,
			.one_line_exp_time_ns     = 59167,

			.max_integration_line     = 0x466 - 4,
			.min_integration_line     = 1,

			.max_long_again = 8 * 1024 * DCG_CONVERSION_GAIN,
			.min_long_again = 1 * 1024 * DCG_CONVERSION_GAIN,
			.max_long_dgain = 4 * 1024,
			.min_long_dgain = 2 * 1024,

			.max_again      = 8 * 1024,
			.min_again      = 2 * 1024,
			.max_dgain      = 4 * 1024,
			.min_dgain      = 1.5 * 1024,

			.start_exposure  = 3 * 400 * 1024,

			.gain_step       = 4,
			.cur_fps         = 30 * 1024,
			.max_fps         = 30 * 1024,
			.min_fps         = 5 * 1024,
			.min_afps        = 5 * 1024,
			.hdr_ratio       = {
				.ratio_l_s = 8 * 1024,
				.ratio_s_vs = 8 * 1024,
				.accuracy = 1024,
			},
			.int_update_delay_frm = 1,
			.gain_update_delay_frm = 1,
		},
		.mipi_info = {
			.mipi_lane = 4,
		},
		.preg_data = ov2775_init_setting_1080p_hdr_2dol,
		.reg_data_count = ARRAY_SIZE(ov2775_init_setting_1080p_hdr_2dol),
	},
};

int ov2775_get_clk(struct ov2775 *sensor, void *clk)
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

static int ov2775_power_on(struct ov2775 *sensor)
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

static int ov2775_power_off(struct ov2775 *sensor)
{
	pr_debug("enter %s\n", __func__);
	if (gpio_is_valid(sensor->pwn_gpio))
		gpio_set_value_cansleep(sensor->pwn_gpio, 0);
	clk_disable_unprepare(sensor->sensor_clk);

	return 0;
}

static int ov2775_s_power(struct v4l2_subdev *sd, int on)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov2775 *sensor = client_to_ov2775(client);

	pr_debug("enter %s\n", __func__);
	if (on)
		ov2775_power_on(sensor);
	else
		ov2775_power_off(sensor);

	return 0;
}

static int ov2775_write_reg(struct ov2775 *sensor, u16 reg, u8 val)
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

static int ov2775_read_reg(struct ov2775 *sensor, u16 reg, u8 *val)
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

	return 0;
}

static int ov2775_write_reg_arry(struct ov2775 *sensor,
				 struct vvcam_sccb_data_s *reg_arry,
				 u32 size)
{
	int i = 0;
	int ret = 0;
	struct i2c_msg msg;
	u8 *send_buf;
	u32 send_buf_len = 0;
	struct i2c_client *i2c_client = sensor->i2c_client;

	send_buf = (u8 *)kmalloc(size + 2, GFP_KERNEL);
	if (!send_buf)
		return -ENOMEM;

	send_buf[send_buf_len++] = (reg_arry[0].addr >> 8) & 0xff;
	send_buf[send_buf_len++] = reg_arry[0].addr & 0xff;
	send_buf[send_buf_len++] = reg_arry[0].data & 0xff;
	for (i=1; i < size; i++) {
		if (reg_arry[i].addr == (reg_arry[i-1].addr + 1)){
			send_buf[send_buf_len++] = reg_arry[i].data & 0xff;
		} else {
			msg.addr  = i2c_client->addr;
			msg.flags = i2c_client->flags;
			msg.buf   = send_buf;
			msg.len   = send_buf_len;
			ret = i2c_transfer(i2c_client->adapter, &msg, 1);
			if (ret < 0) {
				pr_err("%s:i2c transfer error\n",__func__);
				kfree(send_buf);
				return ret;
			}
			send_buf_len = 0;
			send_buf[send_buf_len++] =
				(reg_arry[i].addr >> 8) & 0xff;
			send_buf[send_buf_len++] =
				reg_arry[i].addr & 0xff;
			send_buf[send_buf_len++] =
				reg_arry[i].data & 0xff;
		}
	}

	if (send_buf_len > 0) {
		msg.addr  = i2c_client->addr;
		msg.flags = i2c_client->flags;
		msg.buf   = send_buf;
		msg.len   = send_buf_len;
		ret = i2c_transfer(i2c_client->adapter, &msg, 1);
		if (ret < 0)
			pr_err("%s:i2c transfer end meg error\n",__func__);
		else
			ret = 0;

	}
	kfree(send_buf);
	return ret;
}

static int ov2775_query_capability(struct ov2775 *sensor, void *arg)
{
	struct v4l2_capability *pcap = (struct v4l2_capability *)arg;

	strcpy((char *)pcap->driver, "ov2775");
	sprintf((char *)pcap->bus_info, "csi%d",sensor->csi_id);
	if(sensor->i2c_client->adapter) {
		pcap->bus_info[VVCAM_CAP_BUS_INFO_I2C_ADAPTER_NR_POS] =
			(__u8)sensor->i2c_client->adapter->nr;
	} else {
		pcap->bus_info[VVCAM_CAP_BUS_INFO_I2C_ADAPTER_NR_POS] = 0xFF;
	}
	return 0;
}

static int ov2775_query_supports(struct ov2775 *sensor, void* parry)
{
	int ret = 0;
	struct vvcam_mode_info_array_s *psensor_mode_arry = parry;
	uint32_t support_counts = ARRAY_SIZE(pov2775_mode_info);

	ret = copy_to_user(&psensor_mode_arry->count, &support_counts, sizeof(support_counts));
	ret |= copy_to_user(&psensor_mode_arry->modes, pov2775_mode_info,
			   sizeof(pov2775_mode_info));
	if (ret != 0)
		ret = -ENOMEM;
	return ret;

}

static int ov2775_get_sensor_id(struct ov2775 *sensor, void* pchip_id)
{
	int ret = 0;
	u16 chip_id;
	u8 chip_id_high = 0;
	u8 chip_id_low = 0;
	ret = ov2775_read_reg(sensor, 0x300a, &chip_id_high);
	ret |= ov2775_read_reg(sensor, 0x300b, &chip_id_low);

	chip_id = ((chip_id_high & 0xff) << 8) | (chip_id_low & 0xff);

	ret = copy_to_user(pchip_id, &chip_id, sizeof(u16));
	if (ret != 0)
		ret = -ENOMEM;
	return ret;
}

static int ov2775_get_reserve_id(struct ov2775 *sensor, void* preserve_id)
{
	int ret = 0;
	u16 reserve_id = 0x2770;
	ret = copy_to_user(preserve_id, &reserve_id, sizeof(u16));
	if (ret != 0)
		ret = -ENOMEM;
	return ret;
}

static int ov2775_get_sensor_mode(struct ov2775 *sensor, void* pmode)
{
	int ret = 0;
	ret = copy_to_user(pmode, &sensor->cur_mode,
		sizeof(struct vvcam_mode_info_s));
	if (ret != 0)
		ret = -ENOMEM;
	return ret;
}

static int ov2775_set_sensor_mode(struct ov2775 *sensor, void* pmode)
{
	int ret = 0;
	int i = 0;
	struct vvcam_mode_info_s sensor_mode;
	ret = copy_from_user(&sensor_mode, pmode,
		sizeof(struct vvcam_mode_info_s));
	if (ret != 0)
		return -ENOMEM;
	for (i = 0; i < ARRAY_SIZE(pov2775_mode_info); i++) {
		if (pov2775_mode_info[i].index == sensor_mode.index) {
			memcpy(&sensor->cur_mode, &pov2775_mode_info[i],
				sizeof(struct vvcam_mode_info_s));
			if ((pov2775_mode_info[i].index == 1) &&
			    (sensor->ocp.max_pixel_frequency == 266000000)) {
				sensor->cur_mode.preg_data =
					ov2775_init_setting_1080p_hdr_low_freq;
				sensor->cur_mode.reg_data_count =
					ARRAY_SIZE(ov2775_init_setting_1080p_hdr_low_freq);
				sensor->cur_mode.ae_info.one_line_exp_time_ns = 60784;
			}
			return 0;
		}
	}

	return -ENXIO;
}

static int ov2775_set_lexp(struct ov2775 *sensor, u32 exp)
{
	return 0;
}

static int ov2775_set_exp(struct ov2775 *sensor, u32 exp)
{
	int ret = 0;
	ret |= ov2775_write_reg(sensor, 0x3467, 0x00);
	ret |= ov2775_write_reg(sensor, 0x3464, 0x04);

	ret |= ov2775_write_reg(sensor, 0x30b6, (exp >> 8) & 0xff);
	ret |= ov2775_write_reg(sensor, 0x30b7, exp & 0xff);

	ret |= ov2775_write_reg(sensor, 0x3464, 0x14);
	ret |= ov2775_write_reg(sensor, 0x3467, 0x01);
	return ret;
}

static int ov2775_set_vsexp(struct ov2775 *sensor, u32 exp)
{
	int ret = 0;
	if (exp == 0x16)
		exp = 0x17;
	ret |= ov2775_write_reg(sensor, 0x3467, 0x00);
	ret |= ov2775_write_reg(sensor, 0x3464, 0x04);

	ret |= ov2775_write_reg(sensor, 0x30b8, (exp >> 8) & 0xff);
	ret |= ov2775_write_reg(sensor, 0x30b9, exp & 0xff);

	ret |= ov2775_write_reg(sensor, 0x3464, 0x14);
	ret |= ov2775_write_reg(sensor, 0x3467, 0x01);
	return ret;
}

static int ov2775_set_lgain(struct ov2775 *sensor, u32 gain)
{
	u32 again = 0;
	u32 dgain = 0;

	if (gain < ((2 * DCG_CONVERSION_GAIN) << 10)){
		gain = (2 * DCG_CONVERSION_GAIN) << 10;
	}

	if (gain < ((2 * DCG_CONVERSION_GAIN) << 10)) {
		again = 0;
	} else if (gain < ((4 * DCG_CONVERSION_GAIN) << 10)) {
		again = 1;
	} else if (gain < ((8 * DCG_CONVERSION_GAIN) << 10)) {
		again = 2;
	} else {
		again = 3;
	}
	dgain = (gain * 0x100) / ((( 1<< again) * DCG_CONVERSION_GAIN) << 10);

	sensor->hcg_again = again;
	sensor->hcg_dgain = dgain;
	return 0;
}

static int ov2775_set_gain(struct ov2775 *sensor, u32 gain)
{
	int ret = 0;
	u32 again = 0;
	u32 dgain = 0;
	u8 reg_val;

	if (gain < (3 << 10)){
		gain = 3  << 10;
	}

	if (gain < (3 << 10)) {
		again = 0;
	} else if (gain < (6 << 10)) {
		again = 1;
	} else if (gain < (12 << 10)) {
		again = 2;
	} else {
		again = 3;
	}
	dgain = (gain * 0x100) / (( 1<< again) << 10);

	if (sensor->cur_mode.hdr_mode == SENSOR_MODE_LINEAR) {
		ret = ov2775_read_reg(sensor, 0x30bb, &reg_val);
		reg_val &= ~0x03;
		reg_val |= again  & 0x03;

		ret  = ov2775_write_reg(sensor, 0x3467, 0x00);
		ret |= ov2775_write_reg(sensor, 0x3464, 0x04);

		ret |= ov2775_write_reg(sensor, 0x30bb, reg_val);
		ret |= ov2775_write_reg(sensor, 0x315a, (dgain>>8) & 0xff);
		ret |= ov2775_write_reg(sensor, 0x315b, dgain & 0xff);

		ret |= ov2775_write_reg(sensor, 0x3464, 0x14);
		ret |= ov2775_write_reg(sensor, 0x3467, 0x01);
	} else {
		ret = ov2775_read_reg(sensor, 0x30bb, &reg_val);

		reg_val &= ~0x03;
		reg_val |= sensor->hcg_again  & 0x03;

		reg_val &= ~(0x03 << 2);
		reg_val |= (again & 0x03) << 2;

		ret  = ov2775_write_reg(sensor, 0x3467, 0x00);
		ret |= ov2775_write_reg(sensor, 0x3464, 0x04);

		ret |= ov2775_write_reg(sensor, 0x30bb, reg_val);

		ret |= ov2775_write_reg(sensor, 0x315a, (sensor->hcg_dgain >> 8) & 0xff);
		ret |= ov2775_write_reg(sensor, 0x315b, sensor->hcg_dgain & 0xff);

		ret |= ov2775_write_reg(sensor, 0x315c, (dgain >> 8) & 0xff);
		ret |= ov2775_write_reg(sensor, 0x315d, dgain & 0xff);

		ret |= ov2775_write_reg(sensor, 0x3464, 0x14);
		ret |= ov2775_write_reg(sensor, 0x3467, 0x01);
	}

	return ret;
}

static int ov2775_set_vsgain(struct ov2775 *sensor, u32 gain)
{
	int ret = 0;
	u32 again = 0;
	u32 dgain = 0;
	u8 reg_val;

	if (gain < (3 << 10)) {
		gain = (3 << 10);
	}

	if (gain < (3 << 10)) {
		again = 0;
	} else if (gain < (6 << 10)) {
		again = 1;
	} else if (gain < (12 << 10)) {
		again = 2;
	} else {
		again = 3;
	}
	dgain = (gain * 0x100) / ((1 << again) << 10);

	ret = ov2775_read_reg(sensor, 0x30bb, &reg_val);
	reg_val &= ~0x30;
	reg_val |= (again & 0x03) << 4;

	ret = ov2775_write_reg(sensor, 0x3467, 0x00);
	ret |= ov2775_write_reg(sensor, 0x3464, 0x04);

	ret |= ov2775_write_reg(sensor, 0x30bb, reg_val);
	ret |= ov2775_write_reg(sensor, 0x315e, (dgain >> 8) & 0xff);
	ret |= ov2775_write_reg(sensor, 0x315f, dgain & 0xff);

	ret |= ov2775_write_reg(sensor, 0x3464, 0x14);
	ret |= ov2775_write_reg(sensor, 0x3467, 0x01);

	return ret;
}

static int ov2775_set_fps(struct ov2775 *sensor, u32 fps)
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

	ret = ov2775_write_reg(sensor, 0x30B2, (u8)(vts >> 8) & 0xff);
	ret |= ov2775_write_reg(sensor, 0x30B3, (u8)(vts & 0xff));

	sensor->cur_mode.ae_info.cur_fps = fps;

	if (sensor->cur_mode.hdr_mode == SENSOR_MODE_LINEAR) {
		sensor->cur_mode.ae_info.max_integration_line = vts - 4;
	} else {
		if (sensor->cur_mode.stitching_mode ==
		    SENSOR_STITCHING_DUAL_DCG){
			sensor->cur_mode.ae_info.max_vsintegration_line = 44;
			sensor->cur_mode.ae_info.max_integration_line = vts -
				4 - sensor->cur_mode.ae_info.max_vsintegration_line;
		} else {
			sensor->cur_mode.ae_info.max_integration_line = vts - 4;
		}
	}
	sensor->cur_mode.ae_info.curr_frm_len_lines = vts;
	return ret;
}

static int ov2775_get_fps(struct ov2775 *sensor, u32 *pfps)
{
	*pfps = sensor->cur_mode.ae_info.cur_fps;
	return 0;
}

static int ov2775_set_test_pattern(struct ov2775 *sensor, void * arg)
{
	int ret;
	struct sensor_test_pattern_s test_pattern;

	ret = copy_from_user(&test_pattern, arg, sizeof(test_pattern));
	if (ret != 0)
		return -ENOMEM;
	if (test_pattern.enable) {
		switch (test_pattern.pattern) {
		case 0:
			ret = ov2775_write_reg(sensor, 0x303a, 0x04);
			ret |= ov2775_write_reg(sensor, 0x3253, 0x80);
			break;
		case 1:
			ret = ov2775_write_reg(sensor, 0x303a, 0x04);
			ret |= ov2775_write_reg(sensor, 0x3253, 0x82);
			break;
		case 2:
			ret = ov2775_write_reg(sensor, 0x303a, 0x04);
			ret |= ov2775_write_reg(sensor, 0x3253, 0x83);
			break;
		case 3:
			ret = ov2775_write_reg(sensor, 0x303a, 0x04);
			ret |= ov2775_write_reg(sensor, 0x3253, 0x92);
			break;
		default:
			ret = -1;
			break;
		}
	} else {
		ret = ov2775_write_reg(sensor, 0x303a, 0x04);
		ret |= ov2775_write_reg(sensor, 0x3253, 0x00);
	}
	return ret;
}

static int ov2775_set_ratio(struct ov2775 *sensor, void* pratio)
{
	int ret = 0;
	struct sensor_hdr_artio_s hdr_ratio;
	struct vvcam_ae_info_s *pae_info = &sensor->cur_mode.ae_info;

	ret = copy_from_user(&hdr_ratio, pratio, sizeof(hdr_ratio));

	if ((hdr_ratio.ratio_l_s != pae_info->hdr_ratio.ratio_l_s) ||
	    (hdr_ratio.ratio_s_vs != pae_info->hdr_ratio.ratio_s_vs) ||
	    (hdr_ratio.accuracy != pae_info->hdr_ratio.accuracy)) {
		pae_info->hdr_ratio.ratio_l_s = hdr_ratio.ratio_l_s;
		pae_info->hdr_ratio.ratio_s_vs = hdr_ratio.ratio_s_vs;
		pae_info->hdr_ratio.accuracy = hdr_ratio.accuracy;
		/*ov2775 vs exp is limited for isp,so no need update max exp*/
	}

	return 0;
}

static int ov2775_set_blc(struct ov2775 *sensor, sensor_blc_t *pblc)
{
	int ret = 0;
	u32 r_offset, gr_offset, gb_offset, b_offset;
	u32 r_gain, gr_gain, gb_gain, b_gain;

	r_gain  = sensor->wb.r_gain;
	gr_gain = sensor->wb.gr_gain;
	gb_gain = sensor->wb.gb_gain;
	b_gain  = sensor->wb.b_gain;

	if (r_gain < 0x100)
		r_gain = 0x100;
	if (gr_gain < 0x100)
		gr_gain = 0x100;
	if (gb_gain < 0x100)
		gb_gain = 0x100;
	if (b_gain < 0x100)
		b_gain = 0x100;

	r_offset  = (r_gain  - 0x100) * pblc->red;
	gr_offset = (gr_gain - 0x100) * pblc->gr;
	gb_offset = (gb_gain - 0X100) * pblc->gb;
	b_offset  = (b_gain  - 0X100) * pblc->blue;
	/* R,Gr,Gb,B HCG Offset */
	ret |= ov2775_write_reg(sensor, 0x3378, (r_offset >> 16) & 0xff);
	ret |= ov2775_write_reg(sensor, 0x3379, (r_offset >> 8)  & 0xff);
	ret |= ov2775_write_reg(sensor, 0x337a,  r_offset        & 0xff);

	ret |= ov2775_write_reg(sensor, 0x337b, (gr_offset >> 16) & 0xff);
	ret |= ov2775_write_reg(sensor, 0x337c, (gr_offset >> 8)  & 0xff);
	ret |= ov2775_write_reg(sensor, 0x337d,  gr_offset        & 0xff);

	ret |= ov2775_write_reg(sensor, 0x337e, (gb_offset >> 16) & 0xff);
	ret |= ov2775_write_reg(sensor, 0x337f, (gb_offset >> 8)  & 0xff);
	ret |= ov2775_write_reg(sensor, 0x3380,  gb_offset        & 0xff);

	ret |= ov2775_write_reg(sensor, 0x3381, (b_offset >> 16) & 0xff);
	ret |= ov2775_write_reg(sensor, 0x3382, (b_offset >> 8)  & 0xff);
	ret |= ov2775_write_reg(sensor, 0x3383,  b_offset        & 0xff);

	/* R,Gr,Gb,B LCG Offset */
	ret |= ov2775_write_reg(sensor, 0x3384, (r_offset >> 16) & 0xff);
	ret |= ov2775_write_reg(sensor, 0x3385, (r_offset >> 8)  & 0xff);
	ret |= ov2775_write_reg(sensor, 0x3386,  r_offset        & 0xff);

	ret |= ov2775_write_reg(sensor, 0x3387, (gr_offset >> 16) & 0xff);
	ret |= ov2775_write_reg(sensor, 0x3388, (gr_offset >> 8)  & 0xff);
	ret |= ov2775_write_reg(sensor, 0x3389,  gr_offset        & 0xff);

	ret |= ov2775_write_reg(sensor, 0x338a, (gb_offset >> 16) & 0xff);
	ret |= ov2775_write_reg(sensor, 0x338b, (gb_offset >> 8)  & 0xff);
	ret |= ov2775_write_reg(sensor, 0x338c,  gb_offset        & 0xff);

	ret |= ov2775_write_reg(sensor, 0x338d, (b_offset >> 16) & 0xff);
	ret |= ov2775_write_reg(sensor, 0x338e, (b_offset >> 8)  & 0xff);
	ret |= ov2775_write_reg(sensor, 0x338f,  b_offset        & 0xff);

	/* R,Gr,Gb,B VS Offset */
	ret |= ov2775_write_reg(sensor, 0x3390, (r_offset >> 16) & 0xff);
	ret |= ov2775_write_reg(sensor, 0x3391, (r_offset >> 8)  & 0xff);
	ret |= ov2775_write_reg(sensor, 0x3392,  r_offset        & 0xff);

	ret |= ov2775_write_reg(sensor, 0x3393, (gr_offset >> 16) & 0xff);
	ret |= ov2775_write_reg(sensor, 0x3394, (gr_offset >> 8)  & 0xff);
	ret |= ov2775_write_reg(sensor, 0x3395,  gr_offset        & 0xff);

	ret |= ov2775_write_reg(sensor, 0x3396, (gb_offset >> 16) & 0xff);
	ret |= ov2775_write_reg(sensor, 0x3397, (gb_offset >> 8)  & 0xff);
	ret |= ov2775_write_reg(sensor, 0x3398,  gb_offset        & 0xff);

	ret |= ov2775_write_reg(sensor, 0x3399, (b_offset >> 16) & 0xff);
	ret |= ov2775_write_reg(sensor, 0x339a, (b_offset >> 8)  & 0xff);
	ret |= ov2775_write_reg(sensor, 0x339b,  b_offset        & 0xff);

	memcpy(&sensor->blc, pblc, sizeof(sensor_blc_t));
	return ret;
}

static int ov2775_set_wb(struct ov2775 *sensor, void *pwb_cfg)
{
	int ret = 0;
	bool update_flag = false;
	struct sensor_white_balance_s wb;
	ret = copy_from_user(&wb, pwb_cfg, sizeof(wb));
	if (ret != 0)
		return -ENOMEM;
	ret |= ov2775_write_reg(sensor, 0x3467, 0x00);
	ret |= ov2775_write_reg(sensor, 0x3464, 0x04);
	if (wb.r_gain != sensor->wb.r_gain) {
		ret |= ov2775_write_reg(sensor, 0x3360, (wb.r_gain >> 8) & 0xff);
		ret |= ov2775_write_reg(sensor, 0x3361,  wb.r_gain & 0xff);
		ret |= ov2775_write_reg(sensor, 0x3368, (wb.r_gain >> 8) & 0xff);
		ret |= ov2775_write_reg(sensor, 0x3369,  wb.r_gain & 0xff);
		ret |= ov2775_write_reg(sensor, 0x3370, (wb.r_gain >> 8) & 0xff);
		ret |= ov2775_write_reg(sensor, 0x3371,  wb.r_gain & 0xff);
		update_flag = true;
	}

	if (wb.gr_gain != sensor->wb.gr_gain) {
		ret |= ov2775_write_reg(sensor, 0x3362, (wb.gr_gain >> 8) & 0xff);
		ret |= ov2775_write_reg(sensor, 0x3363,  wb.gr_gain & 0xff);
		ret |= ov2775_write_reg(sensor, 0x336a, (wb.gr_gain >> 8) & 0xff);
		ret |= ov2775_write_reg(sensor, 0x336b,  wb.gr_gain & 0xff);
		ret |= ov2775_write_reg(sensor, 0x3372, (wb.gr_gain >> 8) & 0xff);
		ret |= ov2775_write_reg(sensor, 0x3373,  wb.gr_gain & 0xff);
		update_flag = true;
	}

	if (wb.gb_gain != sensor->wb.gb_gain) {
		ret |= ov2775_write_reg(sensor, 0x3364, (wb.gb_gain >> 8) & 0xff);
		ret |= ov2775_write_reg(sensor, 0x3365,  wb.gb_gain & 0xff);
		ret |= ov2775_write_reg(sensor, 0x336c, (wb.gb_gain >> 8) & 0xff);
		ret |= ov2775_write_reg(sensor, 0x336d,  wb.gb_gain & 0xff);
		ret |= ov2775_write_reg(sensor, 0x3374, (wb.gb_gain >> 8) & 0xff);
		ret |= ov2775_write_reg(sensor, 0x3375,  wb.gb_gain & 0xff);
		update_flag = true;
	}

	if (wb.b_gain != sensor->wb.b_gain) {
		ret |= ov2775_write_reg(sensor, 0x3366, (wb.b_gain >> 8) & 0xff);
		ret |= ov2775_write_reg(sensor, 0x3367,  wb.b_gain & 0xff);
		ret |= ov2775_write_reg(sensor, 0x336e, (wb.b_gain >> 8) & 0xff);
		ret |= ov2775_write_reg(sensor, 0x336f,  wb.b_gain & 0xff);
		ret |= ov2775_write_reg(sensor, 0x3376, (wb.b_gain >> 8) & 0xff);
		ret |= ov2775_write_reg(sensor, 0x3377,  wb.b_gain & 0xff);
		update_flag = true;
	}

	memcpy (&sensor->wb, &wb, sizeof(struct sensor_white_balance_s));

	if (update_flag) {
		ret = ov2775_set_blc(sensor, &sensor->blc);
	}

	ret |= ov2775_write_reg(sensor, 0x3464, 0x14);
	ret |= ov2775_write_reg(sensor, 0x3467, 0x01);

	return ret;
}

static int ov2775_get_expand_curve(struct ov2775 *sensor,
				   sensor_expand_curve_t* pexpand_curve)
{
	int i;
	if ((pexpand_curve->x_bit == 12) && (pexpand_curve->y_bit == 16)) {
		uint8_t expand_px[64] = {6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
					6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
					6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
					6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6};

		memcpy(pexpand_curve->expand_px,expand_px,sizeof(expand_px));

		pexpand_curve->expand_x_data[0] = 0;
		pexpand_curve->expand_y_data[0] = 0;
		for(i = 1; i < 65; i++) {
			pexpand_curve->expand_x_data[i] =
				(1 << pexpand_curve->expand_px[i-1]) +
				pexpand_curve->expand_x_data[i-1];

			if (pexpand_curve->expand_x_data[i] < 512) {
				pexpand_curve->expand_y_data[i] =
					pexpand_curve->expand_x_data[i] << 1;

			} else if (pexpand_curve->expand_x_data[i] < 768)
			{
				pexpand_curve->expand_y_data[i] =
					(pexpand_curve->expand_x_data[i] - 256) << 2 ;

			} else if (pexpand_curve->expand_x_data[i] < 2560) {
				pexpand_curve->expand_y_data[i] =
					(pexpand_curve->expand_x_data[i] - 512) << 3 ;

			} else {
				pexpand_curve->expand_y_data[i] =
					(pexpand_curve->expand_x_data[i] - 2048) << 5;
			}
		}
		return 0;
	}
	return -1;
}

static int ov2775_get_format_code(struct ov2775 *sensor, u32 *code)
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

static int ov2775_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov2775 *sensor = client_to_ov2775(client);
	struct i2c_msg msg;
	u8 *i2c_send_buf;
	u32 i2c_send_buf_len = 0;
	struct vvcam_sccb_data_s *sensor_reg_cfg;
	u32 sensor_reg_size = 0;
	int i = 0;
	int ret;

	pr_debug("enter %s\n", __func__);
	sensor->stream_status = enable;
	if (enable) {
		ov2775_write_reg(sensor, 0x3012, 0x01);
	} else  {
		ov2775_write_reg(sensor, 0x3012, 0x00);
		msleep(100);
		/* if the sensor re-enter streaming from standby mode
		* all registers starting with 0x7000 must be resent
		* before setting 0x3012[0]=1.
		*/
		sensor_reg_cfg =
			(struct vvcam_sccb_data_s *)sensor->cur_mode.preg_data;
		sensor_reg_size = sensor->cur_mode.reg_data_count;

		i2c_send_buf = (u8 *)kmalloc(sensor_reg_size + 2, GFP_KERNEL);
		if (!i2c_send_buf) {
			pr_err("%s:kmalloc i2c_send_buf failed\n",__func__);
			return -ENOMEM;
		}
		for (i = 0; i < sensor_reg_size; i++) {
			if (sensor_reg_cfg[i].addr >= 0x7000) {
				if (i2c_send_buf_len == 0) {
					i2c_send_buf[i2c_send_buf_len++] =
						(sensor_reg_cfg[i].addr >> 8) & 0xff;
					i2c_send_buf[i2c_send_buf_len++] =
						sensor_reg_cfg[i].addr & 0xff;
					i2c_send_buf[i2c_send_buf_len++] =
						sensor_reg_cfg[i].data & 0xff;
				} else {
					if (sensor_reg_cfg[i].addr == (sensor_reg_cfg[i-1].addr + 1)) {
						i2c_send_buf[i2c_send_buf_len++] =
							sensor_reg_cfg[i].data & 0xff;
					} else {
						msg.addr	= client->addr;
						msg.flags	= client->flags;
						msg.buf 	= i2c_send_buf;
						msg.len 	= i2c_send_buf_len;
						ret = i2c_transfer(client->adapter, &msg, 1);
						if (ret < 0) {
							kfree(i2c_send_buf);
							pr_err("%s:i2c transfer error\n",__func__);
							return -EBUSY;
						}
						i2c_send_buf_len = 0;
						i2c_send_buf[i2c_send_buf_len++] =
							(sensor_reg_cfg[i].addr >> 8) & 0xff;
						i2c_send_buf[i2c_send_buf_len++] =
							sensor_reg_cfg[i].addr & 0xff;
						i2c_send_buf[i2c_send_buf_len++] =
							sensor_reg_cfg[i].data & 0xff;
					}
				}
			}
		}
		if (i2c_send_buf_len > 0) {
			msg.addr	= client->addr;
			msg.flags	= client->flags;
			msg.buf 	= i2c_send_buf;
			msg.len 	= i2c_send_buf_len;
			ret = i2c_transfer(client->adapter, &msg, 1);
			if (ret < 0) {
				kfree(i2c_send_buf);
				pr_err("%s:i2c transfer error\n",__func__);
				return -EBUSY;
			}
		}
		kfree(i2c_send_buf);
	}

	return 0;
}
#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 12, 0)
static int ov2775_enum_mbus_code(struct v4l2_subdev *sd,
				 struct v4l2_subdev_state *state,
				 struct v4l2_subdev_mbus_code_enum *code)
#else
static int ov2775_enum_mbus_code(struct v4l2_subdev *sd,
			         struct v4l2_subdev_pad_config *cfg,
			         struct v4l2_subdev_mbus_code_enum *code)
#endif
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov2775 *sensor = client_to_ov2775(client);

	u32 cur_code = MEDIA_BUS_FMT_SBGGR12_1X12;

	if (code->index > 0)
		return -EINVAL;
	ov2775_get_format_code(sensor,&cur_code);
	code->code = cur_code;

	return 0;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 12, 0)
static int ov2775_set_fmt(struct v4l2_subdev *sd,
			  struct v4l2_subdev_state *state,
			  struct v4l2_subdev_format *fmt)
#else
static int ov2775_set_fmt(struct v4l2_subdev *sd,
			  struct v4l2_subdev_pad_config *cfg,
			  struct v4l2_subdev_format *fmt)
#endif
{
	int ret = 0;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov2775 *sensor = client_to_ov2775(client);
	mutex_lock(&sensor->lock);

	if ((fmt->format.width != sensor->cur_mode.size.bounds_width) ||
	    (fmt->format.height != sensor->cur_mode.size.bounds_height)) {
		pr_err("%s:set sensor format %dx%d error\n",
			__func__,fmt->format.width,fmt->format.height);
		mutex_unlock(&sensor->lock);
		return -EINVAL;
	}

	ov2775_write_reg(sensor, 0x3012, 0x00);
	ov2775_write_reg(sensor, 0x3013, 0x01);
	msleep(20);

	ret = ov2775_write_reg_arry(sensor,
		(struct vvcam_sccb_data_s *)sensor->cur_mode.preg_data,
		sensor->cur_mode.reg_data_count);
	if (ret < 0) {
		pr_err("%s:ov2775_write_reg_arry error\n",__func__);
		mutex_unlock(&sensor->lock);
		return -EINVAL;
	}

	ov2775_get_format_code(sensor, &fmt->format.code);
	fmt->format.field = V4L2_FIELD_NONE;
	sensor->format = fmt->format;
	mutex_unlock(&sensor->lock);
	return 0;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 12, 0)
static int ov2775_get_fmt(struct v4l2_subdev *sd,
			  struct v4l2_subdev_state *state,
			  struct v4l2_subdev_format *fmt)
#else
static int ov2775_get_fmt(struct v4l2_subdev *sd,
			  struct v4l2_subdev_pad_config *cfg,
			  struct v4l2_subdev_format *fmt)
#endif
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov2775 *sensor = client_to_ov2775(client);

	mutex_lock(&sensor->lock);
	fmt->format = sensor->format;
	mutex_unlock(&sensor->lock);
	return 0;
}

static long ov2775_priv_ioctl(struct v4l2_subdev *sd,
                              unsigned int cmd,
                              void *arg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ov2775 *sensor = client_to_ov2775(client);
	long ret = 0;
	struct vvcam_sccb_data_s sensor_reg;
	uint32_t value = 0;
	sensor_blc_t blc;
	sensor_expand_curve_t expand_curve;

	mutex_lock(&sensor->lock);
	switch (cmd){
	case VVSENSORIOC_S_POWER:
		ret = 0;
		break;
	case VVSENSORIOC_S_CLK:
		ret = 0;
		break;
	case VVSENSORIOC_G_CLK:
		ret = ov2775_get_clk(sensor,arg);
		break;
	case VVSENSORIOC_RESET:
		ret = 0;
		break;
	case VIDIOC_QUERYCAP:
		ret = ov2775_query_capability(sensor, arg);
		break;
	case VVSENSORIOC_QUERY:
		ret = ov2775_query_supports(sensor, arg);
		break;
	case VVSENSORIOC_G_CHIP_ID:
		ret = ov2775_get_sensor_id(sensor, arg);
		break;
	case VVSENSORIOC_G_RESERVE_ID:
		ret = ov2775_get_reserve_id(sensor, arg);
		break;
	case VVSENSORIOC_G_SENSOR_MODE:
		ret = ov2775_get_sensor_mode(sensor, arg);
		break;
	case VVSENSORIOC_S_SENSOR_MODE:
		ret = ov2775_set_sensor_mode(sensor, arg);
		break;
	case VVSENSORIOC_S_STREAM:
		ret = copy_from_user(&value, arg, sizeof(value));
		ret |= ov2775_s_stream(&sensor->subdev, value);
		break;
	case VVSENSORIOC_WRITE_REG:
		ret = copy_from_user(&sensor_reg, arg,
			sizeof(struct vvcam_sccb_data_s));
		ret |= ov2775_write_reg(sensor, sensor_reg.addr,
			sensor_reg.data);
		break;
	case VVSENSORIOC_READ_REG:
		ret = copy_from_user(&sensor_reg, arg,
			sizeof(struct vvcam_sccb_data_s));
		ret |= ov2775_read_reg(sensor, sensor_reg.addr,
			(u8 *)&sensor_reg.data);
		ret |= copy_to_user(arg, &sensor_reg,
			sizeof(struct vvcam_sccb_data_s));
		break;
	case VVSENSORIOC_S_LONG_EXP:
		ret = copy_from_user(&value, arg, sizeof(value));
		ret |= ov2775_set_lexp(sensor, value);
		break;
	case VVSENSORIOC_S_EXP:
		ret = copy_from_user(&value, arg, sizeof(value));
		ret |= ov2775_set_exp(sensor, value);
		break;
	case VVSENSORIOC_S_VSEXP:
		ret = copy_from_user(&value, arg, sizeof(value));
		ret |= ov2775_set_vsexp(sensor, value);
		break;
	case VVSENSORIOC_S_LONG_GAIN:
		ret = copy_from_user(&value, arg, sizeof(value));
		ret |= ov2775_set_lgain(sensor, value);
		break;
	case VVSENSORIOC_S_GAIN:
		ret = copy_from_user(&value, arg, sizeof(value));
		ret |= ov2775_set_gain(sensor, value);
		break;
	case VVSENSORIOC_S_VSGAIN:
		ret = copy_from_user(&value, arg, sizeof(value));
		ret |= ov2775_set_vsgain(sensor, value);
		break;
	case VVSENSORIOC_S_FPS:
		ret = copy_from_user(&value, arg, sizeof(value));
		ret |= ov2775_set_fps(sensor, value);
		break;
	case VVSENSORIOC_G_FPS:
		ret = ov2775_get_fps(sensor, &value);
		ret |= copy_to_user(arg, &value, sizeof(value));
		break;
	case VVSENSORIOC_S_HDR_RADIO:
		ret = ov2775_set_ratio(sensor, arg);
		break;
	case VVSENSORIOC_S_BLC:
		ret = copy_from_user(&blc, arg, sizeof(blc));
		ret |= ov2775_set_blc(sensor, &blc);
		break;
	case VVSENSORIOC_S_WB:
		ret = ov2775_set_wb(sensor, arg);
		break;
	case VVSENSORIOC_G_EXPAND_CURVE:
		ret = copy_from_user(&expand_curve, arg, sizeof(expand_curve));
		ret |= ov2775_get_expand_curve(sensor, &expand_curve);
		ret |= copy_to_user(arg, &expand_curve, sizeof(expand_curve));
		break;
	case VVSENSORIOC_S_TEST_PATTERN:
		ret= ov2775_set_test_pattern(sensor, arg);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	mutex_unlock(&sensor->lock);
	return ret;
}

static struct v4l2_subdev_video_ops ov2775_subdev_video_ops = {
	.s_stream = ov2775_s_stream,
};

static const struct v4l2_subdev_pad_ops ov2775_subdev_pad_ops = {
	.enum_mbus_code = ov2775_enum_mbus_code,
	.set_fmt = ov2775_set_fmt,
	.get_fmt = ov2775_get_fmt,
};

static struct v4l2_subdev_core_ops ov2775_subdev_core_ops = {
	.s_power = ov2775_s_power,
	.ioctl = ov2775_priv_ioctl,
};

static struct v4l2_subdev_ops ov2775_subdev_ops = {
	.core  = &ov2775_subdev_core_ops,
	.video = &ov2775_subdev_video_ops,
	.pad   = &ov2775_subdev_pad_ops,
};

static int ov2775_link_setup(struct media_entity *entity,
			     const struct media_pad *local,
			     const struct media_pad *remote, u32 flags)
{
	return 0;
}

static const struct media_entity_operations ov2775_sd_media_ops = {
	.link_setup = ov2775_link_setup,
};

static int ov2775_regulator_enable(struct ov2775 *sensor)
{
	int ret = 0;
	struct device *dev = &(sensor->i2c_client->dev);

	pr_debug("enter %s\n", __func__);

	if (sensor->io_regulator) {
		regulator_set_voltage(sensor->io_regulator,
				      OV2775_VOLTAGE_DIGITAL_IO,
				      OV2775_VOLTAGE_DIGITAL_IO);
		ret = regulator_enable(sensor->io_regulator);
		if (ret < 0) {
			dev_err(dev, "set io voltage failed\n");
			return ret;
		}
	}

	if (sensor->analog_regulator) {
		regulator_set_voltage(sensor->analog_regulator,
				      OV2775_VOLTAGE_ANALOG,
				      OV2775_VOLTAGE_ANALOG);
		ret = regulator_enable(sensor->analog_regulator);
		if (ret) {
			dev_err(dev, "set analog voltage failed\n");
			goto err_disable_io;
		}

	}

	if (sensor->core_regulator) {
		regulator_set_voltage(sensor->core_regulator,
				      OV2775_VOLTAGE_DIGITAL_CORE,
				      OV2775_VOLTAGE_DIGITAL_CORE);
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

static void ov2775_regulator_disable(struct ov2775 *sensor)
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

static int ov2775_set_clk_rate(struct ov2775 *sensor)
{
	int ret;
	unsigned int clk;

	clk = sensor->mclk;
	clk = min_t(u32, clk, (u32)OV2775_XCLK_MAX);
	clk = max_t(u32, clk, (u32)OV2775_XCLK_MIN);
	sensor->mclk = clk;

	pr_debug("   Setting mclk to %d MHz\n",sensor->mclk / 1000000);
	ret = clk_set_rate(sensor->sensor_clk, sensor->mclk);
	if (ret < 0)
		pr_debug("set rate filed, rate=%d\n", sensor->mclk);
	return ret;
}

static void ov2775_reset(struct ov2775 *sensor)
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
static int ov2775_retrieve_capture_properties(
			struct ov2775 *sensor,
			struct ov2775_capture_properties* ocp)
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
static int ov2775_probe(struct i2c_client *client)
#else
static int ov2775_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
#endif
{
	int retval;
	struct device *dev = &client->dev;
	struct v4l2_subdev *sd;
	struct ov2775 *sensor;
	u32 chip_id = 0;
	u8 reg_val = 0;

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
						"ov2775_mipi_pwdn");
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
						"ov2775_mipi_reset");
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

	retval = ov2775_retrieve_capture_properties(sensor,&sensor->ocp);
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

	retval = ov2775_regulator_enable(sensor);
	if (retval) {
		dev_err(dev, "regulator enable failed\n");
		return retval;
	}

	ov2775_set_clk_rate(sensor);
	retval = clk_prepare_enable(sensor->sensor_clk);
	if (retval < 0) {
		dev_err(dev, "%s: enable sensor clk fail\n", __func__);
		goto probe_err_regulator_disable;
	}

	retval = ov2775_power_on(sensor);
	if (retval < 0) {
		dev_err(dev, "%s: sensor power on fail\n", __func__);
		goto probe_err_regulator_disable;
	}

	ov2775_reset(sensor);

	ov2775_read_reg(sensor, 0x300a, &reg_val);
	chip_id |= reg_val << 8;
	ov2775_read_reg(sensor, 0x300b, &reg_val);
	chip_id |= reg_val;
	if (chip_id != 0x2770) {
		pr_warn("camera ov2775 is not found\n");
		retval = -ENODEV;
		goto probe_err_power_off;
	}

	sd = &sensor->subdev;
	v4l2_i2c_subdev_init(sd, client, &ov2775_subdev_ops);
	sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	sd->dev = &client->dev;
	sd->entity.ops = &ov2775_sd_media_ops;
	sd->entity.function = MEDIA_ENT_F_CAM_SENSOR;
	sensor->pads[OV2775_SENS_PAD_SOURCE].flags = MEDIA_PAD_FL_SOURCE;
	retval = media_entity_pads_init(&sd->entity,
				OV2775_SENS_PADS_NUM,
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

	memcpy(&sensor->cur_mode, &pov2775_mode_info[0],
			sizeof(struct vvcam_mode_info_s));

	mutex_init(&sensor->lock);
	pr_info("%s camera mipi ov2775, is found\n", __func__);

	return 0;

probe_err_free_entiny:
	media_entity_cleanup(&sd->entity);

probe_err_power_off:
	ov2775_power_off(sensor);

probe_err_regulator_disable:
	ov2775_regulator_disable(sensor);

	return retval;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
static int ov2775_remove(struct i2c_client *client)
#else
static void ov2775_remove(struct i2c_client *client)
#endif
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct ov2775 *sensor = client_to_ov2775(client);

	pr_info("enter %s\n", __func__);

	v4l2_async_unregister_subdev(sd);
	media_entity_cleanup(&sd->entity);
	ov2775_power_off(sensor);
	ov2775_regulator_disable(sensor);
	mutex_destroy(&sensor->lock);

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
	return 0;
#else
#endif
}

static int __maybe_unused ov2775_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct ov2775 *sensor = client_to_ov2775(client);

	sensor->resume_status = sensor->stream_status;
	if (sensor->resume_status) {
		ov2775_s_stream(&sensor->subdev,0);
	}

	return 0;
}

static int __maybe_unused ov2775_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct ov2775 *sensor = client_to_ov2775(client);

	if (sensor->resume_status) {
		ov2775_s_stream(&sensor->subdev,1);
	}

	return 0;
}

static const struct dev_pm_ops ov2775_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(ov2775_suspend, ov2775_resume)
};

static const struct i2c_device_id ov2775_id[] = {
	{"ov2775", 0},
	{},
};
MODULE_DEVICE_TABLE(i2c, ov2775_id);

static const struct of_device_id ov2775_of_match[] = {
	{ .compatible = "ovti,ov2775" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, ov2775_of_match);

static struct i2c_driver ov2775_i2c_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name  = "ov2775",
		.pm = &ov2775_pm_ops,
		.of_match_table	= ov2775_of_match,
	},
	.probe  = ov2775_probe,
	.remove = ov2775_remove,
	.id_table = ov2775_id,
};


module_i2c_driver(ov2775_i2c_driver);
MODULE_DESCRIPTION("OV2775 MIPI Camera Subdev Driver");
MODULE_LICENSE("GPL");
