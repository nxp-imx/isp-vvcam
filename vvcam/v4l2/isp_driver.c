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
#include <linux/clk/clk-conf.h>
#include <linux/dma-mapping.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/debugfs.h>

#include <linux/videodev2.h>
#include <linux/pm_runtime.h>

#include <linux/of_device.h>
#include <linux/sched_clock.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-device.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-event.h>
#include <media/media-entity.h>
#include <uapi/linux/media-bus-format.h>

#include "isp_driver.h"
#include "isp_ioctl.h"
#include "mrv_all_bits.h"
#include "viv_video_kevent.h"

struct clk *clk_isp;

extern MrvAllRegister_t *all_regs;

#ifdef CONFIG_COMPAT
static long isp_ioctl_compat(struct v4l2_subdev *sd,
			     unsigned int cmd, void *arg)
{
	struct isp_device *isp_dev = v4l2_get_subdevdata(sd);

	return isp_priv_ioctl(&isp_dev->ic_dev, cmd, arg);
}

long isp_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	return isp_ioctl_compat(sd, cmd, arg);
}
#else /* CONFIG_COMPAT */
long isp_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	struct isp_device *isp_dev = v4l2_get_subdevdata(sd);

	return isp_priv_ioctl(&isp_dev->ic_dev, cmd, arg);
}
#endif /* CONFIG_COMPAT */

int isp_set_stream(struct v4l2_subdev *sd, int enable)
{
	return 0;
}

static void isp_post_event(struct isp_ic_dev *dev, void *data, size_t size)
{
	struct isp_device *isp_dev;
	struct video_device *vdev;
	struct v4l2_event event;

	if (!dev || !data || !size)
		return;

	isp_dev = container_of(dev, struct isp_device, ic_dev);
	vdev = isp_dev->sd.devnode;
	if (!vdev)
		return;

	memset(&event, 0, sizeof(event));
	memcpy(event.u.data, data, min_t(size_t, size, 64));
	event.type = VIV_VIDEO_ISPIRQ_TYPE;
	v4l2_event_queue(vdev, &event);
}

static int isp_subdev_subscribe_event(struct v4l2_subdev *sd,
		    struct v4l2_fh *fh, struct v4l2_event_subscription *sub)
{
	struct isp_device *isp_dev = v4l2_get_subdevdata(sd);

	if (sub->type != VIV_VIDEO_ISPIRQ_TYPE)
		return -EINVAL;

	if (!isp_dev->ic_dev.post_event)
		isp_dev->ic_dev.post_event = isp_post_event;

	return v4l2_event_subscribe(fh, sub, 8, NULL);
}

static int isp_subdev_unsubscribe_event(struct v4l2_subdev *sd,
		    struct v4l2_fh *fh, struct v4l2_event_subscription *sub)
{
	struct isp_device *isp_dev = v4l2_get_subdevdata(sd);

	if (sub->type != VIV_VIDEO_ISPIRQ_TYPE)
		return -EINVAL;

	if (isp_dev->ic_dev.post_event)
		isp_dev->ic_dev.post_event = NULL;

	return v4l2_event_unsubscribe(fh, sub);
}

static struct v4l2_subdev_core_ops isp_v4l2_subdev_core_ops = {
	.ioctl = isp_ioctl,
	.subscribe_event = isp_subdev_subscribe_event,
	.unsubscribe_event = isp_subdev_unsubscribe_event,
};

static struct v4l2_subdev_video_ops isp_v4l2_subdev_video_ops = {
	.s_stream = isp_set_stream,
};


irqreturn_t isp_hw_isr_reg_update(int irq, void *data)
{
	u32 isp_mis;

	struct isp_irq_data irq_data;
	struct isp_ic_dev *dev = (struct isp_ic_dev *)data;
	
	if (!dev)
		return IRQ_HANDLED;

	isp_mis = isp_read_reg(dev, REG_ADDR(isp_mis));
	isp_write_reg(dev, REG_ADDR(isp_icr), isp_mis);


	if (isp_mis) {
		if (isp_mis & MRV_ISP_MIS_FRAME_MASK) {
			if (dev->isp_update_flag & ISP_FLT_UPDATE) {
				isp_s_flt(dev);
				dev->isp_update_flag &= (~ISP_FLT_UPDATE);
			}
		}

		memset(&irq_data, 0, sizeof(irq_data));
		irq_data.val = isp_mis;
		if (dev->post_event)
			dev->post_event(dev, &irq_data, sizeof(irq_data));
	}
	return IRQ_HANDLED;
}

struct v4l2_subdev_ops isp_v4l2_subdev_ops = {
	.core = &isp_v4l2_subdev_core_ops,
	.video = &isp_v4l2_subdev_video_ops,
};

int isp_hw_probe(struct platform_device *pdev)
{
	struct isp_device *isp_dev;
	struct resource *mem_res;
	int irq;
	int rc;

	pr_info("enter %s\n", __func__);
	isp_dev = kzalloc(sizeof(struct isp_device), GFP_KERNEL);
	if (!isp_dev) {
		goto end;
	}

	rc = fwnode_property_read_u32(of_fwnode_handle(pdev->dev.of_node),
			"id", &isp_dev->id);
	if (rc) {
		pr_info("isp device id not found, use the default.\n");
		isp_dev->id = 0;
	}
	isp_dev->ic_dev.id = isp_dev->id;

	v4l2_subdev_init(&isp_dev->sd, &isp_v4l2_subdev_ops);
	snprintf(isp_dev->sd.name, sizeof(isp_dev->sd.name), 
				"vvcam-isp.%d", isp_dev->ic_dev.id);
	isp_dev->sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	isp_dev->sd.flags |= V4L2_SUBDEV_FL_HAS_EVENTS;
	isp_dev->sd.owner = THIS_MODULE;
	v4l2_set_subdevdata(&isp_dev->sd, isp_dev);
	pm_runtime_enable(&pdev->dev);
	pm_runtime_get_sync(&pdev->dev);
	isp_dev->vd = kzalloc(sizeof(*isp_dev->vd), GFP_KERNEL);
	if (WARN_ON(!isp_dev->vd)) {
		rc = -ENOMEM;
		goto end;
	}

	rc = v4l2_device_register(&(pdev->dev), isp_dev->vd);
	if (WARN_ON(rc < 0))
		goto end;

	rc = v4l2_device_register_subdev(isp_dev->vd, &isp_dev->sd);
	if (rc) {
		pr_err("failed to register subdev %d\n", rc);
		goto end;
	}

	mem_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	isp_dev->ic_dev.base = devm_ioremap_resource(&pdev->dev, mem_res);
	if (IS_ERR(isp_dev->ic_dev.base)) {
		pr_err("failed to get ioremap resource.\n");
		goto end;
	}

#ifdef ISP_REG_RESET
	isp_dev->ic_dev.reset = ioremap(ISP_REG_RESET, 4);
#endif
	pr_debug("ioremap addr: %p", isp_dev->ic_dev.base);

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		pr_err("failed to get irq number.\n");
		goto end;
	}

	rc = devm_request_irq(&pdev->dev, irq, isp_hw_isr_reg_update, IRQF_SHARED,
			dev_name(&pdev->dev), &isp_dev->ic_dev);
	if (rc) {
		pr_err("failed to request irq.\n");
		irq = -1;
		goto end;
	}
	isp_dev->irq = irq;
	pr_debug("request_irq num:%d, rc:%d", irq, rc);

	platform_set_drvdata(pdev, isp_dev);
	rc = v4l2_device_register_subdev_nodes(isp_dev->vd);
	pr_info("vvcam isp driver registered\n");
	return 0;
end:
	if (irq >= 0)
		devm_free_irq(&pdev->dev, irq, &isp_dev->ic_dev);
	kfree(isp_dev);
	return 1;
}

int isp_hw_remove(struct platform_device *pdev)
{
	struct isp_device *isp = platform_get_drvdata(pdev);

	pr_info("enter %s\n", __func__);
	if (!isp)
		return -1;
	devm_free_irq(&pdev->dev, isp->irq, &isp->ic_dev);
	v4l2_device_unregister_subdev(&isp->sd);
	if (isp->vd) {
		v4l2_device_disconnect(isp->vd);
		v4l2_device_put(isp->vd);
	}

	kfree(isp);
	pm_runtime_put(&pdev->dev);
	pm_runtime_disable(&pdev->dev);
	return 0;
}

static int isp_system_suspend(struct device *dev)
{
	return pm_runtime_force_suspend(dev);;
}

static int isp_system_resume(struct device *dev)
{
	int ret;

	ret = pm_runtime_force_resume(dev);
	if (ret < 0) {
		dev_err(dev, "force resume %s failed!\n", dev_name(dev));
		return ret;
	}

	return 0;
}

static const struct dev_pm_ops isp_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(isp_system_suspend, isp_system_resume)
};

static const struct of_device_id isp_of_match[] = {
	{.compatible = "fsl,imx8mp-isp",},
	{ /* sentinel */ },
};

MODULE_DEVICE_TABLE(of, isp_of_match);

static struct platform_driver viv_isp_driver = {
	.probe = isp_hw_probe,
	.remove = isp_hw_remove,
	.driver = {
		   .name = "vvcam-isp",
		   .owner = THIS_MODULE,
		   .of_match_table = isp_of_match,
		   .pm = &isp_pm_ops,
		   }
};

static int __init viv_isp_init_module(void)
{
	int ret = 0;

	pr_info("enter %s\n", __func__);

	ret = platform_driver_register(&viv_isp_driver);
	if (ret) {
		pr_err("register platform driver failed.\n");
		return ret;
	}

	return ret;
}

static void __exit viv_isp_exit_module(void)
{
	pr_info("enter %s\n", __func__);
	platform_driver_unregister(&viv_isp_driver);
}

module_init(viv_isp_init_module);
module_exit(viv_isp_exit_module);

MODULE_AUTHOR("Verisilicon ISP SW Team");
MODULE_LICENSE("GPL");
MODULE_ALIAS("Verisilicon-ISP");
MODULE_VERSION("1.0");
