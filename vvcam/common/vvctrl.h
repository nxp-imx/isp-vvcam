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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
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
#ifndef _VVCTRL_H_
#define _VVCTRL_H_

#include <linux/videodev2.h>

#define VIV_CUSTOM_CID_BASE (V4L2_CID_USER_BASE | 0xf000)
#define V4L2_CID_VIV_STRING (VIV_CUSTOM_CID_BASE + 0x01)
#define V4L2_CID_VIV_SENSOR_MODE (VIV_CUSTOM_CID_BASE + 0x02)
#define V4L2_CID_VIV_SENSOR_RES_W (VIV_CUSTOM_CID_BASE + 0x03)
#define V4L2_CID_VIV_SENSOR_RES_H (VIV_CUSTOM_CID_BASE + 0x04)
#define V4L2_CID_VIV_SENSOR_TPG_ENABLED_STATUS (VIV_CUSTOM_CID_BASE + 0x05)
#define V4L2_CID_VIV_DEWARP_MODE (VIV_CUSTOM_CID_BASE + 0x06)
#define V4L2_CID_VIV_DEWARP_BYPASS_STATUS (VIV_CUSTOM_CID_BASE + 0x07)
#define V4L2_CID_VIV_AEC_ENABLED_STATUS (VIV_CUSTOM_CID_BASE + 0x08)
#define V4L2_CID_VIV_AWB_ENABLED_STATUS (VIV_CUSTOM_CID_BASE + 0x09)
#define V4L2_CID_VIV_AWB_DAMPING_STATUS (VIV_CUSTOM_CID_BASE + 0x0A)
#define V4L2_CID_VIV_LSC_ENABLED_STATUS (VIV_CUSTOM_CID_BASE + 0x0B)
#define V4L2_CID_VIV_CPROC_ENABLED_STATUS (VIV_CUSTOM_CID_BASE + 0x0C)
#define V4L2_CID_VIV_GAMMA_ENABLED_STATUS (VIV_CUSTOM_CID_BASE + 0x0D)
#define V4L2_CID_VIV_GAMMA_MODE (VIV_CUSTOM_CID_BASE + 0x0E)
#define V4L2_CID_VIV_DEMOSAIC_MODE (VIV_CUSTOM_CID_BASE + 0x0F)
#define V4L2_CID_VIV_FILTER_ENABLED_STATUS (VIV_CUSTOM_CID_BASE + 0x10)
#define V4L2_CID_VIV_CAC_ENABLED_STATUS (VIV_CUSTOM_CID_BASE + 0x11)
#define V4L2_CID_VIV_DPCC_ENABLED_STATUS (VIV_CUSTOM_CID_BASE + 0x12)
#define V4L2_CID_VIV_CNR_ENABLED_STATUS (VIV_CUSTOM_CID_BASE + 0x13)
#define V4L2_CID_VIV_DPF_ENABLED_STATUS (VIV_CUSTOM_CID_BASE + 0x14)
#define V4L2_CID_VIV_WDR3_ENABLED_STATUS (VIV_CUSTOM_CID_BASE + 0x15)
#define V4L2_CID_VIV_AVS_ENABLED_STATUS (VIV_CUSTOM_CID_BASE + 0x16)
#define V4L2_CID_VIV_2DNR_ENABLED_STATUS (VIV_CUSTOM_CID_BASE + 0x17)
#define V4L2_CID_VIV_3DNR_ENABLED_STATUS (VIV_CUSTOM_CID_BASE + 0x18)
#define V4L2_CID_VIV_HDR_ENABLED_STATUS (VIV_CUSTOM_CID_BASE + 0x19)
#define V4L2_CID_VIV_HDR_MODE (VIV_CUSTOM_CID_BASE + 0x1A)
#define V4L2_CID_VIV_STITCHING_MODE (VIV_CUSTOM_CID_BASE + 0x1B)
#define V4L2_CID_VIV_IS_OUT_RES_W (VIV_CUSTOM_CID_BASE + 0x1C)
#define V4L2_CID_VIV_IS_OUT_RES_H (VIV_CUSTOM_CID_BASE + 0x1D)
#define V4L2_CID_VIV_MP_OUT_RES_W (VIV_CUSTOM_CID_BASE + 0x1E)
#define V4L2_CID_VIV_MP_OUT_RES_H (VIV_CUSTOM_CID_BASE + 0x1F)
#define V4L2_CID_VIV_MP_OUT_FORMAT (VIV_CUSTOM_CID_BASE + 0x20)
#define V4L2_CID_VIV_PIPELINE_SMP_MODE (VIV_CUSTOM_CID_BASE + 0x21)
#define V4L2_CID_VIV_PIPELINE_DWE_ENABLED_STATUS (VIV_CUSTOM_CID_BASE + 0x22)

enum v4l2_ctrl_direction {
	V4L2_CTRL_GET,
	V4L2_CTRL_SET,
};

struct v4l2_ctrl_data {
	struct v4l2_ext_controls ctrls;
	enum v4l2_ctrl_direction dir;
	int ret;
};

#define nextof(ptr, new_type) ((new_type)((ptr) + 1))

#endif /* _VVCTRL_H_ */
