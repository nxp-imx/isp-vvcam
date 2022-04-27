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
#ifndef _DWE_DRIVER_H_
#define _DWE_DRIVER_H_

#include "dwe_dev.h"
#include "video/vvbuf.h"

#ifdef ENABLE_IRQ
struct dwe_devcore {
	struct vvbuf_ctx bctx[DWE_PADS_NUM];
	struct dwe_ic_dev ic_dev;
	struct media_pad *src_pads[MAX_DWE_NUM];
	struct mutex mutex;
	int state;
	refcount_t refcount;
	resource_size_t start;
	resource_size_t end;
	int (*match)(struct dwe_devcore *core, struct resource *res);
	int irq;
	struct list_head entry;
};
#endif

struct dwe_device {
	struct vvbuf_ctx bctx[DWE_PADS_NUM];
	/* Driver private data */
	struct v4l2_subdev sd;
	struct fwnode_handle fwnode;
#ifdef ENABLE_IRQ
	struct dwe_devcore *core;
	struct media_pad pads[DWE_PADS_NUM];
	int state;
	int id;
	int irq;
#else
	struct v4l2_device *vd;
	struct dwe_ic_dev ic_dev;
#endif
	struct clk *clk_core;
	struct clk *clk_axi;
	struct clk *clk_ahb;
	int refcnt;
};

#ifdef ENABLE_IRQ
struct dwe_devcore *dwe_devcore_init(struct dwe_device *dwe,
				struct resource *res);
void dwe_devcore_deinit(struct dwe_device *dwe);
long dwe_devcore_ioctl(struct dwe_device *dwe, unsigned int cmd, void *args);
#endif
#endif /* _DWE_DRIVER_H_ */
