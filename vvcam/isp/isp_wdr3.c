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

#include <linux/io.h>
#include <linux/module.h>
#include "mrv_all_bits.h"
#include "isp_ioctl.h"
#include "isp_types.h"

#define WDR3_WW						(32)
#define WDR3_HH						(32)
#define WDR3_MAX_VALUE				(1023)
#define WDR3_GAIN_SHIFT				(2)
#define WDR3_NORMALIZE				(1024)
#define WDR3_NORMALIZE_SHIFT		(10)
#ifdef ISP_WDR_V3_20BIT
#define MODULE_INPUT_BIT_DEPTH		(20)
#else
#define MODULE_INPUT_BIT_DEPTH		(12)
#endif
#define MODULE_OUTPUT_BIT_DEPTH		(12)

extern MrvAllRegister_t *all_regs;

static void wdr3_hw_init(struct isp_ic_dev *dev)
{
	struct isp_wdr3_context *wdr3 = &dev->wdr3;
	u32 isp_wdr3_shift_0;
	u32 isp_wdr3_shift_1;

	u32 width, height;
	u32 slice_block_area_factor;
	u32 slice_value_weight[4];
	u32 slice_pixel_slope_merge;
	u32 slice_pixel_base_merge;
	u32 slice_pixel_slope_adjust;
	u32 slice_pixel_base_adjust;
	u32 slice_pixel_slope_entropy;
	u32 slice_pixel_base_entropy;
	u32 slice_sigma_height;
	u32 slice_sigma_width;
	u32 slice_sigma_value;
	u32 slice_block_width;
	u32 slice_block_height;
	u32 isp_wdr3_block_size;
	u32 isp_wdr3_block_area_factor;
	u32 isp_wdr3_value_weight;
	u32 isp_wdr3_pixel_slope;
	u32 isp_wdr3_entropy_slope;
	u32 isp_wdr3_sigma_width;
	u32 isp_wdr3_sigma_height;
	u32 isp_wdr3_sigma_value;
	u32 isp_wdr3_block_flag_width;
	u32 isp_wdr3_block_flag_height;
	u32 isp_wdr3_strength;
	u32 width_left;
	u32 width_count = 0;
	u32 height_left;
	u32 height_count = 0;
	u32 val;
	bool reg_flag = false;
	int i, pos;

	pr_info("enter %s\n", __func__);

	width = isp_read_reg(dev, REG_ADDR(isp_out_h_size));
	height = isp_read_reg(dev, REG_ADDR(isp_out_v_size));

	pr_info("wdr3 res: %d %d \n", width, height);
	/* firware initilization */
	slice_pixel_slope_merge = 128;
	slice_pixel_base_merge = 0;
	slice_pixel_slope_adjust = 128;
	slice_pixel_base_adjust = 0;
	slice_pixel_slope_entropy = 204;
	slice_pixel_base_entropy = 716;

	slice_value_weight[0] = 16;
	slice_value_weight[1] = 5;
	slice_value_weight[2] = 5;
	slice_value_weight[3] = 6;

	slice_block_width = width / WDR3_WW;
	slice_block_height = height / WDR3_HH;
	slice_block_area_factor =
	    WDR3_NORMALIZE * WDR3_NORMALIZE / (slice_block_width *
					       slice_block_height);
	slice_sigma_height =
	    WDR3_NORMALIZE * WDR3_NORMALIZE / slice_block_height;
	slice_sigma_width = WDR3_NORMALIZE * WDR3_NORMALIZE / slice_block_width;
	slice_sigma_value = WDR3_NORMALIZE * WDR3_NORMALIZE / WDR3_MAX_VALUE;

	/* block flag configuration */
	width_left = width - slice_block_width * WDR3_WW;
	height_left = height - slice_block_height * WDR3_HH;
	isp_wdr3_block_flag_width = 0;
	isp_wdr3_block_flag_height = 0;

	for (i = 0, width_count = 0;
	     (i < WDR3_WW) && (width_count < width_left); i++, width_count++) {
		isp_wdr3_block_flag_width |= (1 << i);
	}

	for (i = 0, height_count = 0;
	     (i < WDR3_HH) && (height_count < height_left);
	     i++, height_count++) {
		isp_wdr3_block_flag_height |= (1 << i);
	}

	slice_pixel_base_adjust += 255;
	slice_pixel_base_merge += 255;

#ifndef __KERNEL__
	/*NOTE: register isp_wdr3_shift is read-only register on fpga, can not write
	  NOTE: it is used by cmodel, So it should  be config. */
	u32 slice_pixel_shift;
	u32 slice_output_shift;
	u32 isp_wdr3_shift;

	slice_pixel_shift = MODULE_INPUT_BIT_DEPTH - 5;
	slice_output_shift = MODULE_INPUT_BIT_DEPTH - MODULE_OUTPUT_BIT_DEPTH;

	isp_wdr3_shift = isp_read_reg(dev, REG_ADDR(isp_wdr3_shift));
	REG_SET_SLICE(isp_wdr3_shift, WDR3_PIXEL_SHIFT_BIT, slice_pixel_shift);
	REG_SET_SLICE(isp_wdr3_shift, WDR3_OUTPUT_SHIFT_BIT,
		      slice_output_shift);
	isp_write_reg(dev, REG_ADDR(isp_wdr3_shift), isp_wdr3_shift);
#endif

	isp_wdr3_block_size = isp_read_reg(dev, REG_ADDR(isp_wdr3_block_size));
	REG_SET_SLICE(isp_wdr3_block_size, WDR3_BLOCK_WIDTH, slice_block_width);
	REG_SET_SLICE(isp_wdr3_block_size, WDR3_BLOCK_HEIGHT,
		      slice_block_height);
	isp_write_reg(dev, REG_ADDR(isp_wdr3_block_size), isp_wdr3_block_size);

	isp_wdr3_block_area_factor =
	    isp_read_reg(dev, REG_ADDR(isp_wdr3_block_area_factor));
	REG_SET_SLICE(isp_wdr3_block_area_factor, WDR3_BLOCK_AREA_INVERSE,
		      slice_block_area_factor);
	isp_write_reg(dev, REG_ADDR(isp_wdr3_block_area_factor),
		      isp_wdr3_block_area_factor);
	isp_wdr3_value_weight =
	    isp_read_reg(dev, REG_ADDR(isp_wdr3_value_weight));
	REG_SET_SLICE(isp_wdr3_value_weight, WDR3_VALUE_WEIGHT_0,
		      slice_value_weight[0]);
	REG_SET_SLICE(isp_wdr3_value_weight, WDR3_VALUE_WEIGHT_1,
		      slice_value_weight[1]);
	REG_SET_SLICE(isp_wdr3_value_weight, WDR3_VALUE_WEIGHT_2,
		      slice_value_weight[2]);
	REG_SET_SLICE(isp_wdr3_value_weight, WDR3_VALUE_WEIGHT_3,
		      slice_value_weight[3]);
	isp_write_reg(dev, REG_ADDR(isp_wdr3_value_weight),
		      isp_wdr3_value_weight);

	isp_wdr3_strength = isp_read_reg(dev, REG_ADDR(isp_wdr3_strength));
	REG_SET_SLICE(isp_wdr3_strength, WDR3_MAXIMUM_GAIN,  wdr3->max_gain);
	REG_SET_SLICE(isp_wdr3_strength, WDR3_GLOBAL_STRENGTH,
		      wdr3->global_strength);
	REG_SET_SLICE(isp_wdr3_strength, WDR3_LOCAL_STRENGTH,
		      128);
	REG_SET_SLICE(isp_wdr3_strength, WDR3_TOTAL_STRENGTH, wdr3->strength);
	isp_write_reg(dev, REG_ADDR(isp_wdr3_strength), isp_wdr3_strength);

	isp_wdr3_pixel_slope =
	    isp_read_reg(dev, REG_ADDR(isp_wdr3_pixel_slope));
	REG_SET_SLICE(isp_wdr3_pixel_slope, WDR3_PIXEL_ADJUST_BASE,
		      slice_pixel_base_adjust);
	REG_SET_SLICE(isp_wdr3_pixel_slope, WDR3_PIXEL_ADJUST_SLOPE,
		      slice_pixel_slope_adjust);
	REG_SET_SLICE(isp_wdr3_pixel_slope, WDR3_PIXEL_MERGE_BASE,
		      slice_pixel_base_merge);
	REG_SET_SLICE(isp_wdr3_pixel_slope, WDR3_PIXEL_MERGE_SLOPE,
		      slice_pixel_slope_merge);
	isp_write_reg(dev, REG_ADDR(isp_wdr3_pixel_slope),
		      isp_wdr3_pixel_slope);

	isp_wdr3_entropy_slope =
	    isp_read_reg(dev, REG_ADDR(isp_wdr3_entropy_slope));
	REG_SET_SLICE(isp_wdr3_entropy_slope, WDR3_ENTROPY_BASE,
		      slice_pixel_base_entropy);
	REG_SET_SLICE(isp_wdr3_entropy_slope, WDR3_ENTROPY_SLOPE,
		      slice_pixel_slope_entropy);
	isp_write_reg(dev, REG_ADDR(isp_wdr3_entropy_slope),
		      isp_wdr3_entropy_slope);

	isp_wdr3_sigma_width =
	    isp_read_reg(dev, REG_ADDR(isp_wdr3_sigma_width));
	REG_SET_SLICE(isp_wdr3_sigma_width, WDR3_BILITERAL_WIDTH_SIGMA,
		      slice_sigma_width);
	isp_write_reg(dev, REG_ADDR(isp_wdr3_sigma_width),
		      isp_wdr3_sigma_width);

	isp_wdr3_sigma_height =
	    isp_read_reg(dev, REG_ADDR(isp_wdr3_sigma_height));
	REG_SET_SLICE(isp_wdr3_sigma_height, WDR3_BILITERAL_HEIGHT_SIGMA,
		      slice_sigma_height);
	isp_write_reg(dev, REG_ADDR(isp_wdr3_sigma_height),
		      isp_wdr3_sigma_height);

	isp_wdr3_sigma_value =
	    isp_read_reg(dev, REG_ADDR(isp_wdr3_sigma_value));
	REG_SET_SLICE(isp_wdr3_sigma_value, WDR3_BILITERAL_VALUE_SIGMA,
		      slice_sigma_value);
	isp_write_reg(dev, REG_ADDR(isp_wdr3_sigma_value),
		      isp_wdr3_sigma_value);

	isp_write_reg(dev, REG_ADDR(isp_wdr3_block_flag_width),
		      isp_wdr3_block_flag_width);
	isp_write_reg(dev, REG_ADDR(isp_wdr3_block_flag_height),
		      isp_wdr3_block_flag_height);

	for (i = 0; i < 5; i++) {
		reg_flag = i < 4;
		pos = i * 3;
		val =
		    wdr3->histogram[pos] << (reg_flag ?
					     WDR3_HISTOGRAM_CURVE0_SHIFT :
					     WDR3_HISTOGRAM_CURVE1_SHIFT);
		val |=
		    wdr3->histogram[pos +
				    1] << (reg_flag ?
					   WDR3_HISTOGRAM_CURVE1_SHIFT :
					   WDR3_HISTOGRAM_CURVE2_SHIFT);
		if (reg_flag)
			REG_SET_SLICE(val, WDR3_HISTOGRAM_CURVE2,
				      wdr3->histogram[pos + 2]);
		isp_write_reg(dev, REG_ADDR(isp_wdr3_histogram[i]), val);

		val =
		    wdr3->entropy[pos] << (reg_flag ?
					   WDR3_ENTROPY_CONVERT0_SHIFT :
					   WDR3_ENTROPY_CONVERT1_SHIFT);
		val |=
		    wdr3->entropy[pos +
				  1] << (reg_flag ? WDR3_ENTROPY_CONVERT1_SHIFT
					 : WDR3_ENTROPY_CONVERT2_SHIFT);
		if (reg_flag)
			REG_SET_SLICE(val, WDR3_ENTROPY_CONVERT2,
				      wdr3->entropy[pos + 2]);
		isp_write_reg(dev, REG_ADDR(isp_wdr3_entropy[i]), val);

		val =
		    wdr3->gamma_pre[pos] << (reg_flag ?
					     WDR3_GAMMA_PRE_CURVE0_SHIFT :
					     WDR3_GAMMA_PRE_CURVE1_SHIFT);
		val |=
		    wdr3->gamma_pre[pos +
				    1] << (reg_flag ?
					   WDR3_GAMMA_PRE_CURVE1_SHIFT :
					   WDR3_GAMMA_PRE_CURVE2_SHIFT);
		if (reg_flag)
			REG_SET_SLICE(val, WDR3_GAMMA_PRE_CURVE2,
				      wdr3->gamma_pre[pos + 2]);
		isp_write_reg(dev, REG_ADDR(isp_wdr3_gamma_pre[i]), val);

		val =
		    wdr3->gamma_up[pos] << (reg_flag ?
					    WDR3_GAMMA_UP_CURVE0_SHIFT :
					    WDR3_GAMMA_UP_CURVE1_SHIFT);
		val |=
		    wdr3->gamma_up[pos +
				   1] << (reg_flag ? WDR3_GAMMA_UP_CURVE1_SHIFT
					  : WDR3_GAMMA_UP_CURVE2_SHIFT);
		if (reg_flag)
			REG_SET_SLICE(val, WDR3_GAMMA_UP_CURVE2,
				      wdr3->gamma_up[pos + 2]);
		isp_write_reg(dev, REG_ADDR(isp_wdr3_gamma_up[i]), val);

		val =
		    wdr3->gamma_down[pos] << (reg_flag ?
					      WDR3_GAMMA_DOWN_CURVE0_SHIFT :
					      WDR3_GAMMA_DOWN_CURVE1_SHIFT);
		val |=
		    wdr3->gamma_down[pos +
				     1] << (reg_flag ?
					    WDR3_GAMMA_DOWN_CURVE1_SHIFT :
					    WDR3_GAMMA_DOWN_CURVE2_SHIFT);
		if (reg_flag)
			REG_SET_SLICE(val, WDR3_GAMMA_DOWN_CURVE2,
				      wdr3->gamma_down[pos + 2]);
		isp_write_reg(dev, REG_ADDR(isp_wdr3_gamma_down[i]), val);

		val =
		    wdr3->distance_weight[pos] << (reg_flag ?
						   WDR3_DISTANCE_WEIGHT_CURVE0_SHIFT
						   :
						   WDR3_DISTANCE_WEIGHT_CURVE1_SHIFT);
		val |=
		    wdr3->distance_weight[pos +
					  1] << (reg_flag ?
						 WDR3_DISTANCE_WEIGHT_CURVE1_SHIFT
						 :
						 WDR3_DISTANCE_WEIGHT_CURVE2_SHIFT);
		if (reg_flag)
			REG_SET_SLICE(val, WDR3_DISTANCE_WEIGHT_CURVE2,
				      wdr3->distance_weight[pos + 2]);
		isp_write_reg(dev, REG_ADDR(isp_wdr3_distance_weight[i]), val);

		val =
		    wdr3->difference_weight[pos] << (reg_flag ?
						     WDR3_DIFFERENCE_WEIGHT_CURVE0_SHIFT
						     :
						     WDR3_DIFFERENCE_WEIGHT_CURVE1_SHIFT);
		val |=
		    wdr3->difference_weight[pos +
					    1] << (reg_flag ?
						   WDR3_DIFFERENCE_WEIGHT_CURVE1_SHIFT
						   :
						   WDR3_DIFFERENCE_WEIGHT_CURVE2_SHIFT);
		if (reg_flag)
			REG_SET_SLICE(val, WDR3_DIFFERENCE_WEIGHT_CURVE2,
				      wdr3->difference_weight[pos + 2]);
		isp_write_reg(dev, REG_ADDR(isp_wdr3_difference_weight[i]),
			      val);
	}

	for (i = 0; i < 7; i++) {
		val = 0;
		REG_SET_SLICE(val, WDR3_GLOBAL_CURVE_INVERT0,
			      wdr3->invert_curve[i * 2]);
		REG_SET_SLICE(val, WDR3_GLOBAL_CURVE_INVERT1,
			      wdr3->invert_curve[i * 2 + 1]);
		isp_write_reg(dev, REG_ADDR(isp_wdr3_invert_curve[i]), val);
		val = 0;
		REG_SET_SLICE(val, WDR3_LINEAR_CURVE_INVERT0,
			      wdr3->invert_linear[i * 2]);
		REG_SET_SLICE(val, WDR3_LINEAR_CURVE_INVERT1,
			      wdr3->invert_linear[i * 2 + 1]);
		isp_write_reg(dev, REG_ADDR(isp_wdr3_invert_linear[i]), val);
	}

	isp_wdr3_shift_0 = 0;
	REG_SET_SLICE(isp_wdr3_shift_0, WDR3_HISTOGRAM_SHIFT0, wdr3->shift[0]);
	REG_SET_SLICE(isp_wdr3_shift_0, WDR3_HISTOGRAM_SHIFT1, wdr3->shift[1]);
	REG_SET_SLICE(isp_wdr3_shift_0, WDR3_HISTOGRAM_SHIFT2, wdr3->shift[2]);
	REG_SET_SLICE(isp_wdr3_shift_0, WDR3_HISTOGRAM_SHIFT3, wdr3->shift[3]);
	REG_SET_SLICE(isp_wdr3_shift_0, WDR3_HISTOGRAM_SHIFT4, wdr3->shift[4]);
	REG_SET_SLICE(isp_wdr3_shift_0, WDR3_HISTOGRAM_SHIFT5, wdr3->shift[5]);
	REG_SET_SLICE(isp_wdr3_shift_0, WDR3_HISTOGRAM_SHIFT6, wdr3->shift[6]);
	REG_SET_SLICE(isp_wdr3_shift_0, WDR3_HISTOGRAM_SHIFT7, wdr3->shift[7]);
	isp_write_reg(dev, REG_ADDR(isp_wdr3_shift_0), isp_wdr3_shift_0);

	isp_wdr3_shift_1 = 0;
	REG_SET_SLICE(isp_wdr3_shift_1, WDR3_HISTOGRAM_SHIFT8, wdr3->shift[8]);
	REG_SET_SLICE(isp_wdr3_shift_1, WDR3_HISTOGRAM_SHIFT9, wdr3->shift[9]);
	REG_SET_SLICE(isp_wdr3_shift_1, WDR3_HISTOGRAM_SHIFT10,
		      wdr3->shift[10]);
	REG_SET_SLICE(isp_wdr3_shift_1, WDR3_HISTOGRAM_SHIFT11,
		      wdr3->shift[11]);
	REG_SET_SLICE(isp_wdr3_shift_1, WDR3_HISTOGRAM_SHIFT12,
		      wdr3->shift[12]);
	REG_SET_SLICE(isp_wdr3_shift_1, WDR3_HISTOGRAM_SHIFT13,
		      wdr3->shift[13]);
	isp_write_reg(dev, REG_ADDR(isp_wdr3_shift_1), isp_wdr3_shift_1);
	
	{
	uint32_t isp_wdr3_ctrl = isp_read_reg(dev, REG_ADDR(isp_wdr3_ctrl));
	REG_SET_SLICE(isp_wdr3_ctrl, WDR3_ENABLE, 1);
	isp_write_reg(dev, REG_ADDR(isp_wdr3_ctrl), isp_wdr3_ctrl);
	}
}

int isp_enable_wdr3(struct isp_ic_dev *dev)
{
#ifndef ISP_WDR_V3
	pr_err("unsupported function: %s", __func__);
	return -EINVAL;
#else
	struct isp_wdr3_context *wdr3 = &dev->wdr3;
	int32_t isp_wdr3_ctrl = isp_read_reg(dev, REG_ADDR(isp_wdr3_ctrl));
	wdr3->enable = true;
	REG_SET_SLICE(isp_wdr3_ctrl, WDR3_ENABLE, 1);
	isp_write_reg(dev, REG_ADDR(isp_wdr3_ctrl), isp_wdr3_ctrl);

	{
	uint32_t isp_ctrl = isp_read_reg(dev, REG_ADDR(isp_ctrl));
	REG_SET_SLICE(isp_ctrl, MRV_ISP_ISP_GEN_CFG_UPD, 1);
	isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);
	}
	return 0;
#endif
}

int isp_disable_wdr3(struct isp_ic_dev *dev)
{
#ifndef ISP_WDR_V3
	pr_err("unsupported function: %s", __func__);
	return -EINVAL;
#else
	struct isp_wdr3_context *wdr3 = &dev->wdr3;
	int32_t isp_wdr3_ctrl = isp_read_reg(dev, REG_ADDR(isp_wdr3_ctrl));
	wdr3->enable = false;
	REG_SET_SLICE(isp_wdr3_ctrl, WDR3_ENABLE, 0);
	isp_write_reg(dev, REG_ADDR(isp_wdr3_ctrl), isp_wdr3_ctrl);

	{
	uint32_t isp_ctrl = isp_read_reg(dev, REG_ADDR(isp_ctrl));
	REG_SET_SLICE(isp_ctrl, MRV_ISP_ISP_GEN_CFG_UPD, 1);
	isp_write_reg(dev, REG_ADDR(isp_ctrl), isp_ctrl);
	}
	return 0;
#endif
}

int isp_u_wdr3(struct isp_ic_dev *dev)
{
#ifndef ISP_WDR_V3
	pr_err("unsupported function: %s", __func__);
	return -EINVAL;
#else
	wdr3_hw_init(dev);
	return 0;
#endif
}

int isp_s_wdr3(struct isp_ic_dev *dev)
{
#ifndef ISP_WDR_V3
	pr_err("unsupported function: %s", __func__);
	return -EINVAL;
#else
	struct isp_wdr3_context *wdr3 = &dev->wdr3;
	u32 isp_wdr3_strength = isp_read_reg(dev, REG_ADDR(isp_wdr3_strength));
	u32 width = isp_read_reg(dev, REG_ADDR(isp_out_h_size));
	u32 height = isp_read_reg(dev, REG_ADDR(isp_out_v_size));
	width /= 32;
	height /= 32;

	REG_SET_SLICE(isp_wdr3_strength, WDR3_MAXIMUM_GAIN, wdr3->max_gain);
	REG_SET_SLICE(isp_wdr3_strength, WDR3_GLOBAL_STRENGTH,
		      wdr3->global_strength);
	REG_SET_SLICE(isp_wdr3_strength, WDR3_LOCAL_STRENGTH, 128);
	REG_SET_SLICE(isp_wdr3_strength, WDR3_TOTAL_STRENGTH, wdr3->strength);

	isp_write_reg(dev, REG_ADDR(isp_wdr3_block_size),
		      width | (height << 9));
	isp_write_reg(dev, REG_ADDR(isp_wdr3_strength), isp_wdr3_strength);

	return 0;
#endif
}
