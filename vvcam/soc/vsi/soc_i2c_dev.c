#include <linux/platform_device.h>


static void soc_gpio_i2c_release(struct device *dev)
{
	pr_info("enter %s\n", __func__);
	return;
}


static struct resource soc_gpio_i2c0_resource[] = {
	[0] = {
		.start = 0x00,
		.end   = 0x00,
		.flags = IORESOURCE_MEM,
	},
};

static struct platform_device soc_gpio_i2c0_pdev = {
	.name = "soc_gpio_i2c",
	.id   = -1,
	.resource = soc_gpio_i2c0_resource,
	.num_resources = 1,
	.dev.release = soc_gpio_i2c_release,
};

static struct resource soc_gpio_i2c1_resource[] = {
	[0] = {
		.start = 0x00,
		.end   = 0x00,
		.flags = IORESOURCE_MEM,
	},
};

static struct platform_device soc_gpio_i2c1_pdev = {
	.name = "soc_gpio_i2c",
	.id   = -1,
	.resource = soc_gpio_i2c1_resource,
	.num_resources = 1,
	.dev.release = soc_gpio_i2c_release,
};

int soc_gpio_i2c_register_bus(unsigned int index,unsigned int bus,unsigned long base_addr, unsigned int size,
								void * virt_base)
{
	int ret = 0;
	struct platform_device * i2c_pdev;
	pr_info("enter %s\n", __func__);

	switch(index)
	{
		case 0:
			i2c_pdev = &soc_gpio_i2c0_pdev;
			break;
		case 1:
			i2c_pdev = &soc_gpio_i2c1_pdev;
			break;
		default :
			return -1;
	}

	i2c_pdev->id = bus;
	i2c_pdev->resource[0].start = base_addr;
	i2c_pdev->resource[0].end   = base_addr + size - 1;
	i2c_pdev->resource[0].name  = virt_base;

	ret = platform_device_register(i2c_pdev);

	pr_info("exit %s\n", __func__);
	return ret;
}

void soc_gpio_i2c_unregister_bus(unsigned int index)
{
	struct platform_device * i2c_pdev;

	switch(index)
	{
		case 0:
			i2c_pdev = &soc_gpio_i2c0_pdev;
			break;
		case 1:
			i2c_pdev = &soc_gpio_i2c1_pdev;
			break;
		default :
			return;
	}

	platform_device_unregister(i2c_pdev);

	return ;
}

