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
#include <linux/of_device.h>
#include <linux/sched_clock.h>

#include <linux/videodev2.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-device.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-event.h>
#include <media/media-entity.h>
#include <uapi/linux/media-bus-format.h>
#include "dwe_driver.h"
#include "dwe_ioctl.h"
#define DEVICE_NAME "vvcam-dwe"

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

int dwe_hw_probe(struct platform_device *pdev)
{
	struct dwe_device *dwe_dev;
	int rc = 0;

	pr_info("enter %s\n", __func__);
	dwe_dev = kzalloc(sizeof(struct dwe_device), GFP_KERNEL);
	if (!dwe_dev) {
		rc = -ENOMEM;
		goto end;
	}

	v4l2_subdev_init(&dwe_dev->sd, &dwe_v4l2_subdev_ops);
	snprintf(dwe_dev->sd.name, sizeof(dwe_dev->sd.name), DEVICE_NAME);
	dwe_dev->sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	dwe_dev->sd.flags |= V4L2_SUBDEV_FL_HAS_EVENTS;
	dwe_dev->sd.owner = THIS_MODULE;
	v4l2_set_subdevdata(&dwe_dev->sd, dwe_dev);
	dwe_dev->vd = kzalloc(sizeof(*dwe_dev->vd), GFP_KERNEL);
	if (WARN_ON(!dwe_dev->vd)) {
		rc = -ENOMEM;
		goto end;
	}

	rc = v4l2_device_register(&(pdev->dev), dwe_dev->vd);
	if (WARN_ON(rc < 0))
		goto end;

	rc = v4l2_device_register_subdev(dwe_dev->vd, &dwe_dev->sd);
	if (rc) {
		pr_err("failed to register subdev %d\n", rc);
		goto end;
	}

	dwe_dev->ic_dev.base = ioremap(DWE_REG_BASE, DWE_REG_SIZE);
#ifdef DWE_REG_RESET
	dwe_dev->ic_dev.reset = ioremap(DWE_REG_RESET, 4);
#endif
	pr_info("dwe ioremap addr: 0x%08x 0x%08x %p", DWE_REG_BASE,
		DWE_REG_SIZE, dwe_dev->ic_dev.base);
	platform_set_drvdata(pdev, dwe_dev);
	rc = v4l2_device_register_subdev_nodes(dwe_dev->vd);
	pr_info("vvcam dewarp driver probed\n");
	return 0;
end:
	return rc;
}

int dwe_hw_remove(struct platform_device *pdev)
{
	struct dwe_device *dwe = platform_get_drvdata(pdev);

	pr_info("enter %s\n", __func__);

	if (!dwe)
		return -1;
	v4l2_device_unregister_subdev(&dwe->sd);
	v4l2_device_disconnect(dwe->vd);
	v4l2_device_put(dwe->vd);

	iounmap(dwe->ic_dev.base);
#ifdef DWE_REG_RESET
	iounmap(dwe->ic_dev.reset);
#endif
	kfree(dwe);
	pr_info("vvcam dewarp driver removed\n");
	return 0;
}

static struct platform_driver viv_dwe_driver = {
	.probe = dwe_hw_probe,
	.remove = dwe_hw_remove,
	.driver = {
		   .name = DEVICE_NAME,
		   .owner = THIS_MODULE,
		   }
};

static void dwe_pdev_release(struct device *dev)
{
	pr_info("enter %s\n", __func__);
}

static struct platform_device viv_dwe_pdev = {
	.name = DEVICE_NAME,
	.dev.release = dwe_pdev_release,
};

static int __init viv_dwe_init_module(void)
{
	int ret = 0;

	pr_info("enter %s\n", __func__);
	ret = platform_device_register(&viv_dwe_pdev);
	if (ret) {
		pr_err("register platform device failed.\n");
		return ret;
	}

	ret = platform_driver_register(&viv_dwe_driver);
	if (ret) {
		pr_err("register platform driver failed.\n");
		platform_device_unregister(&viv_dwe_pdev);
		return ret;
	}
	return ret;
}

static void __exit viv_dwe_exit_module(void)
{
	pr_info("enter %s\n", __func__);
	platform_driver_unregister(&viv_dwe_driver);
	platform_device_unregister(&viv_dwe_pdev);
}

module_init(viv_dwe_init_module);
module_exit(viv_dwe_exit_module);

MODULE_AUTHOR("zhiye.yin@verisilicon.com");
MODULE_LICENSE("GPL");
MODULE_ALIAS("Dewarp");
MODULE_VERSION("1.0");
