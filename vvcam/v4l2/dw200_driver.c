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
#include "dw200_driver.h"
#include "dw200_ioctl.h"

#define DEVICE_NAME "vvcam-dw200"

int dw200_subscribe_event(struct v4l2_subdev *sd, struct v4l2_fh *fh,
			  struct v4l2_event_subscription *sub)
{
	return v4l2_event_subscribe(fh, sub, 2, NULL);
}

int dw200_unsubscribe_event(struct v4l2_subdev *sd, struct v4l2_fh *fh,
			    struct v4l2_event_subscription *sub)
{
	return v4l2_event_unsubscribe(fh, sub);
}

#ifdef CONFIG_COMPAT
static long dw200_ioctl_compat(struct v4l2_subdev *sd,
			       unsigned int cmd, void *arg)
{
	struct dw200_device *dw200_dev = v4l2_get_subdevdata(sd);

	return dw200_priv_ioctl(&dw200_dev->ic_dev, cmd, arg);
}

long dw200_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	return dw200_ioctl_compat(sd, cmd, arg);
}
#else /* CONFIG_COMPAT */
long dw200_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	struct dw200_device *dw200_dev = v4l2_get_subdevdata(sd);

	return dw200_priv_ioctl(&^dw200_dev->ic_dev, cmd, arg);
}
#endif /* CONFIG_COMPAT */

int dw200_set_stream(struct v4l2_subdev *sd, int enable)
{
	return 0;
}

static struct v4l2_subdev_core_ops dw200_v4l2_subdev_core_ops = {
	.ioctl = dw200_ioctl,
	.subscribe_event = dw200_subscribe_event,
	.unsubscribe_event = dw200_unsubscribe_event,
};

static struct v4l2_subdev_video_ops dw200_v4l2_subdev_video_ops = {
	.s_stream = dw200_set_stream,
};

static struct v4l2_subdev_ops dw200_v4l2_subdev_ops = {
	.core = &dw200_v4l2_subdev_core_ops,
	.video = &dw200_v4l2_subdev_video_ops,
};

int dw200_hw_probe(struct platform_device *pdev)
{
	struct dw200_device *dw200_dev;
	int rc = 0;

	pr_info("enter %s\n", __func__);
	dw200_dev = kzalloc(sizeof(struct dw200_device), GFP_KERNEL);
	if (!dw200_dev) {
		rc = -ENOMEM;
		goto end;
	}

	v4l2_subdev_init(&dw200_dev->sd, &dw200_v4l2_subdev_ops);
	snprintf(dw200_dev->sd.name, sizeof(dw200_dev->sd.name), DEVICE_NAME);
	dw200_dev->sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	dw200_dev->sd.flags |= V4L2_SUBDEV_FL_HAS_EVENTS;
	dw200_dev->sd.owner = THIS_MODULE;
	v4l2_set_subdevdata(&dw200_dev->sd, dw200_dev);
	dw200_dev->vd = kzalloc(sizeof(*dw200_dev->vd), GFP_KERNEL);
	if (WARN_ON(!dw200_dev->vd)) {
		rc = -ENOMEM;
		goto end;
	}

	rc = v4l2_device_register(&(pdev->dev), dw200_dev->vd);
	if (WARN_ON(rc < 0))
		goto end;

	rc = v4l2_device_register_subdev(dw200_dev->vd, &dw200_dev->sd);
	if (rc) {
		pr_err("failed to register subdev %d\n", rc);
		goto end;
	}
	dw200_dev->ic_dev.dwe_base = ioremap(DWE_REG_BASE, DWE_REG_SIZE);
	dw200_dev->ic_dev.vse_base = ioremap(VSE_REG_BASE, VSE_REG_SIZE);
#ifdef DWE_REG_RESET
	dw200_dev->ic_dev.dwe_reset = ioremap(DWE_REG_RESET, 4);
#endif
#ifdef VSE_REG_RESET
	dw200_dev->ic_dev.vse_reset = ioremap(VSE_REG_RESET, 4);
#endif
	pr_info("dw200 ioremap addr: 0x%08x 0x%08x %p", DWE_REG_BASE,
		DWE_REG_SIZE, dw200_dev->ic_dev.dwe_base);
	platform_set_drvdata(pdev, dw200_dev);
	rc = v4l2_device_register_subdev_nodes(dw200_dev->vd);
	return 0;
end:
	return rc;
}

int dw200_hw_remove(struct platform_device *pdev)
{
	struct dw200_device *dw200 = platform_get_drvdata(pdev);

	pr_info("enter %s\n", __func__);
	if (!dw200)
		return -EINVAL;

	v4l2_device_unregister_subdev(&dw200->sd);
	v4l2_device_disconnect(dw200->vd);
	v4l2_device_put(dw200->vd);

	iounmap(dw200->ic_dev.dwe_base);
	iounmap(dw200->ic_dev.vse_base);
#ifdef DWE_REG_RESET
	iounmap(dw200->ic_dev.dwe_reset);
#endif
#ifdef VSE_REG_RESET
	iounmap(dw200->ic_dev.vse_reset);
#endif
	kzfree(dw200);

	return 0;
}

static struct platform_driver viv_dw200_driver = {
	.probe = dw200_hw_probe,
	.remove = dw200_hw_remove,
	.driver = {
		   .name = DEVICE_NAME,
		   .owner = THIS_MODULE,
		   }
};

static void dw200_pdev_release(struct device *dev)
{
	pr_info("enter %s\n", __func__);
}

static struct platform_device viv_dw200_pdev = {
	.name = DEVICE_NAME,
	.dev.release = dw200_pdev_release,
};

static int __init viv_dw200_init_module(void)
{
	int ret = 0;

	pr_info("enter %s\n", __func__);
	ret = platform_device_register(&viv_dw200_pdev);
	if (ret) {
		pr_err("register platform device failed.\n");
		return ret;
	}

	ret = platform_driver_register(&viv_dw200_driver);
	if (ret) {
		pr_err("register platform driver failed.\n");
		platform_device_unregister(&viv_dw200_pdev);
		return ret;
	}
	return ret;
}

static void __exit viv_dw200_exit_module(void)
{
	pr_info("enter %s\n", __func__);
	platform_driver_unregister(&viv_dw200_driver);
	platform_device_unregister(&viv_dw200_pdev);
}

module_init(viv_dw200_init_module);
module_exit(viv_dw200_exit_module);

MODULE_AUTHOR("zhiye.yin@verisilicon.com");
MODULE_LICENSE("GPL");
MODULE_ALIAS("Dewarp200");
MODULE_VERSION("1.0");
