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
#include <linux/io.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/debugfs.h>
#include <linux/irq.h>

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
#include "isp_driver.h"
#include "isp_ioctl.h"
#include "cma.h"
#include "mrv_all_bits.h"

/* #define ENABLE_IRQ */
#define ISP_HW_IRQ_NUMBER 16
struct clk *clk_isp;

extern MrvAllRegister_t *all_regs;

int isp_open_node(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	struct isp_device *isp_dev = v4l2_get_subdevdata(sd);

	pr_debug("%s E open_cnt %u\n", __func__, ++isp_dev->isp_open_cnt);
	return 0;
}

int isp_close_node(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	struct isp_device *isp_dev = v4l2_get_subdevdata(sd);

	pr_debug("%s E open_cnt %u\n", __func__, --isp_dev->isp_open_cnt);
	return 0;
}

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

static struct v4l2_subdev_core_ops isp_v4l2_subdev_core_ops = {
	.ioctl = isp_ioctl,
	.subscribe_event = v4l2_ctrl_subdev_subscribe_event,
	.unsubscribe_event = v4l2_event_subdev_unsubscribe,
};

static struct v4l2_subdev_video_ops isp_v4l2_subdev_video_ops = {
	.s_stream = isp_set_stream,
};

struct v4l2_subdev_ops isp_v4l2_subdev_ops = {
	.core = &isp_v4l2_subdev_core_ops,
	.video = &isp_v4l2_subdev_video_ops,
};

struct v4l2_subdev_internal_ops isp_subdev_internal_ops = {
	.open = isp_open_node,
	.close = isp_close_node,
};

#ifdef ENABLE_IRQ
static uint32_t isp_hw_handle_reg(struct isp_device *isp_dev)
{
	uint32_t ret = 0;
	uint32_t isp_mis_addr = 0;
	uint32_t isp_icr_addr = 0;
	uint32_t isp_mis_val;
	uint32_t mi_mis_addr = 0;
	uint32_t mi_icr_addr = 0;
	uint32_t mi_mis_val;
	struct v4l2_event event;
	struct isp_irq_data *irq_data =
	    (struct isp_irq_data *)(&event.u.data[0]);

	isp_mis_addr = REG_ADDR(isp_mis);
	isp_icr_addr = REG_ADDR(isp_icr);
#ifdef ISP_MIV2
	mi_mis_addr = REG_ADDR(miv2_mis);
	mi_icr_addr = REG_ADDR(miv2_icr);
#endif
#ifdef ISP_MIV1
	mi_mis_addr = REG_ADDR(mi_mis);
	mi_icr_addr = REG_ADDR(mi_icr);
#endif
	if (!isp_mis_addr || !isp_icr_addr || !mi_mis_addr || !mi_icr_addr) {
		pr_err("get irq register addr failed.\n");
		return ret;
	}

	isp_mis_val = __raw_readl(isp_dev->ic_dev.base + isp_mis_addr);
	if (isp_mis_val != 0x0 && isp_mis_val != 0x20) {
		memset(irq_data, 0x0, sizeof(struct isp_irq_data));
		event.id = 1;
		event.type = VIV_VIDEO_ISPIRQ_TYPE;
		irq_data->addr = isp_mis_addr;
		irq_data->val = isp_mis_val;
		pr_err("%s isp irq addr:0x%x, isp_mis:0x%x.\n", __func__,
			irq_data->addr, irq_data->val);
		v4l2_event_queue(isp_dev->subdev.devnode, &event);
		ret = true;
	}
	__raw_writel(isp_mis_val, isp_dev->ic_dev.base + isp_icr_addr);

	mi_mis_val = __raw_readl(isp_dev->ic_dev.base + mi_mis_addr);
	if (mi_mis_val != 0x0) {
		memset(irq_data, 0x0, sizeof(struct isp_irq_data));
		event.id = 1;
		event.type = VIV_VIDEO_MIIRQ_TYPE;
		irq_data->addr = isp_icr_addr;
		irq_data->val = mi_mis_val;
		pr_err("%s mi irq addr:0x%x, mi_mis:0x%x.\n", __func__,
			irq_data->addr, irq_data->val);
		v4l2_event_queue(isp_dev->subdev.devnode, &event);
		ret = true;
	}
	__raw_writel(mi_mis_val, isp_dev->ic_dev.base + mi_icr_addr);

	return ret;
}

irqreturn_t isp_hw_isr(int irq, void *dev)
{
	irqreturn_t ret = IRQ_NONE;

	if (isp_hw_handle_reg(dev))
		ret = IRQ_HANDLED;

	return ret;
}
#endif

struct isp_device *isp_hw_register(struct v4l2_device *vdev, u64 base, u64 size)
{
	struct isp_device *isp_dev;
	struct device_node *node;
	struct device_node *node_p;
	int ret;
#ifdef ENABLE_IRQ
	int rc;
#endif

	pr_info("enter %s\n", __func__);
	node = of_find_compatible_node(NULL, NULL, "fsl,imx8mp-isp");
	if (!node) {
		pr_err("ISP node not found.\n");
		return NULL;
	}
	ret = of_clk_set_defaults(node, false);
	if (ret < 0) {
		pr_err("clk: couldn't set desired clock for ISP\n");
		return NULL;
	}

	/*
	 * trick to get ISP clock via power-domain in isp node in dts,
	 * as there is no power management in current driver.
	 */
	node_p = of_parse_phandle(node, "power-domains", 0);
	if (!node_p) {
		pr_err("couldn't find power-domain for ISP\n");
		return NULL;
	}
	clk_isp = of_clk_get(node_p, 0);
	if (IS_ERR(clk_isp)) {
		pr_err("couldn't get clock for ISP power-domain\n");
		of_node_put(node_p);
		return NULL;
	}
	of_node_put(node_p);

	clk_prepare_enable(clk_isp);

	isp_dev = kzalloc(sizeof(struct isp_device), GFP_KERNEL);
	if (!isp_dev) {
		goto end;
	}

	v4l2_subdev_init(&isp_dev->subdev, &isp_v4l2_subdev_ops);
	isp_dev->subdev.internal_ops = &isp_subdev_internal_ops;
	snprintf(isp_dev->subdev.name, sizeof(isp_dev->subdev.name), "viv_isp");
	isp_dev->subdev.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	isp_dev->subdev.flags |= V4L2_SUBDEV_FL_HAS_EVENTS;
	isp_dev->subdev.owner = THIS_MODULE;
	v4l2_set_subdevdata(&isp_dev->subdev, isp_dev);
	if (v4l2_device_register_subdev(vdev, &isp_dev->subdev) != 0) {
		return NULL;
	}
	isp_dev->isp_open_cnt = 0;
	isp_dev->ic_dev.base = ioremap(base, size);
#ifdef ISP_REG_RESET
	isp_dev->ic_dev.reset = ioremap(ISP_REG_RESET, 4);
#endif
	pr_debug("ioremap addr: 0x%llx 0x%llx, %p", base, size,
		isp_dev->ic_dev.base);
#ifdef ENABLE_IRQ
	rc = request_irq(ISP_HW_IRQ_NUMBER, isp_hw_isr, IRQF_SHARED, "vsi_isp_irq",
			 isp_dev);
	pr_debug("request_irq num:%d, rc:%d", ISP_HW_IRQ_NUMBER, rc);
#endif
	pr_info("vvcam isp driver registered\n");
	return isp_dev;
end:
	return NULL;
}

int isp_hw_unregister(struct isp_device *isp)
{
	struct v4l2_subdev *sd = NULL;

	pr_info("enter %s\n", __func__);
	if (!isp)
		return -1;
	sd = &isp->subdev;
#ifdef ENABLE_IRQ
	free_irq(ISP_HW_IRQ_NUMBER, isp);
#endif
	v4l2_device_unregister_subdev(sd);

	iounmap(isp->ic_dev.base);
	kzfree(isp);
	if (!IS_ERR(clk_isp))
		clk_disable_unprepare(clk_isp);
	return 0;
}
