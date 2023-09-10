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
#include "dwe_driver.h"
#include "dwe_ioctl.h"

LIST_HEAD(devcore_list);
static DEFINE_SPINLOCK(devcore_list_lock);

long dwe_devcore_ioctl(struct dwe_device *dwe, unsigned int cmd, void *args)
{
	struct dwe_ic_dev *dev = &dwe->core->ic_dev;
	int which;
	long ret = 0;
	uint32_t lut_status;

	switch (cmd) {
	case DWEIOC_RESET:
		break;
	case DWEIOC_S_PARAMS:
		which = dev->which[dwe->id]; /*just set the current one*/
		viv_check_retval(copy_from_user(&dev->info[dwe->id][which],
				args, sizeof(dev->info[dwe->id][which])));
		break;
	case DWEIOC_START:
		if (dwe->state & STATE_DRIVER_STARTED)
			break;
		if (dwe->core->state == 0) {
			ret = dwe_priv_ioctl(&dwe->core->ic_dev,
					DWEIOC_RESET, NULL);
			ret |= dwe_priv_ioctl(&dwe->core->ic_dev, cmd, args);
		}
		dwe->core->state++;
		dwe->state |= STATE_DRIVER_STARTED;
		break;
	case DWEIOC_STOP:
		if (!(dwe->state & STATE_DRIVER_STARTED))
			break;
		dwe->state &= ~STATE_DRIVER_STARTED;
		dwe->core->state--;
		if (dwe->core->state == 0) {
			ret = dwe_priv_ioctl(&dwe->core->ic_dev, cmd, args);
			msleep(1);
			dwe_clean_src_memory(&dwe->core->ic_dev);
			dwe->core->ic_dev.hardware_status = HARDWARE_IDLE;
		}
		break;
	case DWEIOC_SET_LUT: {
		struct lut_info info;

		viv_check_retval(copy_from_user(&info, args, sizeof(info)));
		if (info.port < MAX_CFG_NUM)
			dev->dist_map[dwe->id][info.port] = info.addr;
		else
			pr_err("map num exceeds the max cfg num.\n");
		break;
	}
	case DWEIOC_GET_LUT_STATUS: {
		which = dev->which[dwe->id];
		if (dwe->state & STATE_DRIVER_STARTED) {
			if (dev->dist_map[dwe->id][which] ==
					dev->curmap[dwe->id][which]) {
				lut_status = 0;
			} else {
				lut_status = 1;
			}
		} else {
			lut_status = 0;
		}
		viv_check_retval(copy_to_user(args, &lut_status, sizeof(lut_status)));

		break;
	}
	case VIDIOC_QUERYCAP: {
		struct v4l2_capability *cap = (struct v4l2_capability *)args;

		strcpy((char *)cap->driver, "viv_dewarp100");
		cap->bus_info[0] = (__u8)dwe->id;
		break;
	}
	default:
		return dwe_priv_ioctl(&dwe->core->ic_dev, cmd, args);
	}
	return ret;
}

static int dwe_core_match(struct dwe_devcore *core, struct resource *res)
{
	return core && res && core->start == res->start &&
			core->end == res->end;
}

static int dwe_core_get_index(struct dwe_ic_dev *dev, struct vb2_dc_buf *buf)
{
	struct dwe_devcore *core =
			container_of(dev, struct dwe_devcore, ic_dev);
	int i;

	for (i = 0; i < MAX_DWE_NUM; ++i)
		if (core->src_pads[i] == buf->pad)
			return i;
	return -1;
}

struct dwe_devcore *dwe_devcore_init(struct dwe_device *dwe,
				struct resource *res)
{
	struct dwe_devcore *core, *found = NULL;
	unsigned long flags;
	int rc;

	spin_lock_irqsave(&devcore_list_lock, flags);
	if (!list_empty(&devcore_list)) {
		list_for_each_entry(core, &devcore_list, entry) {
			if (core->match && core->match(core, res)) {
				found = core;
				break;
			}
		}
	}
	spin_unlock_irqrestore(&devcore_list_lock, flags);

	if (found) {
		found->src_pads[dwe->id] = &dwe->pads[DWE_PAD_SINK];
		found->ic_dev.src_bctx[dwe->id] = &dwe->bctx[DWE_PAD_SOURCE];
		found->ic_dev.state[dwe->id] = &dwe->state;
		refcount_inc(&found->refcount);
		return found;
	}

	core = kzalloc(sizeof(struct dwe_devcore), GFP_KERNEL);

	if (!core)
		return NULL;

	core->ic_dev.base = devm_ioremap_resource(dwe->sd.dev, res);
	if (IS_ERR(core->ic_dev.base)) {
		pr_err("failed to get ioremap resource.\n");
		goto end;
	}
	core->start = res->start;
	core->end = res->end;

#ifdef DWE_REG_RESET
	core->ic_dev.reset = ioremap(DWE_REG_RESET, 4);
#endif
	pr_debug("dwe ioremap addr: %llx\n", (u64)core->ic_dev.base);

	vvbuf_ctx_init(&core->bctx[DWE_PAD_SINK]);
	core->ic_dev.sink_bctx = &core->bctx[DWE_PAD_SINK];
	core->irq = dwe->irq;
	pr_debug("request_irq num:%d, rc:%d\n", dwe->irq, rc);

	spin_lock_init(&core->ic_dev.irqlock);

	core->match = dwe_core_match;
	core->src_pads[dwe->id] = &dwe->pads[DWE_PAD_SINK];
	core->ic_dev.src_bctx[dwe->id] = &dwe->bctx[DWE_PAD_SOURCE];
	core->ic_dev.state[dwe->id] = &dwe->state;
	core->ic_dev.get_index = dwe_core_get_index;

	tasklet_init(&core->ic_dev.tasklet, dwe_isr_tasklet, (unsigned long)(&core->ic_dev));

	mutex_init(&core->mutex);
	refcount_set(&core->refcount, 1);

	spin_lock_irqsave(&devcore_list_lock, flags);
	list_add_tail(&core->entry, &devcore_list);
	spin_unlock_irqrestore(&devcore_list_lock, flags);

	return core;
end:
	kfree(core);
	return NULL;
}

void dwe_devcore_deinit(struct dwe_device *dwe)
{
	struct dwe_devcore *core = dwe->core;
	unsigned long flags;

	if (!core)
		return;

	if (refcount_dec_and_test(&core->refcount)) {
		tasklet_kill(&core->ic_dev.tasklet);
		spin_lock_irqsave(&devcore_list_lock, flags);
		list_del(&core->entry);
		spin_unlock_irqrestore(&devcore_list_lock, flags);
		vvbuf_ctx_deinit(&core->bctx[DWE_PAD_SINK]);

#ifdef DWE_REG_RESET
		iounmap(core->ic_dev.reset);
#endif
		mutex_destroy(&core->mutex);
		kfree(core);
	}
}
