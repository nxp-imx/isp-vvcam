#include <linux/module.h>
#include <linux/uaccess.h>
#include "soc_ioctl.h"
#include "vsi_core_gen6.h"

 int gen6_write_reg(void* dev,unsigned int addr,unsigned int val)
{
	struct vvcam_soc_dev *soc_dev;

	soc_dev = (struct vvcam_soc_dev *)dev;

	if (soc_dev == NULL)
		return -1;
		
	writel(val,soc_dev->base + addr);
	
	return 0;
}

 int gen6_read_reg(void* dev,unsigned int addr,unsigned int *val)
{
	struct vvcam_soc_dev *soc_dev;

	soc_dev = (struct vvcam_soc_dev *)dev;

	if (soc_dev == NULL)
		return -1;
		
	*val = readl(soc_dev->base + addr);
	
	return 0;
}


static int gen6_set_isp_power(void* dev,unsigned int id,unsigned int status)
{
	return 0;
}

static int gen6_get_isp_power(void* dev,unsigned int id,unsigned int *status)
{
	return 0;
}

static int gen6_set_isp_reset(void* dev,unsigned int id,unsigned int status)
{
	unsigned int ret = 0;
	struct vvcam_soc_dev *soc_dev;;
	unsigned int reg_addr = 0;
	unsigned int reg_value;

	soc_dev = (struct vvcam_soc_dev *)dev;
	
	if (soc_dev == NULL)
		return -1;
		
	if (id == 0)
	{
		reg_addr = REG_TPG0;
	}else{
		reg_addr = REG_TPG1;
	}
	
	ret = soc_dev->soc_access.read(dev,reg_addr,&reg_value);
	if (status == 0)
	{
		reg_value &= ~(1<<24);
		
	}else{
		reg_value |=  (1<<24);
	}

	ret |= soc_dev->soc_access.write(dev,reg_addr,reg_value);
	ret |= soc_dev->soc_access.write(dev,REG_TPG0,reg_value);
	
	return ret;
}

static int gen6_set_isp_clk(void* dev,unsigned int id,unsigned int freq)
{
	return 0;
}

static int gen6_get_isp_clk(void* dev,unsigned int id,unsigned int *freq)
{
	return 0;
}

static int gen6_set_dwe_power(void* dev,unsigned int id,unsigned int status)
{
	return 0;
}

static int gen6_get_dwe_power(void* dev, unsigned int id,unsigned int *status)
{
	return 0;
}

static int gen6_set_dwe_reset(void* dev,unsigned int id,unsigned int status)
{
	unsigned int ret = 0;
	struct vvcam_soc_dev *soc_dev;;
	unsigned int reg_addr = 0;
	unsigned int reg_value;

	soc_dev = (struct vvcam_soc_dev *)dev;
	
	if (soc_dev == NULL)
		return -1;
		
	reg_addr = REG_DWE_CTRL;

	
	ret = soc_dev->soc_access.read(dev,reg_addr,&reg_value);
	if (status == 0)
	{
		reg_value &= ~(1<<0);
	}else{
		reg_value |=  (1<<0);
	}

	ret |= soc_dev->soc_access.write(dev,reg_addr,reg_value);

	return ret;
}

static int gen6_set_dwe_clk(void* dev,unsigned int id,unsigned int freq)
{
	return 0;
}

static int gen6_get_dwe_clk(void* dev,unsigned int id,unsigned int *freq)
{
	return 0;
}


static int gen6_set_vse_power(void* dev,unsigned int id,unsigned int status)
{
	return 0;
}

static int gen6_get_vse_power(void* dev,unsigned int id,unsigned int *status)
{
	return 0;
}

static int gen6_set_vse_reset(void* dev,unsigned int id,unsigned int status)
{
	unsigned int ret = 0;
	struct vvcam_soc_dev *soc_dev;;
	unsigned int reg_addr = 0;
	unsigned int reg_value;

	soc_dev = (struct vvcam_soc_dev *)dev;
	
	if (soc_dev == NULL)
		return -1;
		
	reg_addr = REG_VSE_CTRL;

	
	ret = soc_dev->soc_access.read(dev,reg_addr,&reg_value);
	if (status == 0)
	{
		reg_value &= ~(1<<0);
	}else{
		reg_value |=  (1<<0);
	}

	ret |= soc_dev->soc_access.write(dev,reg_addr,reg_value);

	return ret;
}

static int gen6_set_vse_clk(void* dev,unsigned int id,unsigned int freq)
{
	return 0;
}

static int gen6_get_vse_clk(void* dev,unsigned int id,unsigned int *freq)
{
	return 0;
}


static int gen6_set_csi_power(void* dev,unsigned int id,unsigned int status)
{
	return 0;
}

static int gen6_get_csi_power(void* dev,unsigned int id,unsigned int *status)
{
	return 0;
}

static int gen6_set_csi_reset(void* dev,unsigned int id,unsigned int status)
{
	unsigned int ret = 0;
	struct vvcam_soc_dev *soc_dev;;
	unsigned int reg_addr = 0;
	unsigned int reg_value;

	soc_dev = (struct vvcam_soc_dev *)dev;
	
	if (soc_dev == NULL)
		return -1;

	if (id == 0)
	{
		reg_addr = REG_TPG0;
	}else{
		reg_addr = REG_TPG1;
	}
	
	ret = soc_dev->soc_access.read(dev,reg_addr,&reg_value);
	if (status == 0)
	{
		reg_value &= ~(1<<4 | 1<<28);
	}else{
		reg_value |=  0x30000210;
	}
	ret |= soc_dev->soc_access.write(dev,reg_addr,reg_value);

	return ret;
}

static int gen6_set_csi_clk(void* dev,unsigned int id,unsigned int freq)
{
	return 0;
}

static int gen6_get_csi_clk(void* dev,unsigned int id,unsigned int *freq)
{
	return 0;
}

static int gen6_set_sensor_power(void* dev,unsigned int id,unsigned int status)
{
	return 0;
}

static int gen6_get_sensor_power(void* dev,unsigned int id,unsigned int *status)
{
	return 0;
}

static int gen6_set_sensor_reset(void* dev,unsigned int id,unsigned int status)
{
	return 0;
}

static int gen6_set_sensor_clk(void* dev,unsigned int id,unsigned int freq)
{
	return 0;
}

static int gen6_get_sensor_clk(void* dev,unsigned int id,unsigned int *freq)
{
	return 0;
}


struct vvcam_soc_function_s  gen6_soc_function = {
	.isp_func.set_power = gen6_set_isp_power,
	.isp_func.get_power = gen6_get_isp_power,
	.isp_func.set_reset = gen6_set_isp_reset,
	.isp_func.set_clk   = gen6_set_isp_clk,
	.isp_func.get_clk   = gen6_get_isp_clk,

	.dwe_func.set_power = gen6_set_dwe_power,
	.dwe_func.get_power = gen6_get_dwe_power,
	.dwe_func.set_reset = gen6_set_dwe_reset,
	.dwe_func.set_clk   = gen6_set_dwe_clk,
	.dwe_func.get_clk   = gen6_get_dwe_clk,

	.vse_func.set_power = gen6_set_vse_power,
	.vse_func.get_power = gen6_get_vse_power,
	.vse_func.set_reset = gen6_set_vse_reset,
	.vse_func.set_clk   = gen6_set_vse_clk,
	.vse_func.get_clk   = gen6_get_vse_clk,

	.csi_func.set_power = gen6_set_csi_power,
	.csi_func.get_power = gen6_get_csi_power,
	.csi_func.set_reset = gen6_set_csi_reset,
	.csi_func.set_clk   = gen6_set_csi_clk,
	.csi_func.get_clk   = gen6_get_csi_clk,

	.sensor_func.set_power = gen6_set_sensor_power,
	.sensor_func.get_power = gen6_get_sensor_power,
	.sensor_func.set_reset = gen6_set_sensor_reset,
	.sensor_func.set_clk   = gen6_set_sensor_clk,
	.sensor_func.get_clk   = gen6_get_sensor_clk,
};



