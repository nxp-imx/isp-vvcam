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
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/debugfs.h>

#include <linux/videodev2.h>
#include <linux/of_device.h>
#include <linux/sched_clock.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-device.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-event.h>
#include <media/media-entity.h>
#include <uapi/linux/media-bus-format.h>
#include "video.h"
#include "dwe_driver.h"
#include "dwe_ioctl.h"

int dwe_open_node(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	struct dwe_device *dwe_dev = v4l2_get_subdevdata(sd);

	pr_debug("%s E open_cnt %u\n", __func__, ++dwe_dev->dwe_open_cnt);
	return 0;
}

int dwe_close_node(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	struct dwe_device *dwe_dev = v4l2_get_subdevdata(sd);

	pr_debug("%s E open_cnt %u\n", __func__, --dwe_dev->dwe_open_cnt);
	return 0;
}

int dwe_subscribe_event(struct v4l2_subdev *sd, struct v4l2_fh *fh,
			struct v4l2_event_subscription *sub)
{
	return v4l2_event_subscribe(fh, sub, 2, NULL);
}

int dwe_unsubscribe_event(struct v4l2_subdev *sd, struct v4l2_fh *fh,
			  struct v4l2_event_subscription *sub)
{
	return v4l2_event_unsubscribe(fh, sub);
}

#ifdef CONFIG_COMPAT
static long dwe_ioctl_compat(struct v4l2_subdev *sd,
			     unsigned int cmd, void *arg)
{
	struct dwe_device *dwe_dev = v4l2_get_subdevdata(sd);

	return dwe_priv_ioctl(&dwe_dev->ic_dev, cmd, arg);
}

long dwe_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	return dwe_ioctl_compat(sd, cmd, arg);
}
#else /* CONFIG_COMPAT */
long dwe_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	struct dwe_device *dwe_dev = v4l2_get_subdevdata(sd);

	return dwe_priv_ioctl(&dwe_dev->ic_dev, cmd, arg);
}
#endif /* CONFIG_COMPAT */

int dwe_set_stream(struct v4l2_subdev *sd, int enable)
{
	return 0;
}

static struct v4l2_subdev_core_ops dwe_v4l2_subdev_core_ops = {
	.ioctl = dwe_ioctl,
	.subscribe_event = dwe_subscribe_event,
	.unsubscribe_event = dwe_unsubscribe_event,
};

static struct v4l2_subdev_video_ops dwe_v4l2_subdev_video_ops = {
	.s_stream = dwe_set_stream,
};

static struct v4l2_subdev_ops dwe_v4l2_subdev_ops = {
	.core = &dwe_v4l2_subdev_core_ops,
	.video = &dwe_v4l2_subdev_video_ops,
};

static struct v4l2_subdev_internal_ops dwe_subdev_internal_ops = {
	.open = dwe_open_node,
	.close = dwe_close_node,
};

void dwe_write_reg(struct dwe_ic_dev *dev, u32 offset, u32 val)
{
	__raw_writel(val, dev->base + offset);
}

u32 dwe_read_reg(struct dwe_ic_dev *dev, u32 offset)
{
	return __raw_readl(dev->base + offset);
}

int dwe_hw_register(struct viv_video_device *vdev)
{
	struct dwe_device *dwe_dev;
	int rc = 0;

	pr_debug("enter %s\n", __func__);
	dwe_dev = kzalloc(sizeof(struct dwe_device), GFP_KERNEL);
	if (!dwe_dev) {
		rc = -ENOMEM;
		goto end;
	}

	v4l2_subdev_init(&dwe_dev->subdev, &dwe_v4l2_subdev_ops);
	dwe_dev->subdev.internal_ops = &dwe_subdev_internal_ops;
	snprintf(dwe_dev->subdev.name, sizeof(dwe_dev->subdev.name), "viv_dwe");
	dwe_dev->subdev.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	dwe_dev->subdev.flags |= V4L2_SUBDEV_FL_HAS_EVENTS;
	dwe_dev->subdev.owner = THIS_MODULE;
	v4l2_set_subdevdata(&dwe_dev->subdev, dwe_dev);
	rc = v4l2_device_register_subdev(vdev->v4l2_dev, &dwe_dev->subdev);
	dwe_dev->dwe_open_cnt = 0;
	dwe_dev->ic_dev.base = ioremap(DWE_REG_BASE, DWE_REG_SIZE);
#ifdef DWE_REG_RESET
	dwe_dev->ic_dev.reset = ioremap(DWE_REG_RESET, 4);
#endif
	pr_debug("dwe ioremap addr: 0x%08x 0x%08x %p", DWE_REG_BASE,
		DWE_REG_SIZE, dwe_dev->ic_dev.base);
	vdev->dwe = dwe_dev;
	pr_info("vvcam dewarp driver registered\n");
	return rc;
end:
	return rc;
}

int dwe_hw_unregister(struct viv_video_device *vdev)
{
	struct v4l2_subdev *sd = NULL;

	pr_debug("enter %s\n", __func__);
	if (!vdev)
		return -1;
	if (!vdev->dwe)
		return -1;
	sd = &vdev->dwe->subdev;
	v4l2_device_unregister_subdev(sd);
	iounmap(vdev->dwe->ic_dev.base);
	kzfree(vdev->dwe);
	vdev->dwe = NULL;
	pr_info("vvcam dewarp driver unregistered\n");
	return 0;
}
