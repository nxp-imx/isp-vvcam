/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 VeriSilicon Holdings Co., Ltd.
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
 * Copyright (c) 2021 VeriSilicon Holdings Co., Ltd.
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
#include <linux/acpi.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/pm_runtime.h>
#include <linux/version.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include "vvfocus.h"

#define DW9790_MIN_FOCUS_POS 0
#define DW9790_MAX_FOCUS_POS 1022
#define DW9790_FOCUS_STEPS 2
#define DW9790_FOCUS_DEF 0

#define DW9790_MSB_ADDR 0x00
#define DW9790_LSB_ADDR 0x01
#define DW9790_INIT_ADDR 0x02

struct dw9790_device {
    uint32_t id;
    char name[16];
    struct v4l2_ctrl_handler ctrls_vcm;
    struct v4l2_ctrl *focus;
    struct v4l2_subdev sd;
    struct mutex lock;
    int32_t cur_pos;
};

static inline struct dw9790_device *sd_to_dw9790_device(struct v4l2_subdev *subdev)
{
	return container_of(subdev, struct dw9790_device, sd);
}

static inline struct dw9790_device *ctrl_to_dw9790_device(struct v4l2_ctrl *ctrl)
{
    return container_of(ctrl->handler, struct dw9790_device, ctrls_vcm);
}

static int  dw9790_i2c_write(struct dw9790_device *dw9790_dev, uint8_t addr, uint8_t *data, uint32_t size)
{
    struct i2c_client *client = v4l2_get_subdevdata(&dw9790_dev->sd);
    int ret = 0;
    uint8_t au8buf[3];

    if ((size == 0) || (size > 2))
         return -1;

    au8buf[0] = addr;
    au8buf[1] = *data;
    if (size == 2) {
        au8buf[2] = *(data + 1);
    }

    ret = i2c_master_send(client, au8buf, size + 1);
    if (ret != size + 1) {
        dev_err(dw9790_dev->sd.dev, "write dw9790 reg error : reg=%x, data=%x\n", addr, *data);
        return -1;
    }
    return 0;
}

static int dw9790_i2c_read(struct dw9790_device *dw9790_dev, uint8_t addr, uint8_t *data)
{
    struct i2c_client *client = v4l2_get_subdevdata(&dw9790_dev->sd);
    uint8_t reg = addr;
    uint8_t value = 0;
    int ret = 0;

    ret = i2c_master_send(client, &reg, sizeof(reg));
    if (ret != sizeof(reg)) {
        dev_err(dw9790_dev->sd.dev, "read reg error: reg=%x\n", addr);
        return -1;
    }

    ret = i2c_master_recv(client, &value, sizeof(value));
    if (ret != sizeof(value)) {
        dev_err(dw9790_dev->sd.dev, "read reg error: reg=%x, data=%x\n", addr, value);
        return -1;
    }
    *data = value;
    return 0;
}

static int dw9790_open(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	return pm_runtime_resume_and_get(sd->dev);
}

static int dw9790_close(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
    pm_runtime_put(sd->dev);
    return 0;
}

static const struct v4l2_subdev_internal_ops dw9790_int_ops = {
    .open = dw9790_open,
    .close = dw9790_close,
};

static int dw9790_query_capability(struct dw9790_device *dw9790_dev, void *arg)
{
	struct v4l2_capability *pcap = (struct v4l2_capability *)arg;

	strcpy((char *)pcap->driver, dw9790_dev->name);
	pcap->bus_info[0] = (uint8_t)dw9790_dev->id;
	return 0;
}

static int dw9790_get_range(struct dw9790_device *dw9790_dev, struct vvfocus_range_s *focus_range)
{
    focus_range->min_pos = DW9790_MIN_FOCUS_POS;
    focus_range->max_pos = DW9790_MAX_FOCUS_POS;
    focus_range->step    = DW9790_FOCUS_STEPS;
    return 0;
}

static int dw9790_set_pos(struct dw9790_device *dw9790_dev, struct vvfocus_pos_s *pfocus_pos)
{
    int ret = 0;
    int32_t len_pos;
    uint8_t data[2];
    if (pfocus_pos->mode == VVFOCUS_MODE_ABSOLUTE) {
        len_pos = pfocus_pos->pos;
    } else {
        len_pos = pfocus_pos->pos + dw9790_dev->cur_pos;
    }

    if ((len_pos > DW9790_MAX_FOCUS_POS) ||
        (len_pos < DW9790_MIN_FOCUS_POS))
        return -1;

    data[0] = (len_pos & 0x3fc) >> 2;
    data[1] = (len_pos & 0x03) << 6;
    ret = dw9790_i2c_write(dw9790_dev, DW9790_MSB_ADDR, data, 2);
    if (ret < 0)
        dev_err(dw9790_dev->sd.dev, "%s set ctrl failed\n", __func__);
    else
        dw9790_dev->cur_pos = len_pos;
    return ret;
}

static int dw9790_get_pos(struct dw9790_device *dw9790_dev, struct vvfocus_pos_s *ppos)
{
    ppos->pos = dw9790_dev->cur_pos;
    ppos->mode = VVFOCUS_MODE_ABSOLUTE;
    return 0;
}

static long dw9790_priv_ioctl(struct v4l2_subdev *sd,
                              unsigned int cmd,
                              void *arg)
{
    struct dw9790_device *dw9790_dev = sd_to_dw9790_device(sd);
    long ret = -1;
    struct vvfocus_reg_s focus_reg;
    struct vvfocus_range_s focus_range;
    struct vvfocus_pos_s focus_pos;

    if (!arg)
        return -ENOMEM;

    mutex_lock(&dw9790_dev->lock);
        switch(cmd) {
        case VIDIOC_QUERYCAP:
            ret = dw9790_query_capability(dw9790_dev, arg);
            break;
        case VVFOCUSIOC_GET_RANGE:
            ret = dw9790_get_range(dw9790_dev, &focus_range);
            ret |= copy_to_user(arg, &focus_range, sizeof(struct vvfocus_range_s));
            break;
        case VVFOCUSIOC_GET_POS:
            ret = dw9790_get_pos(dw9790_dev, &focus_pos);
            ret |= copy_to_user(arg, &focus_pos, sizeof(struct vvfocus_pos_s));
            break;
        case VVFOCUSIOC_SET_POS:
            ret = copy_from_user(&focus_pos, arg, sizeof(struct vvfocus_pos_s));
            ret |= dw9790_set_pos(dw9790_dev, &focus_pos);
            break;
        case VVFOCUSIOC_GET_REG:
            ret = copy_from_user(&focus_reg, arg, sizeof(struct vvfocus_reg_s));
            ret |= dw9790_i2c_read(dw9790_dev, focus_reg.addr, (uint8_t *)&focus_reg.value);
            ret |= copy_to_user(arg, &focus_reg, sizeof(struct vvfocus_reg_s));
            break;
        case VVFOCUSIOC_SET_REG:
            ret = copy_from_user(&focus_reg, arg, sizeof(struct vvfocus_reg_s));
            ret |= dw9790_i2c_write(dw9790_dev, focus_reg.addr, (uint8_t *)&focus_reg.value, 1);
            break;
        default:
            ret = -1;
            break;
        }
    mutex_unlock(&dw9790_dev->lock);

    return ret;
}

static struct v4l2_subdev_core_ops adw9790_core_ops = {
	.ioctl = dw9790_priv_ioctl,
};

static const struct v4l2_subdev_ops dw9790_ops = {
    .core  = &adw9790_core_ops,
};

static int dw9790_set_ctrl(struct v4l2_ctrl *ctrl)
{
    struct dw9790_device *dw9790_dev = ctrl_to_dw9790_device(ctrl);
    struct vvfocus_pos_s focus_pos;
    int ret = 0;
    if (ctrl->id == V4L2_CID_FOCUS_ABSOLUTE) {
        focus_pos.mode = VVFOCUS_MODE_ABSOLUTE;
        focus_pos.pos  = ctrl->val;
        ret = dw9790_set_pos(dw9790_dev, &focus_pos);
        return ret;
    }
    return -EINVAL;
}

static int dw9790_get_ctrl(struct v4l2_ctrl *ctrl)
{
    struct dw9790_device *dw9790_dev = ctrl_to_dw9790_device(ctrl);

    if (ctrl->id == V4L2_CID_FOCUS_ABSOLUTE) {
        ctrl->val = dw9790_dev->cur_pos;
        return 0;
    }
    return -EINVAL;
}

static const struct v4l2_ctrl_ops dw9790_vcm_ctrl_ops = {
    .s_ctrl = dw9790_set_ctrl,
    .g_volatile_ctrl = dw9790_get_ctrl,
};

static int dw9790_init_controls(struct dw9790_device *dw9790_dev)
{
    struct v4l2_ctrl_handler *handler = &dw9790_dev->ctrls_vcm;
    const struct v4l2_ctrl_ops *ops = &dw9790_vcm_ctrl_ops;
    v4l2_ctrl_handler_init(handler, 1);
    dw9790_dev->focus = v4l2_ctrl_new_std(handler, ops, V4L2_CID_FOCUS_ABSOLUTE,
		DW9790_MIN_FOCUS_POS, DW9790_MAX_FOCUS_POS, DW9790_FOCUS_STEPS, DW9790_FOCUS_DEF);
    if (handler->error)
        dev_err(dw9790_dev->sd.dev, "%s fail error: 0x%x\n",__func__, handler->error);
    dw9790_dev->sd.ctrl_handler = handler;

    return handler->error;
}

static int dw9790_init(struct dw9790_device *dw9790_dev)
{
    u8 reg_val = 0x00;
    if (dw9790_i2c_write(dw9790_dev, DW9790_INIT_ADDR, &reg_val, 1) < 0) {
        dev_err(dw9790_dev->sd.dev, "%s Init dw9790 failed\n", __func__);
        return -1;
    }
    return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 3, 0)
static int dw9790_probe(struct i2c_client *client)
#else
static int dw9790_probe(struct i2c_client *client,
                        const struct i2c_device_id *id)
#endif
{
    struct dw9790_device *dw9790_dev;
    struct device *dev = &client->dev;
    int ret;

    dw9790_dev = devm_kzalloc(&client->dev, sizeof(*dw9790_dev),
				  GFP_KERNEL);
    if (!dw9790_dev)
        return -ENOMEM;

    ret = of_property_read_u32(dev->of_node, "id", &dw9790_dev->id);
    if (ret) {
        dev_err(dev, "lens-focus id missing or invalid\n");
        return ret;
    }
    memcpy(dw9790_dev->name, dev->of_node->name, strlen(dev->of_node->name));

    v4l2_i2c_subdev_init(&dw9790_dev->sd, client, &dw9790_ops);
    dw9790_dev->sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	dw9790_dev->sd.internal_ops = &dw9790_int_ops;
	dw9790_dev->sd.entity.function = MEDIA_ENT_F_LENS;

    ret = dw9790_init_controls(dw9790_dev);
    if (ret < 0)
        goto err_cleanup;

    ret = media_entity_pads_init(&dw9790_dev->sd.entity, 0, NULL);
	if (ret < 0)
        goto err_cleanup;

    ret = v4l2_async_register_subdev(&dw9790_dev->sd);
    if (ret < 0)
        goto err_cleanup;

    pm_runtime_enable(&client->dev);

    ret = dw9790_init(dw9790_dev);
    if (ret < 0) {
        dev_err(&client->dev, "%s failed to power on dw9790 %d\n", __func__, ret);
        goto err_cleanup;
    }
    mutex_init(&dw9790_dev->lock);

    return 0;

err_cleanup:
    v4l2_ctrl_handler_free(&dw9790_dev->ctrls_vcm);
    media_entity_cleanup(&dw9790_dev->sd.entity);
    return ret;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
static int dw9790_remove(struct i2c_client *client) 
#else
static void dw9790_remove(struct i2c_client *client)
#endif
{
    struct v4l2_subdev *sd = i2c_get_clientdata(client);
    struct dw9790_device *dw9790_dev = sd_to_dw9790_device(sd);

    v4l2_async_unregister_subdev(&dw9790_dev->sd);
    v4l2_ctrl_handler_free(&dw9790_dev->ctrls_vcm);
    media_entity_cleanup(&dw9790_dev->sd.entity);

    pm_runtime_disable(&client->dev);
    mutex_destroy(&dw9790_dev->lock);
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
    return 0;
#else
#endif
}

static int __maybe_unused dw9790_vcm_suspend(struct device *dev) 
{
    return 0;
}

static int __maybe_unused dw9790_vcm_resume(struct device *dev)
{
    return 0;
}

static int __maybe_unused dw9790_vcm_runtime_suspend(struct device *dev)
{
    return 0;
}

static int __maybe_unused dw9790_vcm_runtime_resume(struct device *dev)
{
    return 0;
}

static const struct of_device_id dw9790_of_table[] = {
    { .compatible = "dw9790" },
    { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, dw9790_of_table);

static const struct dev_pm_ops dw9790_pm_ops = {
    SET_SYSTEM_SLEEP_PM_OPS(dw9790_vcm_suspend, dw9790_vcm_resume)
    SET_RUNTIME_PM_OPS(dw9790_vcm_runtime_suspend, dw9790_vcm_runtime_resume, NULL)
};

static struct i2c_driver dw9790_i2c_driver = {
    .driver = {
		.owner = THIS_MODULE,
		.name  = "dw9790",
		.pm = &dw9790_pm_ops,
		.of_match_table	= dw9790_of_table,
	},
	.probe  = dw9790_probe,
	.remove = dw9790_remove,
};


module_i2c_driver(dw9790_i2c_driver);

MODULE_DESCRIPTION("DW9790 vcm Driver");
MODULE_LICENSE("GPL");
