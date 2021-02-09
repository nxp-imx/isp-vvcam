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
#include <linux/pm_runtime.h>
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

static int dwe_enable_clocks(struct dwe_device *dwe_dev)
{
	int ret;

	ret = clk_prepare_enable(dwe_dev->clk_core);
	if (ret)
		return ret;

	ret = clk_prepare_enable(dwe_dev->clk_axi);
	if (ret)
		goto disable_clk_core;

	ret = clk_prepare_enable(dwe_dev->clk_ahb);
	if (ret)
		goto disable_clk_axi;

	return 0;

disable_clk_axi:
	clk_disable_unprepare(dwe_dev->clk_axi);
disable_clk_core:
	clk_disable_unprepare(dwe_dev->clk_core);

	return ret;
}

static void dwe_disable_clocks(struct dwe_device *dwe_dev)
{
	clk_disable_unprepare(dwe_dev->clk_ahb);
	clk_disable_unprepare(dwe_dev->clk_axi);
	clk_disable_unprepare(dwe_dev->clk_core);
}


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

static int dwe_open(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	pm_runtime_get_sync(sd->dev);
	return 0;
}

static int dwe_close(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	pm_runtime_put_sync(sd->dev);
	return 0;
}

static struct v4l2_subdev_internal_ops dwe_internal_ops = {
	.open = dwe_open,
	.close = dwe_close,
};

int dwe_hw_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct dwe_device *dwe_dev;
	struct resource *mem_res;
	int rc = 0;

	pr_info("enter %s\n", __func__);
	dwe_dev = kzalloc(sizeof(struct dwe_device), GFP_KERNEL);
	if (!dwe_dev) {
		rc = -ENOMEM;
		goto end;
	}

	dwe_dev->clk_core = devm_clk_get(dev, "core");
	if (IS_ERR(dwe_dev->clk_core)) {
		rc = PTR_ERR(dwe_dev->clk_core);
		dev_err(dev, "can't get core clock: %d\n", rc);
		return rc;
	}

	dwe_dev->clk_axi = devm_clk_get(dev, "axi");
	if (IS_ERR(dwe_dev->clk_axi)) {
		rc = PTR_ERR(dwe_dev->clk_axi);
		dev_err(dev, "can't get axi clock: %d\n", rc);
		return rc;
	}

	dwe_dev->clk_ahb = devm_clk_get(dev, "ahb");
	if (IS_ERR(dwe_dev->clk_ahb)) {
		rc = PTR_ERR(dwe_dev->clk_ahb);
		dev_err(dev, "can't get ahb clock: %d\n", rc);
		return rc;
	}

	dwe_dev->sd.internal_ops = &dwe_internal_ops;

	v4l2_subdev_init(&dwe_dev->sd, &dwe_v4l2_subdev_ops);
	snprintf(dwe_dev->sd.name, sizeof(dwe_dev->sd.name), DEVICE_NAME);
	dwe_dev->sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	dwe_dev->sd.flags |= V4L2_SUBDEV_FL_HAS_EVENTS;
	dwe_dev->sd.owner = THIS_MODULE;
	dwe_dev->sd.dev = &pdev->dev;
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

	mem_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	dwe_dev->ic_dev.base = devm_ioremap_resource(&pdev->dev, mem_res);
	if (IS_ERR(dwe_dev->ic_dev.base)) {
		pr_err("failed to get ioremap resource.\n");
		goto end;
	}

#ifdef DWE_REG_RESET
	dwe_dev->ic_dev.reset = ioremap(DWE_REG_RESET, 4);
#endif
	platform_set_drvdata(pdev, dwe_dev);
	rc = v4l2_device_register_subdev_nodes(dwe_dev->vd);
	pm_runtime_enable(&pdev->dev);
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
	pm_runtime_disable(&pdev->dev);
	v4l2_device_unregister_subdev(&dwe->sd);
	v4l2_device_disconnect(dwe->vd);
	v4l2_device_put(dwe->vd);

#ifdef DWE_REG_RESET
	iounmap(dwe->ic_dev.reset);
#endif
	kfree(dwe);
	pr_info("vvcam dewarp driver removed\n");
	return 0;
}

static int dwe_system_suspend(struct device *dev)
{
	return pm_runtime_force_suspend(dev);
}

static int dwe_system_resume(struct device *dev)
{
	int ret;

	ret = pm_runtime_force_resume(dev);
	if (ret < 0) {
		dev_err(dev, "force resume %s failed!\n", dev_name(dev));
		return ret;
	}

	return 0;
}

static int dwe_runtime_suspend(struct device *dev)
{
	struct dwe_device *dwe_dev = dev_get_drvdata(dev);

	dwe_disable_clocks(dwe_dev);

	return 0;
}

static int dwe_runtime_resume(struct device *dev)
{
	struct dwe_device *dwe_dev = dev_get_drvdata(dev);

	dwe_enable_clocks(dwe_dev);

	return 0;
}

static const struct dev_pm_ops dwe_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(dwe_system_suspend, dwe_system_resume)
	SET_RUNTIME_PM_OPS(dwe_runtime_suspend, dwe_runtime_resume, NULL)
};

static const struct of_device_id dwe_of_match[] = {
	{.compatible = DWE_COMPAT_NAME,},
	{ /* sentinel */ },
};

MODULE_DEVICE_TABLE(of, dwe_of_match);

static struct platform_driver viv_dwe_driver = {
	.probe = dwe_hw_probe,
	.remove = dwe_hw_remove,
	.driver = {
		   .name = DWE_DEVICE_NAME,
		   .owner = THIS_MODULE,
		   .of_match_table = dwe_of_match,
		   .pm = &dwe_pm_ops,
	}
};

static int __init viv_dwe_init_module(void)
{
	int ret = 0;

	pr_info("enter %s\n", __func__);
	ret = platform_driver_register(&viv_dwe_driver);
	if (ret)
		pr_err("register platform driver failed.\n");

	return ret;
}

static void __exit viv_dwe_exit_module(void)
{
	pr_info("enter %s\n", __func__);
	platform_driver_unregister(&viv_dwe_driver);
}

module_init(viv_dwe_init_module);
module_exit(viv_dwe_exit_module);

MODULE_AUTHOR("zhiye.yin@verisilicon.com");
MODULE_LICENSE("GPL");
MODULE_ALIAS("Dewarp");
MODULE_VERSION("1.0");
