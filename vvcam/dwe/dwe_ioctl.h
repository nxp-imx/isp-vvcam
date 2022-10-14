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
#ifndef _DWE_IOC_H_
#define _DWE_IOC_H_
#include "dwe_dev.h"

enum {
	DWEIOC_RESET = 0x100,
	DWEIOC_S_PARAMS,
	DWEIOC_ENABLE_BUS,
	DWEIOC_DISABLE_BUS,
	DWEIOC_DISABLE_IRQ,
	DWEIOC_CLEAR_IRQ,
	DWEIOC_READ_IRQ,
	DWEIOC_START,
	DWEIOC_STOP,
	DWEIOC_START_DMA_READ,
	DWEIOC_SET_BUFFER,
	DWEIOC_SET_LUT,
	DWEIOC_GET_LUT_STATUS,
};

struct lut_info {
	u32 port;
	u64 addr;
};

long dwe_priv_ioctl(struct dwe_ic_dev *dev, unsigned int cmd, void *args);

int dwe_reset(struct dwe_ic_dev *dev);
int dwe_s_params(struct dwe_ic_dev *dev, struct dwe_hw_info *info);
int dwe_enable_bus(struct dwe_ic_dev *dev, bool enable);
int dwe_disable_irq(struct dwe_ic_dev *dev);
int dwe_clear_irq(struct dwe_ic_dev *dev);
int dwe_read_irq(struct dwe_ic_dev *dev, u32 *ret);
int dwe_start_dma_read(struct dwe_ic_dev *dev,
				struct dwe_hw_info *info, u64 addr);
int dwe_set_buffer(struct dwe_ic_dev *dev, struct dwe_hw_info *info, u64 addr);
int dwe_set_lut(struct dwe_ic_dev *dev, u64 addr);
#ifdef __KERNEL__
irqreturn_t dwe_hw_isr(int irq, void *data);
void dwe_clear_interrupts(struct dwe_ic_dev *dev);
void dwe_isr_tasklet(unsigned long arg);
void dwe_clean_src_memory(struct dwe_ic_dev *dev);
#endif
#endif /* _DWE_IOC_H_ */
