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

#include "vse_driver.h"
#include "vse_ioctl.h"
#define DEVICE_NAME "vvcam-vse"

int vse_subscribe_event(struct v4l2_subdev *sd, struct v4l2_fh *fh,
			struct v4l2_event_subscription *sub)
{
	return v4l2_event_subscribe(fh, sub, 2, NULL);
}

int vse_unsubscribe_event(struct v4l2_subdev *sd, struct v4l2_fh *fh,
			  struct v4l2_event_subscription *sub)
{
	return v4l2_event_unsubscribe(fh, sub);
}

#ifdef CONFIG_COMPAT
static long vse_ioctl_compat(struct v4l2_subdev *sd,
			     unsigned int cmd, void *arg)
{
	struct vse_device *vse_dev = v4l2_get_subdevdata(sd);

	return vse_priv_ioctl(&vse_dev->ic_dev, cmd, arg);
}

long vse_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	return vse_ioctl_compat(sd, cmd, arg);
}
#else /* CONFIG_COMPAT */
long vse_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	struct vse_device *vse_dev = v4l2_get_subdevdata(sd);

	return vse_priv_ioctl(&^vse_dev->ic_dev, cmd, arg);
}
#endif /* CONFIG_COMPAT */

int vse_set_stream(struct v4l2_subdev *sd, int enable)
{
	return 0;
}

static struct v4l2_subdev_core_ops vse_v4l2_subdev_core_ops = {
	.ioctl = vse_ioctl,
	.subscribe_event = vse_subscribe_event,
	.unsubscribe_event = vse_unsubscribe_event,
};

static struct v4l2_subdev_video_ops vse_v4l2_subdev_video_ops = {
	.s_stream = vse_set_stream,
};

static struct v4l2_subdev_ops vse_v4l2_subdev_ops = {
	.core = &vse_v4l2_subdev_core_ops,
	.video = &vse_v4l2_subdev_video_ops,
};

int vse_hw_probe(struct platform_device *pdev)
{
	struct vse_device *vse_dev;
	int rc = 0;

	pr_info("enter %s\n", __func__);
	vse_dev = kzalloc(sizeof(struct vse_device), GFP_KERNEL);
	if (!vse_dev) {
		rc = -ENOMEM;
		goto end;
	}

	v4l2_subdev_init(&vse_dev->sd, &vse_v4l2_subdev_ops);

	snprintf(vse_dev->sd.name, sizeof(vse_dev->sd.name), DEVICE_NAME);
	vse_dev->sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	vse_dev->sd.flags |= V4L2_SUBDEV_FL_HAS_EVENTS;
	vse_dev->sd.owner = THIS_MODULE;
	v4l2_set_subdevdata(&vse_dev->sd, vse_dev);
	vse_dev->vd = kzalloc(sizeof(*vse_dev->vd), GFP_KERNEL);
	if (WARN_ON(!vse_dev->vd)) {
		rc = -ENOMEM;
		goto end;
	}

	rc = v4l2_device_register(&(pdev->dev), vse_dev->vd);
	if (WARN_ON(rc < 0))
		goto end;

	rc = v4l2_device_register_subdev(vse_dev->vd, &vse_dev->sd);
	if (rc) {
		pr_err("failed to register subdev %d\n", rc);
		goto end;
	}
	vse_dev->ic_dev.base = ioremap(VSE_REG_BASE, VSE_REG_SIZE);
#ifdef VSE_REG_RESET
	vse_dev->ic_dev.reset = ioremap(VSE_REG_RESET, 4);
#endif
	pr_info("vse ioremap addr: 0x%08x 0x%08x %p", VSE_REG_BASE,
		VSE_REG_SIZE, vse_dev->ic_dev.base);
	platform_set_drvdata(pdev, vse_dev);
	rc = v4l2_device_register_subdev_nodes(vse_dev->vd);
	return rc;
end:
	return rc;
}

int vse_hw_remove(struct platform_device *pdev)
{
	struct vse_device *vse = platform_get_drvdata(pdev);

	pr_info("enter %s\n", __func__);

	if (!vse)
		return -1;

	v4l2_device_unregister_subdev(&vse->sd);
	v4l2_device_disconnect(vse->vd);
	v4l2_device_put(vse->vd);

	iounmap(vse->ic_dev.base);
	kzfree(vse);
	return 0;
}

static struct platform_driver viv_vse_driver = {
	.probe		= vse_hw_probe,
	.remove		= vse_hw_remove,
	.driver = {
		.name  = DEVICE_NAME,
		.owner = THIS_MODULE,
	}
};

static void vse_pdev_release(struct device *dev)
{
	pr_info("enter %s\n", __func__);
}

static struct platform_device viv_vse_pdev = {
	.name = DEVICE_NAME,
	.dev.release = vse_pdev_release,
};


static int __init viv_vse_init_module(void)
{
	int ret = 0;

	pr_info("enter %s\n", __func__);
	ret = platform_device_register(&viv_vse_pdev);
	if (ret) {
		pr_err("register platform device failed.\n");
		return ret;
	}

	ret = platform_driver_register(&viv_vse_driver);
	if (ret) {
		pr_err("register platform driver failed.\n");
		platform_device_unregister(&viv_vse_pdev);
		return ret;
	}
	return ret;
}

static void __exit viv_vse_exit_module(void)
{
	pr_info("enter %s\n", __func__);
	platform_driver_unregister(&viv_vse_driver);
	platform_device_unregister(&viv_vse_pdev);
}

module_init(viv_vse_init_module);
module_exit(viv_vse_exit_module);

MODULE_AUTHOR("zhiye.yin@verisilicon.com");
MODULE_LICENSE("GPL");
MODULE_ALIAS("VSE");
MODULE_VERSION("1.0");
