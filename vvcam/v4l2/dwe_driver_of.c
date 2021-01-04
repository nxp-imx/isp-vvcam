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
#include <linux/pm_runtime.h>
#include <media/v4l2-event.h>

#include "dwe_driver.h"
#include "dwe_ioctl.h"

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
	return dwe_devcore_ioctl(v4l2_get_subdevdata(sd), cmd, arg);
}

long dwe_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	return dwe_ioctl_compat(sd, cmd, arg);
}
#else /* CONFIG_COMPAT */
long dwe_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	return dwe_devcore_ioctl(v4l2_get_subdevdata(sd), cmd, arg);
}
#endif /* CONFIG_COMPAT */

int dwe_set_stream(struct v4l2_subdev *sd, int enable)
{
	struct dwe_device *dwe_dev = v4l2_get_subdevdata(sd);
	struct vvbuf_ctx *ctx;
	struct media_pad *pad;
	unsigned long flags;
	int state;

	if (!enable)
		dwe_dev->state &= ~STATE_STREAM_STARTED;
	else
		dwe_dev->state |= STATE_STREAM_STARTED;

	pad = &dwe_dev->pads[DWE_PAD_SINK];
	pad = media_entity_remote_pad(pad);

	if (pad && is_media_entity_v4l2_subdev(pad->entity)) {
		sd = media_entity_to_v4l2_subdev(pad->entity);
		v4l2_subdev_call(sd, video, s_stream, enable);
	}

	if (!enable) {
		mutex_lock(&dwe_dev->core->mutex);
		state = dwe_dev->core->state;
		mutex_unlock(&dwe_dev->core->mutex);
		if (state <= 0) {
			ctx = &dwe_dev->core->bctx[DWE_PAD_SINK];
			spin_lock_irqsave(&ctx->irqlock, flags);
			if (!list_empty(&ctx->dmaqueue))
				list_del_init(&ctx->dmaqueue);
			spin_unlock_irqrestore(&ctx->irqlock, flags);
		}

		ctx = &dwe_dev->bctx[DWE_PAD_SOURCE];
		spin_lock_irqsave(&ctx->irqlock, flags);
		if (!list_empty(&ctx->dmaqueue))
			list_del_init(&ctx->dmaqueue);
		spin_unlock_irqrestore(&ctx->irqlock, flags);
	}
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

static int dwe_link_setup(struct media_entity *entity,
				const struct media_pad *local,
				const struct media_pad *remote, u32 flags)
{
	return 0;
}

static const struct media_entity_operations dwe_media_ops = {
	.link_setup = dwe_link_setup,
	.link_validate = v4l2_subdev_link_validate,
};

static void dwe_src_buf_notify(struct vvbuf_ctx *ctx, struct vb2_dc_buf *buf)
{
	struct v4l2_subdev *sd;
	struct dwe_device *dwe;
	unsigned long flags;

	if (unlikely(!ctx || !buf))
		return;

	sd = media_entity_to_v4l2_subdev(buf->pad->entity);
	dwe = container_of(sd, struct dwe_device, sd);

	if (!dwe || !(dwe->state & STATE_STREAM_STARTED)) {
		vvbuf_ready(ctx, buf->pad, buf);
		return;
	}

	ctx = &dwe->core->bctx[DWE_PAD_SINK];

	spin_lock_irqsave(&ctx->irqlock, flags);
	list_add_tail(&buf->irqlist, &ctx->dmaqueue);
	spin_unlock_irqrestore(&ctx->irqlock, flags);

	dwe_on_buf_update(&dwe->core->ic_dev);
}

static const struct vvbuf_ops dwe_src_buf_ops = {
	.notify = dwe_src_buf_notify,
};

static void dwe_dst_buf_notify(struct vvbuf_ctx *ctx, struct vb2_dc_buf *buf)
{
	struct v4l2_subdev *sd;
	struct dwe_device *dwe;
	unsigned long flags;

	if (unlikely(!ctx || !buf))
		return;

	sd = media_entity_to_v4l2_subdev(buf->pad->entity);
	dwe = container_of(sd, struct dwe_device, sd);

	spin_lock_irqsave(&ctx->irqlock, flags);
	list_add_tail(&buf->irqlist, &ctx->dmaqueue);
	spin_unlock_irqrestore(&ctx->irqlock, flags);

	if (dwe)
		dwe_on_buf_update(&dwe->core->ic_dev);
}

static const struct vvbuf_ops dwe_dst_buf_ops = {
	.notify = dwe_dst_buf_notify,
};

int dwe_hw_probe(struct platform_device *pdev)
{
	struct dwe_device *dwe_dev;
	struct resource *mem_res;
	int i, irq;
	int rc;

	pr_info("enter %s\n", __func__);
	dwe_dev = kzalloc(sizeof(struct dwe_device), GFP_KERNEL);
	if (!dwe_dev)
		return -ENOMEM;

	rc = fwnode_property_read_u32(of_fwnode_handle(pdev->dev.of_node),
			"id", &dwe_dev->id);
	if (rc) {
		pr_info("dwe device id not found, use the default.\n");
		dwe_dev->id = 0;
	}

	v4l2_subdev_init(&dwe_dev->sd, &dwe_v4l2_subdev_ops);
	snprintf(dwe_dev->sd.name, sizeof(dwe_dev->sd.name),
			"%s.%d", DWE_DEVICE_NAME, dwe_dev->id);
	dwe_dev->sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	dwe_dev->sd.flags |= V4L2_SUBDEV_FL_HAS_EVENTS;
	dwe_dev->sd.owner = THIS_MODULE;
	v4l2_set_subdevdata(&dwe_dev->sd, dwe_dev);
	dwe_dev->sd.dev = &pdev->dev;

	mem_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);

	for (i = 0; i < DWE_PADS_NUM; ++i)
		vvbuf_ctx_init(&dwe_dev->bctx[i]);
	dwe_dev->bctx[DWE_PAD_SINK].ops = &dwe_src_buf_ops;
	dwe_dev->bctx[DWE_PAD_SOURCE].ops = &dwe_dst_buf_ops;

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		pr_err("failed to get irq number.\n");
		goto end;
	}
	dwe_dev->irq = irq;

	platform_set_drvdata(pdev, dwe_dev);

	dwe_dev->sd.entity.name = dwe_dev->sd.name;
	dwe_dev->sd.entity.obj_type = MEDIA_ENTITY_TYPE_V4L2_SUBDEV;
	dwe_dev->sd.entity.function = MEDIA_ENT_F_IO_V4L;
	dwe_dev->sd.entity.ops = &dwe_media_ops;
	dwe_dev->pads[DWE_PAD_SINK].flags =
			MEDIA_PAD_FL_SINK | MEDIA_PAD_FL_MUST_CONNECT;
	dwe_dev->pads[DWE_PAD_SOURCE].flags =
			MEDIA_PAD_FL_SOURCE | MEDIA_PAD_FL_MUST_CONNECT;
	rc = media_entity_pads_init(&dwe_dev->sd.entity,
			DWE_PADS_NUM, dwe_dev->pads);
	if (rc)
		goto end;

	dwe_dev->core = dwe_devcore_init(dwe_dev, mem_res);

	dwe_dev->sd.fwnode = of_fwnode_handle(pdev->dev.of_node);
	rc = v4l2_async_register_subdev(&dwe_dev->sd);
	if (rc)
		goto end;

	pr_info("vvcam dewarp driver probed\n");
	return 0;
end:
	if (dwe_dev->core)
		dwe_devcore_deinit(dwe_dev);
	for (i = 0; i < DWE_PADS_NUM; ++i)
		vvbuf_ctx_deinit(&dwe_dev->bctx[i]);
	kfree(dwe_dev);
	return rc;
}

int dwe_hw_remove(struct platform_device *pdev)
{
	struct dwe_device *dwe = platform_get_drvdata(pdev);
	int i;

	pr_info("enter %s\n", __func__);

	if (!dwe)
		return -1;

	for (i = 0; i < DWE_PADS_NUM; ++i)
		vvbuf_ctx_deinit(&dwe->bctx[i]);
	dwe_devcore_deinit(dwe);
	media_entity_cleanup(&dwe->sd.entity);
	v4l2_async_unregister_subdev(&dwe->sd);

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

static const struct dev_pm_ops dwe_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(dwe_system_suspend, dwe_system_resume)
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
	if (ret) {
		pr_err("register platform driver failed.\n");
		return ret;
	}
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
