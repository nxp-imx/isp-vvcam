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
#include <media/v4l2-event.h>
#include <linux/mfd/syscon.h>
#include <linux/regmap.h>
#include <linux/of_reserved_mem.h>
#include <linux/pm_domain.h>
#include <linux/version.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

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
	int ret = 0;
	struct isp_device *isp_dev = v4l2_get_subdevdata(sd);
	mutex_lock(&isp_dev->mlock);
	ret = isp_ioctl_compat(sd, cmd, arg);
	mutex_unlock(&isp_dev->mlock);
	return ret;
}
#else /* CONFIG_COMPAT */
long isp_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	int ret = 0;
	struct isp_device *isp_dev = v4l2_get_subdevdata(sd);

	mutex_lock(&isp_dev->mlock);
	ret = isp_priv_ioctl(&isp_dev->ic_dev, cmd, arg);
	mutex_unlock(&isp_dev->mlock);

	return ret;
}
#endif /* CONFIG_COMPAT */

static int isp_enable_clocks(struct isp_device *isp_dev)
{
	int ret;

	ret = clk_prepare_enable(isp_dev->clk_sensor);
	if (ret)
		return ret;

	ret = clk_prepare_enable(isp_dev->clk_core);
	if (ret)
		goto disable_clk_sensor;

	ret = clk_prepare_enable(isp_dev->clk_axi);
	if (ret)
		goto disable_clk_core;

	ret = clk_prepare_enable(isp_dev->clk_ahb);
	if (ret)
		goto disable_clk_axi;

	return 0;

disable_clk_axi:
	clk_disable_unprepare(isp_dev->clk_axi);
disable_clk_core:
	clk_disable_unprepare(isp_dev->clk_core);
disable_clk_sensor:
	clk_disable_unprepare(isp_dev->clk_sensor);

	return ret;
}

static void isp_disable_clocks(struct isp_device *isp_dev)
{
	clk_disable_unprepare(isp_dev->clk_sensor);
	clk_disable_unprepare(isp_dev->clk_ahb);
	clk_disable_unprepare(isp_dev->clk_axi);
	clk_disable_unprepare(isp_dev->clk_core);
}

int isp_set_stream(struct v4l2_subdev *sd, int enable)
{
	struct isp_device *isp_dev = v4l2_get_subdevdata(sd);
	pr_info("enter %s %d\n", __func__, enable);

	if (!enable) {
		isp_dev->state &= ~STATE_STREAM_STARTED;
	} else
		isp_dev->state |= STATE_STREAM_STARTED;
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

struct v4l2_subdev_ops isp_v4l2_subdev_ops = {
	.core = &isp_v4l2_subdev_core_ops,
	.video = &isp_v4l2_subdev_video_ops,
};

static int isp_link_setup(struct media_entity *entity,
				const struct media_pad *local,
				const struct media_pad *remote, u32 flags)
{
	return 0;
}

static const struct media_entity_operations isp_media_ops = {
	.link_setup = isp_link_setup,
	.link_validate = v4l2_subdev_link_validate,
};

static void isp_buf_notify(struct vvbuf_ctx *ctx, struct vb2_dc_buf *buf)
{
	struct v4l2_subdev *sd;
	struct isp_device *isp_dev;
	if (unlikely(!ctx || !buf))
		return;
	sd = media_entity_to_v4l2_subdev(buf->pad->entity);
	isp_dev = container_of(sd, struct isp_device, sd);
	if (unlikely(isp_dev->refcnt == 0)) {
		isp_dev->ic_dev.free(&isp_dev->ic_dev, buf);
		return;
	}
	vvbuf_push_buf(ctx,buf);
	return;
}

static const struct vvbuf_ops isp_buf_ops = {
	.notify = isp_buf_notify,
};

static int isp_buf_alloc(struct isp_ic_dev *dev, struct isp_buffer_context *buf)
{
	struct isp_device *isp_dev;
	struct media_pad *remote_pad;
	struct vb2_dc_buf *buff;

	if (!dev || !buf)
		return -EINVAL;

	isp_dev = container_of(dev, struct isp_device, ic_dev);

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 19, 0)
	remote_pad = media_entity_remote_pad(&isp_dev->pads[ISP_PAD_SOURCE]);
#else
	remote_pad = media_pad_remote_pad_first(&isp_dev->pads[ISP_PAD_SOURCE]);
#endif
	if (remote_pad) {
		if (is_media_entity_v4l2_video_device(remote_pad->entity)) {
			/* isp connect to video, so no need allo buf for isp */
			return 0;
		}
	}

	buff = kzalloc(sizeof(struct vb2_dc_buf), GFP_KERNEL);
	if (!buff)
		return -ENOMEM;

	buff->pad = &isp_dev->pads[ISP_PAD_SOURCE];
#ifdef ISP_MP_34BIT
	buff->dma = buf->addr_y << 2;
#else
	buff->dma = buf->addr_y;
#endif
	buff->flags = 1;
	vvbuf_push_buf(&isp_dev->bctx, buff);

	return 0;
}

static int isp_buf_free(struct isp_ic_dev *dev, struct vb2_dc_buf *buf)
{
	if (buf && buf->flags)
		kfree(buf);
	return 0;
}

static int isp_open(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	struct isp_device *isp_dev = v4l2_get_subdevdata(sd);

	mutex_lock(&isp_dev->mlock);
	pm_runtime_get_sync(sd->dev);
	isp_dev->refcnt++;
	if (isp_dev->refcnt == 1) {
		msleep(1);
		isp_clear_interrupts(&isp_dev->ic_dev);
		if (devm_request_irq(sd->dev, isp_dev->irq,
				isp_hw_isr,IRQF_TRIGGER_HIGH | IRQF_SHARED,
				dev_name(sd->dev), &isp_dev->ic_dev) != 0) {
			pr_err("failed to request irq.\n");
			isp_dev->refcnt = 0;
			pm_runtime_put_sync(sd->dev);
			mutex_unlock(&isp_dev->mlock);
			return -1;
		}
	}
	mutex_unlock(&isp_dev->mlock);
	return 0;
}

static int isp_close(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	struct isp_device *isp_dev = v4l2_get_subdevdata(sd);

	if (isp_dev->refcnt == 0)
		return 0;

	mutex_lock(&isp_dev->mlock);
	isp_dev->refcnt--;
	if (isp_dev->refcnt == 0){
		if (isp_dev->state & STATE_DRIVER_STARTED)
			isp_mi_stop(&isp_dev->ic_dev);
		isp_dev->state = STATE_STOPPED;
		devm_free_irq(sd->dev, isp_dev->irq, &isp_dev->ic_dev);
		isp_priv_ioctl(&isp_dev->ic_dev, ISPIOC_RESET, NULL);
		isp_clear_interrupts(&isp_dev->ic_dev);
		msleep(5);
		clean_dma_buffer(&isp_dev->ic_dev);
	}

	pm_runtime_put(sd->dev);
	mutex_unlock(&isp_dev->mlock);
	return 0;
}

static struct v4l2_subdev_internal_ops isp_internal_ops = {
	.open = isp_open,
	.close = isp_close,
};


static int isp_info_procfs_show(struct seq_file *sfile, void *offset)
{
	struct isp_device *isp_dev;
	isp_dev = (struct isp_device *) sfile->private;

	seq_printf(sfile ,"/********************************VSI ISP%d INFO********************************/\n",
				isp_dev->id);
	seq_printf(sfile, "width\t height\t frame_in\t frame_out\t frame_loss\t fps\t status\n");
	seq_printf(sfile, "%d\t %d\t %-16lld%-16lld%-16lld%d.%d\t %s\n",
				isp_dev->ic_dev.ctx.isWindow.width, isp_dev->ic_dev.ctx.isWindow.height,
				isp_dev->ic_dev.frame_in_cnt,
				isp_dev->ic_dev.frame_in_cnt - isp_dev->ic_dev.frame_loss_cnt[0],
				isp_dev->ic_dev.frame_loss_cnt[0],
				isp_dev->ic_dev.fps[0] / 100, isp_dev->ic_dev.fps[0] % 100,
				isp_dev->ic_dev.streaming ? "run" : "idle");
	return 0;
}

static int isp_procfs_open(struct inode *inode, struct file *file)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 17, 0)
	return single_open(file, isp_info_procfs_show, PDE_DATA(inode));
#else
	return single_open(file, isp_info_procfs_show, pde_data(inode));
#endif
}

static ssize_t isp_procfs_write(struct file *file,
		const char __user *buffer, size_t count, loff_t *ppos)
{
	struct isp_device *isp_dev;
	char strbuf[128];
	char str[128] = "";
	int i;

	if (count >= 128) {
		return count;
	}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 17, 0)
	isp_dev = (struct isp_device *) PDE_DATA(file_inode(file));
#else
	isp_dev = (struct isp_device *) pde_data(file_inode(file));
#endif

	if (copy_from_user(strbuf, buffer, count))
		return -EFAULT;
	strbuf[count] = '\0';
	sscanf(strbuf, "%s", str);

	if (!strcmp("clear", str) || !strlen(str)) {
		isp_dev->ic_dev.frame_in_cnt = 0;
		for (i = 0; i < MI_PATH_NUM; i++) {
			isp_dev->ic_dev.frame_loss_cnt[i] = 0;
			isp_dev->ic_dev.fps[i] = 0;
		}
	}
	return count;
}

static const struct proc_ops isp_procfs_ops = {
	.proc_open = isp_procfs_open,
	.proc_release = seq_release,
	.proc_read = seq_read,
	.proc_write = isp_procfs_write,
	.proc_lseek = seq_lseek,
};

static int isp_create_procfs(struct isp_device *isp_dev)
{
	struct proc_dir_entry *ent;
	char isp_procfs_filename[30];
	if (!isp_dev) {
		return -1;
	}
	sprintf(isp_procfs_filename, "vsiisp%d", isp_dev->id);

	ent = proc_create_data(isp_procfs_filename, 0444, NULL, &isp_procfs_ops, isp_dev);
	if (!ent)
		return -1;

	isp_dev->pde = ent;
	return 0;
}


/**
 * isp_attach_pm_domains() - attach the power domains
 * On i.MX8MP there is multiple power domains
 * required, so need to link them.
 */
static int isp_attach_pm_domains(struct isp_device *isp_dev,
		struct device *dev)
{
	int ret, i;
	struct isp_pd *priv;

	isp_dev->num_domains = of_count_phandle_with_args(dev->of_node,
							"power-domains",
							"#power-domain-cells");
	if (isp_dev->num_domains <= 1)
		return 0;

	isp_dev->priv = devm_kzalloc(dev, sizeof(struct isp_pd),
					GFP_KERNEL);
	if (!isp_dev->priv) {
		dev_err(dev, "failed alloc the priv\n");
		return -ENOMEM;
	}

	priv = isp_dev->priv;
	priv->num_domains = isp_dev->num_domains;
	priv->pd_dev = devm_kmalloc_array(dev, priv->num_domains,
					  sizeof(*priv->pd_dev),
					  GFP_KERNEL);
	if (!priv->pd_dev) {
		ret = -ENOMEM;
		goto end;
	}

	priv->pd_dev_link = devm_kmalloc_array(dev, priv->num_domains,
					       sizeof(*priv->pd_dev_link),
					       GFP_KERNEL);
	if (!priv->pd_dev_link) {
		ret = -ENOMEM;
		goto end;
	}

	for (i = 0; i < priv->num_domains; i++) {
		priv->pd_dev[i] = dev_pm_domain_attach_by_id(dev, i);
		if (IS_ERR(priv->pd_dev[i])) {
			ret = PTR_ERR(priv->pd_dev[i]);
			goto detach_pm;
		}

		/*
		 * device_link_add will check priv->pd_dev[i], if it is
		 * NULL, then will break.
		 */
		priv->pd_dev_link[i] = device_link_add(dev,
						       priv->pd_dev[i],
						       DL_FLAG_STATELESS |
						       DL_FLAG_PM_RUNTIME);
		if (!priv->pd_dev_link[i]) {
			dev_pm_domain_detach(priv->pd_dev[i], false);
			ret = -EINVAL;
			goto detach_pm;
		}
	}

	return 0;

detach_pm:
	while (--i >= 0) {
		device_link_del(priv->pd_dev_link[i]);
		dev_pm_domain_detach(priv->pd_dev[i], false);
	}

end:
	devm_kfree(dev, isp_dev->priv);
	return ret;
}

static int isp_detach_pm_domains(struct isp_device *isp_dev)
{
	int i;
	struct isp_pd *priv = isp_dev->priv;

	if (isp_dev->num_domains <= 1)
		return 0;

	if (priv == NULL) {
		pr_err("isp device priv is null!\n");
		return -ENOMEM;
	}

	for (i = 0; i < priv->num_domains; i++) {
		device_link_del(priv->pd_dev_link[i]);
		dev_pm_domain_detach(priv->pd_dev[i], false);
	}

	return 0;
}

int isp_hw_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct isp_device *isp_dev;
	struct resource *mem_res;
	int irq;
	int rc;
	int i;
	pr_info("enter %s\n", __func__);
	isp_dev = kzalloc(sizeof(struct isp_device), GFP_KERNEL);
	if (!isp_dev)
		return -ENOMEM;

	mutex_init(&isp_dev->mlock);

	rc = fwnode_property_read_u32(of_fwnode_handle(pdev->dev.of_node),
			"id", &isp_dev->id);
	if (rc) {
		pr_info("isp device id not found, use the default.\n");
		isp_dev->id = 0;
	}
	isp_dev->ic_dev.id = isp_dev->id;

	/* There are multiple power domains required by ISP on imx865 platform */
	rc = isp_attach_pm_domains(isp_dev, dev);
	if (rc) {
		dev_err(dev, "isp_attach_pm_domains failed\n");
		goto err_put_isp;
	}

	isp_dev->clk_core = devm_clk_get(dev, "core");
	if (IS_ERR(isp_dev->clk_core)) {
		rc = PTR_ERR(isp_dev->clk_core);
		dev_err(dev, "can't get core clock: %d\n", rc);
		goto err_detach_domains;
	}

	isp_dev->clk_axi = devm_clk_get(dev, "axi");
	if (IS_ERR(isp_dev->clk_axi)) {
		rc = PTR_ERR(isp_dev->clk_axi);
		dev_err(dev, "can't get axi clock: %d\n", rc);
		goto err_detach_domains;
	}

	isp_dev->clk_ahb = devm_clk_get(dev, "ahb");
	if (IS_ERR(isp_dev->clk_ahb)) {
		rc = PTR_ERR(isp_dev->clk_ahb);
		dev_err(dev, "can't get ahb clock: %d\n", rc);
		goto err_detach_domains;
	}

	isp_dev->clk_sensor = devm_clk_get(dev, "sensor");
	if (IS_ERR(isp_dev->clk_sensor)) {
		rc = PTR_ERR(isp_dev->clk_sensor);
		dev_err(dev, "can't get sensor clock: %d\n", rc);
		goto err_detach_domains;
	}

	isp_dev->sd.internal_ops = &isp_internal_ops;

#ifdef ISP8000NANO_V1802
	isp_dev->ic_dev.mix_gpr = syscon_regmap_lookup_by_phandle(
		pdev->dev.of_node, "gpr");
	if (IS_ERR(isp_dev->ic_dev.mix_gpr)) {
		pr_warn("failed to get mix gpr\n");
		isp_dev->ic_dev.mix_gpr = NULL;
		rc = -ENOMEM;
		goto err_detach_domains;
	}
#endif
	v4l2_subdev_init(&isp_dev->sd, &isp_v4l2_subdev_ops);
	snprintf(isp_dev->sd.name, sizeof(isp_dev->sd.name),
			"%s.%d", ISP_DEVICE_NAME, isp_dev->id);
	isp_dev->sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	isp_dev->sd.flags |= V4L2_SUBDEV_FL_HAS_EVENTS;
	isp_dev->sd.owner = THIS_MODULE;
	v4l2_set_subdevdata(&isp_dev->sd, isp_dev);
	isp_dev->sd.dev = &pdev->dev;

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
	isp_dev->ic_dev.state = &isp_dev->state;

	vvbuf_ctx_init(&isp_dev->bctx);
	isp_dev->bctx.ops = &isp_buf_ops;
	isp_dev->ic_dev.bctx = &isp_dev->bctx;

	isp_dev->ic_dev.alloc = isp_buf_alloc;
	isp_dev->ic_dev.free = isp_buf_free;

	isp_dev->ic_dev.frame_in_cnt = 0;
	for (i = 0; i < MI_PATH_NUM; i++) {
		isp_dev->ic_dev.frame_loss_cnt[i] = 0;
		isp_dev->ic_dev.fps[i] = 0;
	}

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		pr_err("failed to get irq number.\n");
		goto end;
	}

	isp_dev->irq = irq;
	pr_debug("request_irq num:%d, rc:%d", irq, rc);
	spin_lock_init(&isp_dev->ic_dev.lock);
	spin_lock_init(&isp_dev->ic_dev.irqlock);
	tasklet_init(&isp_dev->ic_dev.tasklet, isp_isr_tasklet, (unsigned long)(&isp_dev->ic_dev));

	platform_set_drvdata(pdev, isp_dev);

	isp_dev->sd.entity.name = isp_dev->sd.name;
	isp_dev->sd.entity.obj_type = MEDIA_ENTITY_TYPE_V4L2_SUBDEV;
	isp_dev->sd.entity.function = MEDIA_ENT_F_IO_V4L;
	isp_dev->sd.entity.ops = &isp_media_ops;
	isp_dev->pads[ISP_PAD_SOURCE].flags =
			MEDIA_PAD_FL_SOURCE | MEDIA_PAD_FL_MUST_CONNECT;
	rc = media_entity_pads_init(&isp_dev->sd.entity,
			ISP_PADS_NUM, isp_dev->pads);
	if (rc)
		goto end;

	isp_dev->sd.fwnode = of_fwnode_handle(pdev->dev.of_node);
	rc = v4l2_async_register_subdev(&isp_dev->sd);
	if (rc)
		goto end;

	rc = isp_create_procfs(isp_dev);
	if (rc) {
		pr_err("create isp proc fs failed.\n");
		goto end;
	}

	pm_runtime_enable(&pdev->dev);

	pr_info("vvcam isp driver registered\n");
	return 0;
end:
	pm_runtime_disable(&pdev->dev);

err_detach_domains:
	vvbuf_ctx_deinit(&isp_dev->bctx);
	isp_detach_pm_domains(isp_dev);

err_put_isp:
	devm_kfree(dev, isp_dev->priv);
	kfree(isp_dev);

	return rc;
}

int isp_hw_remove(struct platform_device *pdev)
{
	struct isp_device *isp = platform_get_drvdata(pdev);
	int rc;

	pr_info("enter %s\n", __func__);
	if (!isp)
		return -1;

	tasklet_kill(&isp->ic_dev.tasklet);
	vvbuf_ctx_deinit(&isp->bctx);
	media_entity_cleanup(&isp->sd.entity);
	v4l2_async_unregister_subdev(&isp->sd);

	rc = isp_detach_pm_domains(isp);

	if (rc) {
		pr_err("vvcam isp detach pm domain failed\n");
		return rc;
	}

	proc_remove(isp->pde);
	pm_runtime_disable(&pdev->dev);
	kfree(isp);
	pr_info("vvcam isp driver removed\n");
	return 0;
}

static int isp_system_suspend(struct device *dev)
{
	struct platform_device *pdev;
	struct isp_device *isp = NULL;
	pdev = container_of(dev, struct platform_device, dev);
	isp = platform_get_drvdata(pdev);
	if(!isp){
		dev_err(dev, "isp suspend failed!\n");
		return -1;
	}

	if(isp->ic_dev.streaming == true) {
		isp_stop_stream(&isp->ic_dev);
	}

	return pm_runtime_force_suspend(dev);
}

static int isp_system_resume(struct device *dev)
{
	int ret;
	struct platform_device *pdev;
	struct isp_device *isp = NULL;
	ret = pm_runtime_force_resume(dev);
	if (ret < 0) {
		dev_err(dev, "force resume %s failed!\n", dev_name(dev));
		return ret;
	}
	pdev = container_of(dev, struct platform_device, dev);
	isp = platform_get_drvdata(pdev);
	if(!isp){
		dev_err(dev, "isp resume failed!\n");
		return -1;
	}

	if(isp->ic_dev.streaming == true) {
		isp_start_stream(&isp->ic_dev, 1);
	}
	return 0;
}

static int isp_runtime_suspend(struct device *dev)
{
	struct isp_device *isp_dev = dev_get_drvdata(dev);

	isp_disable_clocks(isp_dev);

	return 0;
}

static int isp_runtime_resume(struct device *dev)
{
	struct isp_device *isp_dev = dev_get_drvdata(dev);

	isp_enable_clocks(isp_dev);

	return 0;
}

static const struct dev_pm_ops isp_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(isp_system_suspend, isp_system_resume)
	SET_RUNTIME_PM_OPS(isp_runtime_suspend, isp_runtime_resume, NULL)
};

static const struct of_device_id isp_of_match[] = {
	{.compatible = ISP_COMPAT_NAME,},
	{ /* sentinel */ },
};

MODULE_DEVICE_TABLE(of, isp_of_match);

static struct platform_driver viv_isp_driver = {
	.probe = isp_hw_probe,
	.remove = isp_hw_remove,
	.driver = {
		   .name = ISP_DEVICE_NAME,
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
