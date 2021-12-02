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
#include "ar1335_regs_12MP.h"


#define AR1335_VOLTAGE_ANALOG			2800000
#define AR1335_VOLTAGE_DIGITAL_CORE		1500000
#define AR1335_VOLTAGE_DIGITAL_IO		1800000

#define AR1335_XCLK_MIN 6000000
#define AR1335_XCLK_MAX 48000000

#define MOTO_DEFAULT_DISTANCE	0 // 200 // 150   // 0-1023
#define MOTO_I2C_ADDR  0x0C

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
	struct i2c_client *moto_i2c;		/* moto */
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
};

static struct vvcam_mode_info_s par1335_mode_info[] = {
	{
		.index          = 0,
		.width          = 1920,
		.height         = 1080,
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

			.max_again             = 8 * 1024,
			.min_again             = 1 * 1024,
			.max_dgain             = 3 * 1024,
			.min_dgain             = 1 * 1024,
			.gain_step             = 2,

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
		.preg_data      = ar1335_init_setting_1080p,
		.reg_data_count = ARRAY_SIZE(ar1335_init_setting_1080p),
	},
	{
		.index          = 1,
		.width          = 1920,
		.height         = 1080,
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
			.max_long_dgain = 4 * 1024,
			.min_long_dgain = 2 * 1024,

			.max_again      = 8 * 1024,
			.min_again      = 2 * 1024,
			.max_dgain      = 4 * 1024,
			.min_dgain      = 1.5 * 1024,

			.max_short_again = 8 * 1024,
			.min_short_again = 2 * 1024,
			.max_short_dgain = 4 * 1024,
			.min_short_dgain = 1.5 * 1024,
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
		.preg_data = ar1335_init_setting_1080p,
		.reg_data_count = ARRAY_SIZE(ar1335_init_setting_1080p),
	},
	{
		.index          = 2,
		.width          = 1920,
		.height         = 1080,
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
			.max_long_dgain = 4 * 1024,
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
		.preg_data = ar1335_init_setting_1080p,
		.reg_data_count = ARRAY_SIZE(ar1335_init_setting_1080p),
	},
    {
		.index          = 3,
		.width          = 4096,
		.height         = 3072,
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

			.max_again             = 8 * 1024,
			.min_again             = 1 * 1024,
			.max_dgain             = 3 * 1024,
			.min_dgain             = 1 * 1024,
			.gain_step             = 2,

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

	return *val;
}

static s32 moto_write_reg(struct ar1335 *sensor, u8 reg, u8 val)
{
	struct device *dev = &sensor->i2c_client->dev;
	u8 au8Buf[2] = { reg , val};

	if (i2c_master_send(sensor->moto_i2c, au8Buf, 2) < 0) {
		dev_err(dev, "Write moto reg error: reg=%x, val=%x\n", reg, val);
		return -1;
	}

	return 0;
}

static s32 moto_read_reg(struct ar1335 *sensor, u8 reg, u8 *val)
{
	struct device *dev = &sensor->i2c_client->dev;
	u8 au8RegBuf[1] = { reg };
	u8 au8RdVal[1] = {0};

	if (i2c_master_send(sensor->moto_i2c, au8RegBuf, 1) != 1) {
		dev_err(dev, "Read moto reg error: reg=%x\n", reg);
		return -1;
	}

	if (i2c_master_recv(sensor->moto_i2c, au8RdVal, 1) != 1) {
		dev_err(dev, "Read moto reg error: reg=%x, val=%x\n",
                        reg, au8RdVal[0]);
		return -1;
	}

	*val = au8RdVal[0];

	return *val;
}

#if 0
static int ar1335_write_reg_arry(struct ar1335 *sensor,
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
#endif

static int ar1335_query_capability(struct ar1335 *sensor, void *arg)
{
#if 1
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
#else
    return 0;
#endif
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
#if 0
	int ret = 0;
	u16 chip_id;
	u8 chip_id_high = 0;
	u8 chip_id_low = 0;
	ret = ar1335_read_reg(sensor, 0x300a, &chip_id_high);
	ret |= ar1335_read_reg(sensor, 0x300b, &chip_id_low);

	chip_id = ((chip_id_high & 0xff) << 8) | (chip_id_low & 0xff);

	ret = copy_to_user(pchip_id, &chip_id, sizeof(u16));
	if (ret != 0)
		ret = -ENOMEM;
	return ret;
#else
    return 0;
#endif
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
    printk("enter %s\n",__func__);
	ret = copy_from_user(&sensor_mode, pmode,
		sizeof(struct vvcam_mode_info_s));
	if (ret != 0)
		return -ENOMEM;
	for (i = 0; i < ARRAY_SIZE(par1335_mode_info); i++) 
    {
		if (par1335_mode_info[i].index == sensor_mode.index) 
        {
			memcpy(&sensor->cur_mode, &par1335_mode_info[i],sizeof(struct vvcam_mode_info_s));
			if ((par1335_mode_info[i].index == 1) && (sensor->ocp.max_pixel_frequency == 266000000)) 
            {
				sensor->cur_mode.preg_data = ar1335_init_setting_1080p;
				sensor->cur_mode.reg_data_count = ARRAY_SIZE(ar1335_init_setting_1080p);
				sensor->cur_mode.ae_info.one_line_exp_time_ns = 60784;
			}
			return 0;
		}
	}
	return -ENXIO;
}

static int ar1335_set_lexp(struct ar1335 *sensor, u32 exp)
{
	return 0;
}

static int ar1335_set_exp(struct ar1335 *sensor, u32 exp)
{
#if 0
	int ret = 0;
	ret |= ar1335_write_reg(sensor, 0x3467, 0x00);
	ret |= ar1335_write_reg(sensor, 0x3464, 0x04);

	ret |= ar1335_write_reg(sensor, 0x30b6, (exp >> 8) & 0xff);
	ret |= ar1335_write_reg(sensor, 0x30b7, exp & 0xff);

	ret |= ar1335_write_reg(sensor, 0x3464, 0x14);
	ret |= ar1335_write_reg(sensor, 0x3467, 0x01);
	return ret;
#else
    printk("enter %s\n",__func__);
    return 0;
#endif
}

static int ar1335_set_vsexp(struct ar1335 *sensor, u32 exp)
{
#if 0
	int ret = 0;
	if (exp == 0x16)
		exp = 0x17;
	ret |= ar1335_write_reg(sensor, 0x3467, 0x00);
	ret |= ar1335_write_reg(sensor, 0x3464, 0x04);

	ret |= ar1335_write_reg(sensor, 0x30b8, (exp >> 8) & 0xff);
	ret |= ar1335_write_reg(sensor, 0x30b9, exp & 0xff);

	ret |= ar1335_write_reg(sensor, 0x3464, 0x14);
	ret |= ar1335_write_reg(sensor, 0x3467, 0x01);
	return ret;
#else

    printk("enter %s\n",__func__);
    return 0;
#endif
}

static int ar1335_set_lgain(struct ar1335 *sensor, u32 gain)
{
#if 0
	int ret = 0;
	u32 again = 0;
	u32 dgain = 0;
	u16 reg_val;

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

	ret = ar1335_read_reg(sensor, 0x30bb, &reg_val);
	reg_val &= ~0x03;
	reg_val |= again  & 0x03;

	ret  = ar1335_write_reg(sensor, 0x3467, 0x00);
	ret |= ar1335_write_reg(sensor, 0x3464, 0x04);

	ret |= ar1335_write_reg(sensor, 0x30bb, reg_val);
	ret |= ar1335_write_reg(sensor, 0x315a, (dgain>>8) & 0xff);
	ret |= ar1335_write_reg(sensor, 0x315b, dgain & 0xff);

	ret |= ar1335_write_reg(sensor, 0x3464, 0x14);
	ret |= ar1335_write_reg(sensor, 0x3467, 0x01);

	return ret;
#else
    return 0;
#endif
}

static const u16 reg_gain_setting [] = {
0x2010, 0x2014, 0x2018, 0x201C, 0x2020, 0x2022, 0x2024, 0x2026, 
0x2028, 0x202A, 0x202C, 0x202E, 0x2030, 0x2031, 0x2032, 0x2033, 
0x2034, 0x2035, 0x2036, 0x2037, 0x2038, 0x2039, 0x203A, 0x203B, 
0x203C, 0x203D, 0x203E, 0x203F, 0x213F, 0x223F, 0x233F, 0x243F, 
0x253F, 0x263F, 0x273F, 0x28BF, 0x29BF, 0x2ABF, 0x2BBF, 0x2CBF, 
0x2DBF, 0x2EBF, 0x2FBF, 0x30BF, 0x31BF, 0x32BF, 0x33BF, 0x34BF, 
0x35BF, 0x36BF, 0x37BF, 0x393F, 0x3A3F, 0x3B3F, 0x3C3F, 0x3D3F, 
0x3E3F, 0x3F3F, 0x403F, 0x413F, 0x423F, 0x433F, 0x443F, 0x453F, 
0x463F, 0x473F, 0x48BF, 0x49BF, 0x4ABF, 0x4BBF, 0x4CBF, 0x4DBF, 
0x4EBF, 0x4FBF, 0x50BF, 0x51BF, 0x52BF, 0x53BF, 0x54BF, 0x55BF, 
0x56BF, 0x57BF, 0x593F, 0x5A3F, 0x5B3F, 0x5C3F, 0x5D3F, 0x5E5F, 
0x5F3F, 0x603F, 0x613F, 0x623F, 0x633F
};


static int ar1335_set_gain(struct ar1335 *sensor, u32 gain)
{
#if 0
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
		ret = ar1335_read_reg(sensor, 0x30bb, &reg_val);
		reg_val &= ~0x03;
		reg_val |= again  & 0x03;

		ret  = ar1335_write_reg(sensor, 0x3467, 0x00);
		ret |= ar1335_write_reg(sensor, 0x3464, 0x04);

		ret |= ar1335_write_reg(sensor, 0x30bb, reg_val);
		ret |= ar1335_write_reg(sensor, 0x315a, (dgain>>8) & 0xff);
		ret |= ar1335_write_reg(sensor, 0x315b, dgain & 0xff);

		ret |= ar1335_write_reg(sensor, 0x3464, 0x14);
		ret |= ar1335_write_reg(sensor, 0x3467, 0x01);
	} else {
		ret = ar1335_read_reg(sensor, 0x30bb, &reg_val);
		reg_val &= ~(0x03 << 2);
		reg_val |= (again & 0x03) << 2;

		ret  = ar1335_write_reg(sensor, 0x3467, 0x00);
		ret |= ar1335_write_reg(sensor, 0x3464, 0x04);

		ret |= ar1335_write_reg(sensor, 0x30bb, reg_val);
		ret |= ar1335_write_reg(sensor, 0x315c, (dgain >> 8) & 0xff);
		ret |= ar1335_write_reg(sensor, 0x315d, dgain & 0xff);

		ret |= ar1335_write_reg(sensor, 0x3464, 0x14);
		ret |= ar1335_write_reg(sensor, 0x3467, 0x01);
	}

	return ret;
#else
	__u32 gain_set = 0;
	int ret = 0;
	gain_set = (gain / 25) * 25;
	if (gain_set < gain) {
		gain_set += 25; 
	}
	
	printk("set new gain %d, %d\n", gain, gain_set);
	if (gain_set < 100) {
		printk("invalid new gain value %d\n", gain_set);
		gain_set = 100;
	}

	ret = ar1335_write_reg(sensor, 0x305E, 0x35BF);
	gain = gain_set;
    return 0;

#endif
}

static int ar1335_set_vsgain(struct ar1335 *sensor, u32 gain)
{
#if 0
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

	ret = ar1335_read_reg(sensor, 0x30bb, &reg_val);
	reg_val &= ~0x30;
	reg_val |= (again & 0x03) << 4;

	ret = ar1335_write_reg(sensor, 0x3467, 0x00);
	ret |= ar1335_write_reg(sensor, 0x3464, 0x04);

	ret |= ar1335_write_reg(sensor, 0x30bb, reg_val);
	ret |= ar1335_write_reg(sensor, 0x315e, (dgain >> 8) & 0xff);
	ret |= ar1335_write_reg(sensor, 0x315f, dgain & 0xff);

	ret |= ar1335_write_reg(sensor, 0x3464, 0x14);
	ret |= ar1335_write_reg(sensor, 0x3467, 0x01);

	return ret;
#else
    return 0;
#endif
}

static int ar1335_set_fps(struct ar1335 *sensor, u32 fps)
{
#if 0
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

#if 0
	ret = ar1335_write_reg(sensor, 0x30B2, (u8)(vts >> 8) & 0xff);
	ret |= ar1335_write_reg(sensor, 0x30B3, (u8)(vts & 0xff));
#endif

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
#else
    printk("enter %s\n",__func__);
    return 0;
#endif
}

static int ar1335_get_fps(struct ar1335 *sensor, u32 *pfps)
{
#if 0
	*pfps = sensor->cur_mode.ae_info.cur_fps;
	return 0;
#else
    printk("enter %s\n",__func__);
	return 0;
#endif
}

static int ar1335_set_test_pattern(struct ar1335 *sensor, void * arg)
{
#if 0
	int ret;
	struct sensor_test_pattern_s test_pattern;

	ret = copy_from_user(&test_pattern, arg, sizeof(test_pattern));
	if (ret != 0)
		return -ENOMEM;
	if (test_pattern.enable) {
		switch (test_pattern.pattern) {
		case 0:
			ret = ar1335_write_reg(sensor, 0x303a, 0x04);
			ret |= ar1335_write_reg(sensor, 0x3253, 0x80);
			break;
		case 1:
			ret = ar1335_write_reg(sensor, 0x303a, 0x04);
			ret |= ar1335_write_reg(sensor, 0x3253, 0x82);
			break;
		case 2:
			ret = ar1335_write_reg(sensor, 0x303a, 0x04);
			ret |= ar1335_write_reg(sensor, 0x3253, 0x83);
			break;
		case 3:
			ret = ar1335_write_reg(sensor, 0x303a, 0x04);
			ret |= ar1335_write_reg(sensor, 0x3253, 0x92);
			break;
		default:
			ret = -1;
			break;
		}
	} else {
		ret = ar1335_write_reg(sensor, 0x303a, 0x04);
		ret |= ar1335_write_reg(sensor, 0x3253, 0x00);
	}
	return ret;
#else
    printk("enter %s\n",__func__);
    return 0;
#endif
}

static int ar1335_set_ratio(struct ar1335 *sensor, void* pratio)
{
#if 0
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
		/*ar1335 vs exp is limited for isp,so no need update max exp*/
	}

	return 0;
#else

    printk("enter %s\n",__func__);
    return 0;
#endif
}

static int ar1335_set_blc(struct ar1335 *sensor, sensor_blc_t *pblc)
{
#if 0
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
	//R,Gr,Gb,B HCG Offset
	ret |= ar1335_write_reg(sensor, 0x3378, (r_offset >> 16) & 0xff);
	ret |= ar1335_write_reg(sensor, 0x3379, (r_offset >> 8)  & 0xff);
	ret |= ar1335_write_reg(sensor, 0x337a,  r_offset        & 0xff);

	ret |= ar1335_write_reg(sensor, 0x337b, (gr_offset >> 16) & 0xff);
	ret |= ar1335_write_reg(sensor, 0x337c, (gr_offset >> 8)  & 0xff);
	ret |= ar1335_write_reg(sensor, 0x337d,  gr_offset        & 0xff);

	ret |= ar1335_write_reg(sensor, 0x337e, (gb_offset >> 16) & 0xff);
	ret |= ar1335_write_reg(sensor, 0x337f, (gb_offset >> 8)  & 0xff);
	ret |= ar1335_write_reg(sensor, 0x3380,  gb_offset        & 0xff);

	ret |= ar1335_write_reg(sensor, 0x3381, (b_offset >> 16) & 0xff);
	ret |= ar1335_write_reg(sensor, 0x3382, (b_offset >> 8)  & 0xff);
	ret |= ar1335_write_reg(sensor, 0x3383,  b_offset        & 0xff);

	//R,Gr,Gb,B LCG Offset
	ret |= ar1335_write_reg(sensor, 0x3384, (r_offset >> 16) & 0xff);
	ret |= ar1335_write_reg(sensor, 0x3385, (r_offset >> 8)  & 0xff);
	ret |= ar1335_write_reg(sensor, 0x3386,  r_offset        & 0xff);

	ret |= ar1335_write_reg(sensor, 0x3387, (gr_offset >> 16) & 0xff);
	ret |= ar1335_write_reg(sensor, 0x3388, (gr_offset >> 8)  & 0xff);
	ret |= ar1335_write_reg(sensor, 0x3389,  gr_offset        & 0xff);

	ret |= ar1335_write_reg(sensor, 0x338a, (gb_offset >> 16) & 0xff);
	ret |= ar1335_write_reg(sensor, 0x338b, (gb_offset >> 8)  & 0xff);
	ret |= ar1335_write_reg(sensor, 0x338c,  gb_offset        & 0xff);

	ret |= ar1335_write_reg(sensor, 0x338d, (b_offset >> 16) & 0xff);
	ret |= ar1335_write_reg(sensor, 0x338e, (b_offset >> 8)  & 0xff);
	ret |= ar1335_write_reg(sensor, 0x338f,  b_offset        & 0xff);

	//R,Gr,Gb,B VS Offset
	ret |= ar1335_write_reg(sensor, 0x3390, (r_offset >> 16) & 0xff);
	ret |= ar1335_write_reg(sensor, 0x3391, (r_offset >> 8)  & 0xff);
	ret |= ar1335_write_reg(sensor, 0x3392,  r_offset        & 0xff);

	ret |= ar1335_write_reg(sensor, 0x3393, (gr_offset >> 16) & 0xff);
	ret |= ar1335_write_reg(sensor, 0x3394, (gr_offset >> 8)  & 0xff);
	ret |= ar1335_write_reg(sensor, 0x3395,  gr_offset        & 0xff);

	ret |= ar1335_write_reg(sensor, 0x3396, (gb_offset >> 16) & 0xff);
	ret |= ar1335_write_reg(sensor, 0x3397, (gb_offset >> 8)  & 0xff);
	ret |= ar1335_write_reg(sensor, 0x3398,  gb_offset        & 0xff);

	ret |= ar1335_write_reg(sensor, 0x3399, (b_offset >> 16) & 0xff);
	ret |= ar1335_write_reg(sensor, 0x339a, (b_offset >> 8)  & 0xff);
	ret |= ar1335_write_reg(sensor, 0x339b,  b_offset        & 0xff);

	memcpy(&sensor->blc, pblc, sizeof(sensor_blc_t));
	return ret;
#else

    printk("enter %s\n",__func__);
    return 0;
#endif
}

static int ar1335_set_wb(struct ar1335 *sensor, void *pwb_cfg)
{
#if 0
	int ret = 0;
	bool update_flag = false;
	struct sensor_white_balance_s wb;
	ret = copy_from_user(&wb, pwb_cfg, sizeof(wb));
	if (ret != 0)
		return -ENOMEM;
	if (wb.r_gain != sensor->wb.r_gain) {
		ret |= ar1335_write_reg(sensor, 0x3360, (wb.r_gain >> 8) & 0xff);
		ret |= ar1335_write_reg(sensor, 0x3361,  wb.r_gain & 0xff);
		ret |= ar1335_write_reg(sensor, 0x3368, (wb.r_gain >> 8) & 0xff);
		ret |= ar1335_write_reg(sensor, 0x3369,  wb.r_gain & 0xff);
		ret |= ar1335_write_reg(sensor, 0x3370, (wb.r_gain >> 8) & 0xff);
		ret |= ar1335_write_reg(sensor, 0x3371,  wb.r_gain & 0xff);
		update_flag = true;
	}

	if (wb.gr_gain != sensor->wb.gr_gain) {
		ret |= ar1335_write_reg(sensor, 0x3362, (wb.gr_gain >> 8) & 0xff);
		ret |= ar1335_write_reg(sensor, 0x3363,  wb.gr_gain & 0xff);
		ret |= ar1335_write_reg(sensor, 0x336a, (wb.gr_gain >> 8) & 0xff);
		ret |= ar1335_write_reg(sensor, 0x336b,  wb.gr_gain & 0xff);
		ret |= ar1335_write_reg(sensor, 0x3372, (wb.gr_gain >> 8) & 0xff);
		ret |= ar1335_write_reg(sensor, 0x3373,  wb.gr_gain & 0xff);
		update_flag = true;
	}

	if (wb.gb_gain != sensor->wb.gb_gain) {
		ret |= ar1335_write_reg(sensor, 0x3364, (wb.gb_gain >> 8) & 0xff);
		ret |= ar1335_write_reg(sensor, 0x3365,  wb.gb_gain & 0xff);
		ret |= ar1335_write_reg(sensor, 0x336c, (wb.gb_gain >> 8) & 0xff);
		ret |= ar1335_write_reg(sensor, 0x336d,  wb.gb_gain & 0xff);
		ret |= ar1335_write_reg(sensor, 0x3374, (wb.gb_gain >> 8) & 0xff);
		ret |= ar1335_write_reg(sensor, 0x3375,  wb.gb_gain & 0xff);
		update_flag = true;
	}

	if (wb.b_gain != sensor->wb.b_gain) {
		ret |= ar1335_write_reg(sensor, 0x3366, (wb.b_gain >> 8) & 0xff);
		ret |= ar1335_write_reg(sensor, 0x3367,  wb.b_gain & 0xff);
		ret |= ar1335_write_reg(sensor, 0x336e, (wb.b_gain >> 8) & 0xff);
		ret |= ar1335_write_reg(sensor, 0x336f,  wb.b_gain & 0xff);
		ret |= ar1335_write_reg(sensor, 0x3376, (wb.b_gain >> 8) & 0xff);
		ret |= ar1335_write_reg(sensor, 0x3377,  wb.b_gain & 0xff);
		update_flag = true;
	}
	if (ret != 0)
		return ret;

	memcpy (&sensor->wb, &wb, sizeof(struct sensor_white_balance_s));

	if (update_flag) {
		ret = ar1335_set_blc(sensor, &sensor->blc);
	}
	return ret;
#else

    printk("enter %s\n",__func__);
    return 0;
#endif
}

static int ar1335_get_expand_curve(struct ar1335 *sensor,
				   sensor_expand_curve_t* pexpand_curve)
{
#if 0
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
#else

    printk("enter %s\n",__func__);
    return 0;
#endif
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

static int ar1335_stream_on(struct ar1335 *sensor)
{
	int ret;
	u16 val = 0;
	pr_info("enter %s\n", __func__);
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
	pr_info("enter %s\n", __func__);
	ret = ar1335_read_reg(sensor, AR1335_RESET_REG, &val);
	if (ret < 0)
		return ret;
	val &= (~0x0004);
	ret = ar1335_write_reg(sensor, AR1335_RESET_REG, val);

	return ret;
}

static int ar1335_s_stream(struct v4l2_subdev *sd, int enable)
{
#if 0
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ar1335 *sensor = client_to_ar1335(client);
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
		ar1335_write_reg(sensor, 0x3012, 0x01);
	} else  {
		ar1335_write_reg(sensor, 0x3012, 0x00);
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
#else
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ar1335 *sensor = client_to_ar1335(client);
    int ret;

	printk("enter %s\n", __func__);
	if (enable) {
        ret = ar1335_stream_on(sensor);
        if (ret < 0)
            return ret;
    }
	else {
        ret = ar1335_stream_off(sensor);
        if (ret < 0)
            return ret;
    }

	return 0;

#endif
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

struct ar1335_datafmt {
	u32 code;
	enum v4l2_colorspace colorspace;
};


static const struct ar1335_datafmt ar1335_colour_fmts[] = {
	{MEDIA_BUS_FMT_SGRBG10_1X10, V4L2_COLORSPACE_JPEG},
};

/* Find a data format by a pixel code in an array */
static const struct ar1335_datafmt
                *ar1335_find_datafmt(u32 code)
{
	int i;

    printk("enter %s\n", __func__);
	for (i = 0; i < ARRAY_SIZE(ar1335_colour_fmts); i++)
		if (ar1335_colour_fmts[i].code == code)
			return ar1335_colour_fmts + i;

	return NULL;
}

/* download ar1335 settings to sensor through i2c */
static int ar1335_download_firmware(struct ar1335 *sensor,
				    struct vvsensor_ar1335_reg_value_t *mode_setting,
				    s32 size)
{
	register u32 delay_ms = 0;
	register u16 reg_addr = 0;
	register u16 mask = 0;
	register u16 val = 0;
	u16 reg_val = 0;
	int i, retval = 0;

	pr_info("enter %s\n", __func__);
	for (i = 0; i < size; ++i, ++mode_setting) {
		delay_ms = mode_setting->delay;
		reg_addr = mode_setting->addr;
		val = mode_setting->val;
		mask = mode_setting->mask;

		if (mask) {
			printk("mask\n");
			retval = ar1335_read_reg(sensor, reg_addr, &reg_val);
			if (retval < 0)
				break;

			reg_val &= ~(u16)mask;
			val &= mask;
			val |= reg_val;
		}

		retval = ar1335_write_reg(sensor, reg_addr, val);
		if (retval < 0)
			break;

		if (delay_ms)
			msleep(delay_ms);
	}

    ar1335_stream_off(sensor);

	return retval;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 12, 0)
static int ar1335_set_fmt(struct v4l2_subdev *sd,
			  struct v4l2_subdev_state *state,
			  struct v4l2_subdev_format *format)
#else
static int ar1335_set_fmt(struct v4l2_subdev *sd,
			  struct v4l2_subdev_pad_config *cfg,
			  struct v4l2_subdev_format *format)

#endif
{
#if 0
	int ret = 0;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ar1335 *sensor = client_to_ar1335(client);
	mutex_lock(&sensor->lock);

	if ((fmt->format.width != sensor->cur_mode.width) ||
	    (fmt->format.height != sensor->cur_mode.height)) {
		pr_err("%s:set sensor format %dx%d error\n",
			__func__,fmt->format.width,fmt->format.height);
		mutex_unlock(&sensor->lock);
		return -EINVAL;
	}

	ar1335_write_reg(sensor, 0x3013, 0x01);
	msleep(10);

	ret = ar1335_write_reg_arry(sensor,
		(struct vvcam_sccb_data_s *)sensor->cur_mode.preg_data,
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
#else
	struct v4l2_mbus_framefmt *mf = &format->format;
	const struct ar1335_datafmt *fmt = ar1335_find_datafmt(mf->code);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ar1335 *sensor = client_to_ar1335(client);
	unsigned int i;
	struct vvsensor_ar1335_reg_value_t *mode_setting = NULL;
	int array_size = 0;

	pr_debug("enter %s\n", __func__);
	if (format->pad) {
		return -EINVAL;
	}

	if (!fmt) {
		mf->code = ar1335_colour_fmts[0].code;
		mf->colorspace = ar1335_colour_fmts[0].colorspace;
	}

	mf->field = V4L2_FIELD_NONE;
	/*  old search method,  vsi need change to
	    search resolution by width/height */
	/*  try_to_find_resolution(sensor, mf); */
	if (format->which == V4L2_SUBDEV_FORMAT_TRY)
		return 0;

	for (i=0; i<ARRAY_SIZE(par1335_mode_info); i++)
	{
		if (0)
		{
		    if (mf->width == par1335_mode_info[i].width && 
			    mf->height == par1335_mode_info[i].height &&
			    par1335_mode_info[i].hdr_mode != SENSOR_MODE_LINEAR)
			{
				memcpy(&(sensor->cur_mode), &par1335_mode_info[i], sizeof(struct vvcam_mode_info_s));
				mode_setting = par1335_mode_info[i].preg_data;
				array_size = par1335_mode_info[i].reg_data_count;
				return ar1335_download_firmware(sensor, mode_setting, array_size);
			}
		}else
		{
			if (mf->width == par1335_mode_info[i].width && 
			    mf->height == par1335_mode_info[i].height &&
			    par1335_mode_info[i].hdr_mode == SENSOR_MODE_LINEAR)
			{
				memcpy(&(sensor->cur_mode), &par1335_mode_info[i], sizeof(struct vvcam_mode_info_s));
				mode_setting = par1335_mode_info[i].preg_data;
				array_size = par1335_mode_info[i].reg_data_count;
				return ar1335_download_firmware(sensor, mode_setting, array_size);
			}
		}
	}

	pr_err("%s search error: %d %d\n", __func__, mf->width, mf->height);
	return -EINVAL;;

#endif
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

    printk("enter %s\n",__func__);
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
	case VVSENSORIOC_S_LONG_EXP:
		ret = copy_from_user(&value, arg, sizeof(value));
		ret |= ar1335_set_lexp(sensor, value);
		break;
	case VVSENSORIOC_S_EXP:
		ret = copy_from_user(&value, arg, sizeof(value));
		ret |= ar1335_set_exp(sensor, value);
		break;
	case VVSENSORIOC_S_VSEXP:
		ret = copy_from_user(&value, arg, sizeof(value));
		ret |= ar1335_set_vsexp(sensor, value);
		break;
	case VVSENSORIOC_S_LONG_GAIN:
		ret = copy_from_user(&value, arg, sizeof(value));
		ret |= ar1335_set_lgain(sensor, value);
		break;
	case VVSENSORIOC_S_GAIN:
		ret = copy_from_user(&value, arg, sizeof(value));
		ret |= ar1335_set_gain(sensor, value);
		break;
	case VVSENSORIOC_S_VSGAIN:
		ret = copy_from_user(&value, arg, sizeof(value));
		ret |= ar1335_set_vsgain(sensor, value);
		break;
	case VVSENSORIOC_S_FPS:
		ret = copy_from_user(&value, arg, sizeof(value));
		ret |= ar1335_set_fps(sensor, value);
		break;
	case VVSENSORIOC_G_FPS:
		ret = ar1335_get_fps(sensor, &value);
		ret |= copy_to_user(arg, &value, sizeof(value));
		break;
	case VVSENSORIOC_S_HDR_RADIO:
		ret = ar1335_set_ratio(sensor, arg);
		break;
	case VVSENSORIOC_S_BLC:
		ret = copy_from_user(&blc, arg, sizeof(blc));
		ret |= ar1335_set_blc(sensor, &blc);
		break;
	case VVSENSORIOC_S_WB:
		ret = ar1335_set_wb(sensor, arg);
		break;
	case VVSENSORIOC_G_EXPAND_CURVE:
		ret = copy_from_user(&expand_curve, arg, sizeof(expand_curve));
		ret |= ar1335_get_expand_curve(sensor, &expand_curve);
		ret |= copy_to_user(arg, &expand_curve, sizeof(expand_curve));
		break;
	case VVSENSORIOC_S_TEST_PATTERN:
		ret= ar1335_set_test_pattern(sensor, arg);
		break;
	default:
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

#if 1
static int ar1335_config_init(struct ar1335 *sensor)
{
	struct vvsensor_ar1335_reg_value_t *mode_setting = NULL;
	int array_size = 0;
	int retval = 0;

   printk("enter %s\n",__func__);
   mode_setting = par1335_mode_info[3].preg_data;
   array_size = par1335_mode_info[3].reg_data_count;
   retval = ar1335_download_firmware(sensor, mode_setting, array_size);
	if (retval < 0)
		return retval;

	return 0;
}

static int init_device(struct ar1335 *sensor)
{
	int retval;
	retval = ar1335_config_init(sensor);
	if (retval < 0)
		return retval;

	return 0;
}
#endif

static int ar1335_probe(struct i2c_client *client,
                        const struct i2c_device_id *id)
{
	int retval;
	struct device *dev = &client->dev;
	struct v4l2_subdev *sd;
	struct ar1335 *sensor;
    u16 chip_id;

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

	sensor->moto_i2c = i2c_new_dummy_device(client->adapter,
						MOTO_I2C_ADDR);
	if (!sensor->moto_i2c) {
		// retval = -ENODEV;
		return -ENODEV;
	}

	retval = ar1335_power_on(sensor);
	if (retval < 0) {
		dev_err(dev, "%s: sensor power on fail\n", __func__);
		goto probe_err_regulator_disable;
	}


	ar1335_reset(sensor);

    printk("test ar1335 I2C read/write operations:\n");

    ar1335_read_reg(sensor, AR1335_CHIP_VERSION_REG, &chip_id);
	if (chip_id != AR1335_CHIP_ID)
    {
		dev_err(dev, "Sensor AR1335 is not found\n");
        return -ENODEV;
    }

    printk("chip_id = 0x%x\n",chip_id);

#if 1
	retval = init_device(sensor);
	if (retval < 0) {
		clk_disable_unprepare(sensor->sensor_clk);
		pr_warn("camera ar1335 init fail\n");
		return -ENODEV;
	}
#endif

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

	memcpy(&sensor->cur_mode, &par1335_mode_info[3],
			sizeof(struct vvcam_mode_info_s));

	mutex_init(&sensor->lock);

#ifdef MOTO_DEFAULT_DISTANCE
	{
		u8 val;
		u16 dist;
		// init 
		retval = moto_write_reg(sensor, 0x2, 0x0);
		if (retval == 0) {
			// set distance 10bit
			dist = MOTO_DEFAULT_DISTANCE << 6;

			// low 2bit
			retval = moto_read_reg(sensor, 0x1, &val);
			if (retval == 0) {
				val = val & 0x3f;
				val = val | (dist & 0xc0);
				retval = moto_write_reg(sensor, 0x1, val);
			}

			// high 8bit
			retval = moto_write_reg(sensor, 0x0, (dist >> 8) & 0xff);

			moto_read_reg(sensor, 0x0, &val);
			dist = val << 2;
			moto_read_reg(sensor, 0x1, &val);
			dist += (val >> 6) & 0x3;
			printk("set default distance to %x\n", dist);
		}
		else {
			printk("moto init failed\n");
		}
	}
#endif




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

static int ar1335_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct ar1335 *sensor = client_to_ar1335(client);

	pr_info("enter %s\n", __func__);

	v4l2_async_unregister_subdev(sd);
	media_entity_cleanup(&sd->entity);
	ar1335_power_off(sensor);
	ar1335_regulator_disable(sensor);
	mutex_destroy(&sensor->lock);

	return 0;
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
