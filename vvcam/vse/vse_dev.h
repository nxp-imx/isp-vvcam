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
#ifndef _VSE_DEV_H_
#define _VSE_DEV_H_

#define VSE_OUTPUT_CHANNEL_NUMBER 3
#define VSE_INPUT_CHANNEL_NUMBER 6
/*
 *  NOTICE NOTICE NOTICE:
 *  First scale output max resolution is 4096*2160  (max size 4096*2160*3 )
 *  Second scale output max resolution is 1920*1080
 *  Third scale output max resolution is 1280*720
 */

#ifndef __KERNEL__
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define pr_info printf
#define pr_err printf
#define copy_from_user(a, b, c) vse_copy_data(a, b, c)
#define copy_to_user(a, b, c) vse_copy_data(a, b, c)
#define __user
#define __iomem

typedef bool(*pVseReadBar) (uint32_t bar, uint32_t * data);
typedef bool(*pVseWriteBar) (uint32_t bar, uint32_t data);

extern void vse_set_func(pVseReadBar read_func, pVseWriteBar write_func);
extern long vse_copy_data(void *dst, void *src, int size);

#endif

#include "vvdefs.h"

struct vse_crop_size {
	u32 left;
	u32 right;
	u32 top;
	u32 bottom;
};

struct vse_size {
	u32 width;
	u32 height;
};

struct vse_format_conv_settings {
	u32 in_format;
	u32 out_format;
};

struct vse_mi_settings {
	bool enable;
	u32 out_format;
	u32 width;
	u32 height;
};

struct vse_params {
	u32 src_w;
	u32 src_h;
	u32 in_format;
	u32 input_select;
	struct vse_crop_size crop_size[3];
	struct vse_size out_size[3];
	struct vse_format_conv_settings format_conv[3];
	bool resize_enable[3];
	struct vse_mi_settings mi_settings[3];
};

struct vse_ic_dev {
	struct vse_params params;
	void __iomem *base;
	void __iomem *reset;
};

#endif /* _VSE_DEV_H_ */
