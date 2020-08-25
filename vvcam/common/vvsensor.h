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
#ifndef _VVSENSOR_PUBLIC_HEADER_H_
#define _VVSENSOR_PUBLIC_HEADER_H_

#ifndef __KERNEL__
#include <stdint.h>
#else
#include <linux/uaccess.h>
#endif

enum {
	VVSENSORIOC_RESET = 0x100,
	VVSENSORIOC_S_CLK,
	VVSENSORIOC_G_CLK,
	VVSENSORIOC_S_POWER,
	VVSENSORIOC_G_POWER,
	VVSENSORIOC_SENSOR_SCCB_CFG,
	VVSENSORIOC_FOCUS_SCCB_CFG,
	VVSENSORIOC_READ_REG,
	VVSENSORIOC_WRITE_REG,
	VVSENSORIOC_READ_ARRAY,
	VVSENSORIOC_WRITE_ARRAY,
	VVSENSORIOC_AF_READ_REG,
	VVSENSORIOC_AF_WRITE_REG,
	VVSENSORIOC_G_MIPI,
	VVSENSORIOC_G_NAME,
	VVSENSORIOC_G_RESERVE_ID,
	VVSENSORIOC_G_CHIP_ID,
	VVSENSORIOC_S_INIT,
	VVSENSORIOC_S_STREAM,
	VVSENSORIOC_S_EXP,
	VVSENSORIOC_S_VSEXP,
	VVSENSORIOC_S_GAIN,
	VVSENSORIOC_S_VSGAIN,
	VVSENSORIOC_S_FRAMESIZE,
	VVSENSORIOC_ENUM_FRAMESIZES,
	VVSENSORIOC_S_HDR_MODE,
	VVSENSORIOC_G_HDR_MODE,
	VVSENSORIOC_S_FPS,
	VVSENSORIOC_G_FPS,
	VVSENSORIOC_S_HDR_RADIO,
	VVSENSORIOC_G_AE_INFO,
	VVSENSORIOC_QUERY,
	VVSENSORIOC_G_SENSOR_MODE,
	VVSENSORIOC_MAX,
};

/* W/R registers */
struct vvcam_sccb_data {
	uint32_t addr;
	uint32_t data;
};

/* init settings */
struct vvsensor_reg_value_t {
	uint16_t addr;
	uint8_t val;
	uint8_t mask;
	uint32_t delay;
};

/* priv ioctl */
struct vvsensor_gain_context {
	uint32_t again;
	uint32_t dgain;
};

/* vsi native usage */
struct vvcam_sccb_cfg_s {
	uint8_t slave_addr;
	uint8_t addr_byte;
	uint8_t data_byte;
};

struct vvcam_sccb_array {
	uint32_t count;
	struct vvcam_sccb_data *sccb_data;
};

typedef struct vvcam_ae_info_s {
	uint32_t MaxFrameLengthLines;
	uint32_t CurFrameLengthLines;
	uint32_t one_line_exp_time_ns;
	uint32_t max_interrgation_time;
	uint32_t min_interrgation_time;
	uint32_t interrgation_accuracy;
	uint32_t max_gain;
	uint32_t min_gain;
	uint32_t gain_accuracy;
	uint32_t cur_fps;
	uint32_t hdr_radio;
} vvcam_ae_info_t;

struct sensor_mipi_info {
	uint32_t mipi_lane;
	uint32_t sensor_data_bit;
};

enum sensor_hdr_mode_s
{
	SENSOR_MODE_LINEAR ,
	SENSOR_MODE_HDR_STITCH,
	SENSOR_MODE_HDR_NATIVE,
};

enum SENSOR_BAYER_PATTERN_E
{
    BAYER_BGGR    = 0,
    BAYER_GBRG    = 1,
    BAYER_GRBG    = 2,
    BAYER_RGGB    = 3,
    BAYER_BUTT
};

enum sensor_stitching_mode_e
{
	SENSOR_STITCHING_DUAL_DCG        = 0,   /**< dual DCG mode 3x12-bit */
	SENSOR_STITCHING_3DOL            = 1,   /**< dol3 frame 3x12-bit */
	SENSOR_STITCHING_LINEBYLINE      = 2,   /**< 3x12-bit line by line without waiting */
	SENSOR_STITCHING_16BIT_COMPRESS  = 3,   /**< 16-bit compressed data + 12-bit RAW */
	SENSOR_STITCHING_DUAL_DCG_NOWAIT = 4,   /**< 2x12-bit dual DCG without waiting */
	SENSOR_STITCHING_2DOL            = 5,   /**< dol2 frame or 1 CG+VS sx12-bit RAW */
	SENSOR_STITCHING_L_AND_S         = 6,   /**< L+S 2x12-bit RAW */
	SENSOR_STITCHING_MAX

};

#define VVCAM_SUPPORT_MAX_MODE_COUNT 20

typedef struct vvcam_mode_info {
	uint32_t index;
	uint32_t width;
	uint32_t height;
	uint32_t fps;
	uint32_t hdr_mode;
	uint32_t stitching_mode;
	uint32_t bit_width;
	uint32_t bayer_pattern;
	vvcam_ae_info_t ae_info;
	void *preg_data;
	uint32_t reg_data_count;
	
} vvcam_mode_info_t;

typedef struct vvcam_mode_info_arry {
	uint32_t count;
	struct vvcam_mode_info modes[VVCAM_SUPPORT_MAX_MODE_COUNT];
} vvcam_mode_info_arry_t;

#endif

