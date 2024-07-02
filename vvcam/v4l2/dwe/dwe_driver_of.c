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
#include <linux/module.h>
#include <linux/pm_runtime.h>
#include <linux/version.h>
#include <media/v4l2-event.h>

#include "dwe_driver.h"
#include "dwe_ioctl.h"

#define DEWARP_NODE_NUM  (2)

static struct dwe_device *pdwe_dev[DEWARP_NODE_NUM] = {NULL};

static int dwe_subscribe_event(struct v4l2_subdev *sd, struct v4l2_fh *fh,
			struct v4l2_event_subscription *sub)
{
	return v4l2_event_subscribe(fh, sub, 2, NULL);
}

static int dwe_unsubscribe_event(struct v4l2_subdev *sd, struct v4l2_fh *fh,
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

static long dwe_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	int ret = 0;
	struct dwe_device *dwe_dev = v4l2_get_subdevdata(sd);

	mutex_lock(&dwe_dev->core->mutex);
	ret = dwe_ioctl_compat(sd, cmd, arg);
	mutex_unlock(&dwe_dev->core->mutex);
	return ret;
}
#else /* CONFIG_COMPAT */
static long dwe_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	int ret = 0;

	struct dwe_device *dwe_dev = v4l2_get_subdevdata(sd);

	mutex_lock(&dwe_dev->core->mutex);
	ret = dwe_devcore_ioctl(v4l2_get_subdevdata(sd), cmd, arg);
	mutex_unlock(&dwe_dev->core->mutex);
	return ret;
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

static int dwe_set_stream(struct v4l2_subdev *sd, int enable)
{
	struct dwe_device *dwe_dev = v4l2_get_subdevdata(sd);
	struct vvbuf_ctx *ctx;
	struct media_pad *pad;
	unsigned long flags;

	if (!enable)
		dwe_dev->state &= ~STATE_STREAM_STARTED;
	else
		dwe_dev->state |= STATE_STREAM_STARTED;

	pad = &dwe_dev->pads[DWE_PAD_SINK];

	pad = media_pad_remote_pad_first(pad);

	if (pad && is_media_entity_v4l2_subdev(pad->entity)) {
		sd = media_entity_to_v4l2_subdev(pad->entity);
		v4l2_subdev_call(sd, video, s_stream, enable);
	}

	if (!enable) {
		ctx = &dwe_dev->bctx[DWE_PAD_SOURCE];
		spin_lock_irqsave(&ctx->irqlock, flags);
		if (!list_empty(&ctx->dmaqueue))
			list_del_init(&ctx->dmaqueue);
		spin_unlock_irqrestore(&ctx->irqlock, flags);
	}
	return 0;
}

static const struct v4l2_subdev_core_ops dwe_v4l2_subdev_core_ops = {
	.ioctl = dwe_ioctl,
	.subscribe_event = dwe_subscribe_event,
	.unsubscribe_event = dwe_unsubscribe_event,
};

static const struct v4l2_subdev_video_ops dwe_v4l2_subdev_video_ops = {
	.s_stream = dwe_set_stream,
};

static const struct v4l2_subdev_ops dwe_v4l2_subdev_ops = {
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

	if (unlikely(!ctx || !buf))
		return;
	sd = media_entity_to_v4l2_subdev(buf->pad->entity);
	dwe = container_of(sd, struct dwe_device, sd);

	if (!dwe || !(dwe->state & STATE_STREAM_STARTED)) {
		vvbuf_ready(ctx, buf->pad, buf);
		return;
	}

	ctx = &dwe->core->bctx[DWE_PAD_SINK];
	vvbuf_push_buf(ctx, buf);
	if ((dwe->core->ic_dev.hardware_status == HARDWARE_IDLE) &&
	    (dwe->state == (STATE_DRIVER_STARTED | STATE_STREAM_STARTED))) {
		dwe->core->ic_dev.hardware_status = HARDWARE_BUSY;
		tasklet_schedule(&dwe->core->ic_dev.tasklet);
	}
}

static const struct vvbuf_ops dwe_src_buf_ops = {
	.notify = dwe_src_buf_notify,
};

static void dwe_dst_buf_notify(struct vvbuf_ctx *ctx, struct vb2_dc_buf *buf)
{
	if (unlikely(!ctx || !buf))
		return;
	vvbuf_push_buf(ctx, buf);
}

static const struct vvbuf_ops dwe_dst_buf_ops = {
	.notify = dwe_dst_buf_notify,
};

static void fake_pdev_release(struct device *dev)
{
}

static struct platform_device fake_pdev = {
	.name = "fsl,fake-imx8mp-dwe",
	.id   = 1,
	.dev.release = fake_pdev_release,
};

static int dwe_fake_pdev_creat(void)
{
	return  platform_device_register(&fake_pdev);
}

static void dwe_fake_pdev_destory(void)
{
	platform_device_unregister(&fake_pdev);
}

static int dwe_open(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	int ret = 0;
	struct dwe_device *dwe_dev = v4l2_get_subdevdata(sd);

	mutex_lock(&dwe_dev->core->mutex);
	pm_runtime_get_sync(pdwe_dev[0]->sd.dev);
	if ((pdwe_dev[0]->refcnt > 0) || (pdwe_dev[1]->refcnt > 0))
		goto unlock;

	if ((pdwe_dev[0]->refcnt + pdwe_dev[1]->refcnt) == 0) {
		msleep(1);
		dwe_clear_interrupts(&pdwe_dev[0]->core->ic_dev);
		if (devm_request_irq(pdwe_dev[0]->sd.dev,
					pdwe_dev[0]->irq,
					dwe_hw_isr, IRQF_SHARED,
					dev_name(pdwe_dev[0]->sd.dev),
					&pdwe_dev[0]->core->ic_dev) != 0) {
			pr_err("failed to request irq.\n");
			pm_runtime_put_sync(pdwe_dev[0]->sd.dev);
			ret = -1;
			goto unlock;
		}
	}

unlock:
	if (ret == 0)
		dwe_dev->refcnt++;
	mutex_unlock(&dwe_dev->core->mutex);
	return ret;
}

static int dwe_close(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	struct dwe_device *dwe_dev = v4l2_get_subdevdata(sd);
	struct vvbuf_ctx *ctx;
	unsigned long flags;

	if (dwe_dev->refcnt == 0)
		return 0;

	mutex_lock(&dwe_dev->core->mutex);
	dwe_dev->refcnt--;
	if (dwe_dev->refcnt == 0){
		dwe_dev->state = 0;
		msleep(10);
		ctx = &dwe_dev->bctx[DWE_PAD_SOURCE];
		spin_lock_irqsave(&ctx->irqlock, flags);
		if (!list_empty(&ctx->dmaqueue))
			list_del_init(&ctx->dmaqueue);
		spin_unlock_irqrestore(&ctx->irqlock, flags);
	}
	if (((pdwe_dev[0]->refcnt) != 0) || ((pdwe_dev[1]->refcnt) != 0)) {
		goto exit;
	}
	dwe_priv_ioctl(&pdwe_dev[0]->core->ic_dev, DWEIOC_STOP, NULL);
	devm_free_irq(pdwe_dev[0]->sd.dev, pdwe_dev[0]->irq, &pdwe_dev[0]->core->ic_dev);
	dwe_clear_interrupts(&pdwe_dev[0]->core->ic_dev);
	pdwe_dev[0]->core->state = 0;

	msleep(5);

	dwe_clean_src_memory(&pdwe_dev[0]->core->ic_dev);
exit:
	pm_runtime_put(pdwe_dev[0]->sd.dev);
	mutex_unlock(&dwe_dev->core->mutex);
	return 0;
}

static const struct v4l2_subdev_internal_ops dwe_internal_ops = {
	.open = dwe_open,
	.close = dwe_close,
};

struct v4l2_subdev *g_dwe_subdev[DEWARP_NODE_NUM] = {NULL};
EXPORT_SYMBOL_GPL(g_dwe_subdev);

static int dwe_hw_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct dwe_device *dwe_dev;
	struct resource *mem_res;
	int irq;
	int rc, i, index;
	int dev_id;

	pr_info("enter %s\n", __func__);

	mem_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!mem_res) {
		pr_err("can't fetch device resource info\n");
		return -ENODEV;
	}

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		pr_err("failed to get irq number.\n");
		return -ENODEV;
	}

	rc = dwe_fake_pdev_creat();
	if (rc < 0) {
		pr_err("failed to creat fake pdev for dwe1.\n");
		goto dewarp_destory_fake_pdev;
	}

	for (dev_id = 0; dev_id < DEWARP_NODE_NUM; dev_id++) {
		pdwe_dev[dev_id] =
			kzalloc(sizeof(struct dwe_device), GFP_KERNEL);
			if (!pdwe_dev[dev_id]) {
				if (dev_id == 0)
					goto dewarp_destory_fake_pdev;
				else {
					dev_id = dev_id - 1;
					goto dewarp_entity_pads_deinit;
				}

		}

		dwe_dev = pdwe_dev[dev_id];
		dwe_dev->id = dev_id;
		if (dev_id == 0){
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
		}
		dwe_dev->sd.internal_ops = &dwe_internal_ops;
		v4l2_subdev_init(&dwe_dev->sd, &dwe_v4l2_subdev_ops);
		snprintf(dwe_dev->sd.name, sizeof(dwe_dev->sd.name),
				"%s.%d", DWE_DEVICE_NAME, dwe_dev->id);
		dwe_dev->sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
		dwe_dev->sd.flags |= V4L2_SUBDEV_FL_HAS_EVENTS;
		dwe_dev->sd.owner = THIS_MODULE;

		v4l2_set_subdevdata(&dwe_dev->sd, dwe_dev);

		if (dev_id == 0)
			dwe_dev->sd.dev = &pdev->dev;
		else
			dwe_dev->sd.dev = &fake_pdev.dev;
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
		if (rc < 0)
			goto dewarp_entity_pads_deinit;
	}

	for (dev_id = 0; dev_id < DEWARP_NODE_NUM; dev_id++) {
		dwe_dev = pdwe_dev[dev_id];
		for (i = 0; i < DWE_PADS_NUM; ++i)
			vvbuf_ctx_init(&dwe_dev->bctx[i]);

		dwe_dev->refcnt = 0;
		dwe_dev->bctx[DWE_PAD_SINK].ops = &dwe_src_buf_ops;
		dwe_dev->bctx[DWE_PAD_SOURCE].ops = &dwe_dst_buf_ops;
		dwe_dev->irq = irq;

		dwe_dev->core = dwe_devcore_init(dwe_dev, mem_res);
		if (dev_id == 0)
			dwe_dev->sd.fwnode = of_fwnode_handle(pdev->dev.of_node);
		else
			dwe_dev->sd.fwnode = &dwe_dev->fwnode;

		rc = v4l2_async_register_subdev(&dwe_dev->sd);
		if (rc < 0)
			goto dewarp_core_deinit;
		g_dwe_subdev[dev_id] = &dwe_dev->sd;
	}
	pm_runtime_enable(&pdev->dev);
	pr_info("vvcam dewarp driver probed\n");
	return 0;

dewarp_core_deinit:
	dwe_devcore_deinit(pdwe_dev[0]);
	for (index = 0; index <= dev_id; index++) {
		dwe_dev = pdwe_dev[index];
		for (i = 0; i < DWE_PADS_NUM; i++)
			vvbuf_ctx_deinit(&dwe_dev->bctx[i]);
	}
	dev_id = DEWARP_NODE_NUM - 1;

dewarp_entity_pads_deinit:
	for (index = 0; index <= dev_id; index++) {
		dwe_dev = pdwe_dev[index];
		media_entity_cleanup(&dwe_dev->sd.entity);
		kfree(dwe_dev);
	}
dewarp_destory_fake_pdev:
	dwe_fake_pdev_destory();
	return rc;
}

static void dwe_hw_remove(struct platform_device *pdev)
{
	struct dwe_device *dwe_dev;
	int dev_id;
	int i;

	pr_info("enter %s\n", __func__);
	pm_runtime_disable(&pdev->dev);
	dwe_devcore_deinit(pdwe_dev[0]);

	for (dev_id = 0; dev_id < DEWARP_NODE_NUM; dev_id++) {
		dwe_dev = pdwe_dev[dev_id];
		for (i = 0; i < DWE_PADS_NUM; i++)
			vvbuf_ctx_deinit(&dwe_dev->bctx[i]);
		media_entity_cleanup(&dwe_dev->sd.entity);
		v4l2_async_unregister_subdev(&dwe_dev->sd);
		kfree(dwe_dev);
	}
	dwe_fake_pdev_destory();
	pr_info("vvcam dewarp driver removed\n");
	return;
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
	struct dwe_device *dwe_dev = pdwe_dev[0];

	dwe_disable_clocks(dwe_dev);

	return 0;
}

static int dwe_runtime_resume(struct device *dev)
{
	struct dwe_device *dwe_dev = pdwe_dev[0];

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
