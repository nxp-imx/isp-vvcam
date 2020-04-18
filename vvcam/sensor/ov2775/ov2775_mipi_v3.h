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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
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
#ifndef _VVCAM_OV2775_MIPI_V3_H_
#define _VVCAM_OV2775_MIPI_V3_H_

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
	ov2775_mode_MAX = 0,
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

struct reg_value {
	u16 u16RegAddr;
	u8 u8Val;
	u8 u8Mask;
	u32 u32Delay_ms;
};

struct ov2775_mode_info {
	enum ov2775_mode mode;
	enum ov2775_downsize_mode dn_mode;
	u32 width;
	u32 height;
	struct reg_value *init_data_ptr;
	u32 init_data_size;
};

struct ov2775_pll_info {
	enum ov2775_mode mode;
	struct reg_value *init_data_ptr;
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
	struct clk *sensor_clk;
	int csi;

	void (*io_init) (struct ov2775 *);
	int pwn_gpio, rst_gpio;
	bool hdr;
};

#define client_to_ov2775(client)	container_of(i2c_get_clientdata(client), struct ov2775, subdev)

int ov2775_hw_register(struct v4l2_device *vdev);
void ov2775_hw_unregister(void);
s32 ov2775_write_reg(struct ov2775 *sensor, u16 reg, u8 val);
s32 ov2775_read_reg(struct ov2775 *sensor, u16 reg, u8 * val);

#endif
