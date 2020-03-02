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
#include "sam_ioctl.h"
#include "mxc-mipi-csi2-sam.h"

int csis_s_fmt(struct v4l2_subdev *sd, struct csi_sam_format *fmt)
{
	u32 code;
	const struct csis_pix_format *csis_format;
	struct csi_state *state = container_of(sd, struct csi_state, sd);

	switch (fmt->format) {
	case V4L2_PIX_FMT_SBGGR12:
		code = MEDIA_BUS_FMT_SBGGR12_1X12;
		break;
	default:
		return -EINVAL;
	}
	csis_format = find_csis_format(code);
	if (csis_format == NULL)
		return -EINVAL;

	state->csis_fmt = csis_format;
	state->format.width = fmt->width;
	state->format.height = fmt->height;
	disp_mix_gasket_config(state);
	mipi_csis_set_params(state);
	return 0;
}

long csis_priv_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	int ret = 1;
	struct csi_state *state = container_of(sd, struct csi_state, sd);

	pr_info("enter %s\n", __func__);
	switch (cmd) {
	case CSIOC_RESET:
		mipi_csis_sw_reset(state);
		ret = 0;
		break;
	case CSIOC_POWERON:
		ret = mipi_csis_s_power(sd, 1);
		break;
	case CSIOC_POWEROFF:
		ret = mipi_csis_s_power(sd, 0);
		break;
	case CSIOC_STREAMON:
		ret = mipi_csis_s_stream(sd, 1);
		break;
	case CSIOC_STREAMOFF:
		ret = mipi_csis_s_stream(sd, 0);
		break;
	case CSIOC_S_FMT:
		ret = csis_s_fmt(sd, (struct csi_sam_format *)arg);
	default:
		// pr_err("unsupported csi-sam command %d.", cmd);
		break;
	}

	return ret;
}

