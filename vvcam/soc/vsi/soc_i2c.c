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
#include <asm/io.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/debugfs.h>
#include <linux/i2c.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/platform_device.h>

struct soc_gpio_i2c{
	struct i2c_adapter	adap;
	unsigned long  phy_base_addr;
	unsigned long  size;
	void __iomem * virt_base_addr;
	spinlock_t     lock;
};


static void i2c_gpio_setsda_val(void *source, int state)
{
	struct soc_gpio_i2c *i2c = source;
	int reg_data;

	reg_data = readl(i2c->virt_base_addr);
	reg_data = (reg_data & (~0x1)) | (0x10 | (state & 0x01));

	writel(reg_data, i2c->virt_base_addr);
	return ;
}

static void i2c_gpio_setscl_val(void *source, int state)
{
	struct soc_gpio_i2c *i2c = source;
	int reg_data;

	reg_data = readl(i2c->virt_base_addr);
	reg_data = (reg_data & (~(0x1 << 8))) | ((0x10 | (state & 0x01)) << 8);

	writel(reg_data, i2c->virt_base_addr);
	return ;
}

static int i2c_gpio_getsda(void *source)
{
	struct soc_gpio_i2c *i2c = source;
	unsigned int reg_data;

	reg_data = readl(i2c->virt_base_addr);
	reg_data = (reg_data & (~0x10));
	writel(reg_data, i2c->virt_base_addr);

	return (readl(i2c->virt_base_addr) & 0x01);
}
// Start :When SCL is High ,SDA Change to low
static void i2c_gpio_start(void *source)
{
	i2c_gpio_setscl_val(source, 1);
	i2c_gpio_setsda_val(source, 1);
	udelay(5);
	i2c_gpio_setsda_val(source, 0);
	udelay(1);
	return;
}
//Send Bit : When SCL is LOW, sda change;When SCL is HIgh,SDA hold;
static void i2c_gpio_send_byte(void *source, unsigned char val)
{
	int bit_idx;
	for (bit_idx=7; bit_idx >= 0; bit_idx--)
	{
		i2c_gpio_setscl_val(source, 0);
		i2c_gpio_setsda_val(source, (val >> bit_idx) & 0x01);
		udelay(1);
		i2c_gpio_setscl_val(source, 1);
		udelay(1);
	}
	i2c_gpio_setscl_val(source, 0);
	i2c_gpio_setsda_val(source, 1);
	return;
}
//Read Bit: When SCL is High read data;
static unsigned char i2c_gpio_read_byte(void *source)
{
	int bit_idx;
	unsigned char data = 0;

	i2c_gpio_getsda(source);//SDA change to read

	for(bit_idx=7; bit_idx>=0; bit_idx--)
	{

		i2c_gpio_setscl_val(source, 1);
		udelay(1);
		data = (data << 1) | i2c_gpio_getsda(source);
		i2c_gpio_setscl_val(source, 0);
		udelay(1);
	}
	return data;
}

static int i2c_gpio_wait_ack(void *source)
{
	unsigned int i2c_retry_ack_cnt = 0;

	i2c_gpio_getsda(source);//SDA change to read
	udelay(1);
	i2c_gpio_setscl_val(source, 1);

	while(i2c_gpio_getsda(source) == 1)
	{
		udelay(1);
		if (i2c_retry_ack_cnt++ > 20)
		{
			 return 0;
		}
	}
	i2c_gpio_setscl_val(source, 0);
	return 1;
}

static void i2c_gpio_stop(void *source)
{
	i2c_gpio_setsda_val(source, 0);
	udelay(1);
	i2c_gpio_setscl_val(source, 1);
	udelay(1);
	i2c_gpio_setsda_val(source, 1);
	return;
}

static int i2c_gpio_write(void *source,struct i2c_msg *msg)
{

	unsigned char slave_address;
	unsigned char *buf;
	int i;
	slave_address = (msg->addr) << 1;
	buf = msg->buf;

	i2c_gpio_start(source);
	i2c_gpio_send_byte(source, slave_address);
	if (i2c_gpio_wait_ack(source) == 0) return -1;

	for (i = 0; i < msg->len; i++)
	{
		i2c_gpio_send_byte(source, buf[i]);
		i2c_gpio_wait_ack(source);
	}

	i2c_gpio_stop(source);
	return 0;
}

static int i2c_gpio_read(void *source,struct i2c_msg *msg)
{
	unsigned char slave_address;
	unsigned char *buf;
	int i;
	slave_address = ((msg->addr) << 1) | 0x01;
	buf = msg->buf;

	i2c_gpio_start(source);
	i2c_gpio_send_byte(source, slave_address);
	if (i2c_gpio_wait_ack(source) == 0) return -1;

	for (i = 0; i < msg->len; i++)
	{
		buf[i] = i2c_gpio_read_byte(source);
		i2c_gpio_wait_ack(source);
	}

	return 0;
}

static int i2c_gpio_xfer(struct i2c_adapter *adapter,
						struct i2c_msg *msgs, int num)
{
	int result;
	int i;
	struct soc_gpio_i2c *i2c;
	void *source;

	source = i2c_get_adapdata(adapter);
	i2c = source;

	for (i=0; i<num; i++)
	{
		if (msgs[i].flags & I2C_M_RD)
		{
			result = i2c_gpio_read(source, &(msgs[i]));
		}else
		{
			result = i2c_gpio_write(source, &(msgs[i]));
		}

		if (result < 0)
			goto fail0;

	}



fail0:
	return (result < 0) ? result : num;
}

static u32 i2c_gpio_func(struct i2c_adapter *adapter)
{
	return I2C_FUNC_I2C | (I2C_FUNC_SMBUS_EMUL & ~I2C_FUNC_SMBUS_QUICK) |
	       I2C_FUNC_PROTOCOL_MANGLING;
}



static const struct i2c_algorithm gpio_i2c_algo = {
	.master_xfer	= i2c_gpio_xfer,
	.functionality	= i2c_gpio_func,
};


static int soc_gpio_i2c_probe(struct platform_device *pdev)
{
	int ret;
	struct soc_gpio_i2c *i2c;
	struct i2c_adapter *adap;
	struct resource		*mem;
	pr_info("%s in\n", __func__);

	i2c = devm_kzalloc(&pdev->dev, sizeof(struct soc_gpio_i2c), GFP_KERNEL);
	if (!i2c)
	{
		return -ENOMEM;
	}
	memset(i2c, 0, sizeof(struct soc_gpio_i2c));
	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);

	i2c->phy_base_addr  = mem->start;
	i2c->size           = mem->end - mem->start + 1;
	//i2c->virt_base_addr = devm_ioremap_resource(&pdev->dev, mem);
	//if (IS_ERR(i2c->virt_base_addr))
	//	return PTR_ERR(i2c->virt_base_addr);
	i2c->virt_base_addr = (void __iomem *)mem->name;

	spin_lock_init(&(i2c->lock));

	adap = &i2c->adap;
	platform_set_drvdata(pdev, adap);

	i2c_set_adapdata(adap, i2c);
	adap->owner = THIS_MODULE;
	snprintf(adap->name, sizeof(adap->name), "soc_gpio_i2c");
	adap->timeout = 2 * HZ;
	adap->retries = 0;
	adap->algo = &gpio_i2c_algo;
	adap->class = I2C_CLASS_HWMON | I2C_CLASS_SPD;
	adap->dev.parent = &pdev->dev;
	adap->nr		= pdev->id;
	adap->dev.of_node = pdev->dev.of_node;

	ret = i2c_add_numbered_adapter(adap);
	if (ret)
	{
		pr_info("%s:i2c_add_adapter failed 0x%x\n", __func__, ret);
		kfree(i2c);
		return ret;
	}

	return 0;
}

static int soc_gpio_i2c_remove(struct platform_device *pdev)
{
	struct i2c_adapter *adap;

	adap = platform_get_drvdata(pdev);
	i2c_del_adapter(adap);

	return 0;
}

static struct platform_driver soc_gpio_i2c_driver = {
	.probe =  soc_gpio_i2c_probe,
	.remove = soc_gpio_i2c_remove,
	.driver = {
		.name = "soc_gpio_i2c",
		.owner = THIS_MODULE,
	},
};

static unsigned int i2c_driver_register_flag = 0;

int soc_gpio_i2c_driver_init(void)
{
	if (i2c_driver_register_flag == 0)
	{
		platform_driver_register(&soc_gpio_i2c_driver);
	}
	i2c_driver_register_flag++;
	return 0;
}
void soc_gpio_i2c_driver_exit(void)
{
	if (i2c_driver_register_flag == 1)
	{
		pr_info("enter %s\n", __func__);
		platform_driver_unregister(&soc_gpio_i2c_driver);
	}

	i2c_driver_register_flag--;
	return;
}






