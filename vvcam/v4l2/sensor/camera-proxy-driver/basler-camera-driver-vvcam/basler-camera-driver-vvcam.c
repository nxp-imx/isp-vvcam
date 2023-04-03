/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/ctype.h>
#include <linux/device.h>
#include <linux/of_graph.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/regulator/consumer.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/v4l2-mediabus.h>
#include <media/v4l2-async.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <media/v4l2-fwnode.h>
#include <media/v4l2-subdev.h>
#include "basler-camera-driver-vvcam.h"
#include "vvsensor.h"

/*global variable*/
static struct register_access ra_tmp;


/* compact name as v4l2_capability->driver is limited to 16 characters */
#ifdef CONFIG_BASLER_CAMERA_VVCAM
#define SENSOR_NAME "basler-vvcam"
#else
#define SENSOR_NAME "basler-camera"
#endif


/*
 * ABRM register offsets
 *
 */
#define ABRM_GENCP_VERSION		0x0
#define ABRM_MANUFACTURER_NAME		0x4
#define ABRM_MODEL_NAME			0x44
#define ABRM_FAMILY_NAME		0x84
#define ABRM_DEVICE_VERSION		0xC4
#define ABRM_MANUFACTURER_INFO		0x104
#define ABRM_SERIAL_NUMBER		0x144
#define ABRM_USER_DEFINED_NAME		0x184
#define ABRM_DEVICE_CAPABILITIES	0x1C4

/*
 * ABRM register bits
 *
 */
#define ABRM_DEVICE_CAPABILITIES_USER_DEFINED_NAMES_SUPPORT	0x1
#define ABRM_DEVICE_CAPABILITIES_STRING_ENCODING		0x0f
#define ABRM_DEVICE_CAPABILITIES_FAMILY_NAME			0x100


/*
 * Maximum read i2c burst
 *
 * TODO: To be replace by a register call of SBRM
 *
 */
#define I2C_MAXIMUM_READ_BURST	8



static int basler_read_register_chunk(struct i2c_client* client, __u8* buffer, __u8 buffer_size, __u16 register_address);

static int basler_camera_s_ctrl(struct v4l2_ctrl *ctrl);
static int basler_camera_g_volatile_ctrl(struct v4l2_ctrl *ctrl);

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
static int basler_camera_validate(const struct v4l2_ctrl *ctrl, u32 idx, union v4l2_ctrl_ptr ptr);
#else
static int basler_camera_validate(const struct v4l2_ctrl *ctrl, union v4l2_ctrl_ptr ptr);
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
static void basler_camera_init(const struct v4l2_ctrl *ctrl, u32 idx, union v4l2_ctrl_ptr ptr);
#else
static void basler_camera_init(const struct v4l2_ctrl *ctrl, u32 tot_elems, union v4l2_ctrl_ptr ptr);
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
static bool basler_camera_equal(const struct v4l2_ctrl *ctrl, u32 idx, union v4l2_ctrl_ptr ptr1, union v4l2_ctrl_ptr ptr2);
#else
static bool basler_camera_equal(const struct v4l2_ctrl *ctrl, union v4l2_ctrl_ptr ptr1, union v4l2_ctrl_ptr ptr2);
#endif


struct basler_camera_dev {
	struct i2c_client *i2c_client;
	struct v4l2_device  *v4l2_dev;
	struct v4l2_subdev sd;
	struct media_pad pad;

	/* lock to protect all members below */
	struct mutex lock;

	int power_count;
	struct v4l2_ctrl_handler ctrl_handler;

	struct basler_device_information device_information;

	int csi;
};

/**
 * basler_write_burst - issue a burst I2C message in master transmit mode
 * @client: Handle to slave device
 * @ra_p: Data structure that hold the register address and data that will be written to the slave
 *
 * Returns negative errno, or else the number of bytes written.
 */
static int basler_write_burst(struct i2c_client *client,
			      struct register_access *ra_p)
{
	int ret;
	__u16 old_address;

	if (ra_p->data_size > sizeof(ra_p->data)){
		dev_err(&client->dev, "i2c burst array too big, max allowed %lu, got %d\n", sizeof(ra_p->data), ra_p->data_size);
		return -EINVAL;
	}

	old_address = ra_p->address;
	ra_p->address = cpu_to_be16(ra_p->address);

	if (I2CREAD == (ra_p->command | I2CREAD)){
		ra_tmp.address = ra_p->address;
		ra_tmp.data_size = ra_p->data_size;
		old_address = ra_p->address;
		return ra_p->data_size;
	}
	else if(I2CWRITE == (ra_p->command | I2CWRITE)){
		ret = i2c_master_send(client, (char *)ra_p, ra_p->data_size + sizeof(ra_p->address));

		if(ret)
			ra_p->data_size = ret;

		old_address = ra_p->address;
		return ret;
	}
	else
		return -EPERM;
}

/**
 * basler_read_burst - issue a burst I2C message in master transmit mode
 * @client: Handle to slave device
 * @ra_p: Data structure store the data read from slave
 *
 * Note: Before data can read use basler_write_burst with read command
 *       to send the register address
 *
 * Returns negative errno, or else the number of bytes written.
 */
static int basler_read_burst(struct i2c_client *client,
		struct register_access *ra_p)
{
	int ret;

	ret = basler_read_register_chunk(client, ra_p->data, ra_tmp.data_size,
									ra_tmp.address);
	if (ret < 0)
		ra_p->data_size = 0;
	else
		ra_p->data_size = ret;

	return ret;
}


static int basler_read_register_chunk(struct i2c_client* client, __u8* buffer, __u8 buffer_size, __u16 register_address)
{
	struct i2c_msg msgs[2] = {};
	int ret = 0;

	msgs[0].addr = client->addr;
	msgs[0].flags = 0;
	msgs[0].buf = (__u8 *)&register_address;
	msgs[0].len = sizeof(register_address);

	msgs[1].addr = client->addr;
	msgs[1].flags = I2C_M_RD;
	msgs[1].buf = buffer;
	msgs[1].len = buffer_size;

	ret = i2c_transfer(client->adapter, msgs, 2);
	if (ret < 0) {
		pr_err("i2c_transfer() failed: %d\n", ret);
		return ret;
	}

	if (ret != 2) {
		pr_err("i2c_transfer() incomplete");
		return -EIO;
	}

	return msgs[1].len;
}

static int basler_read_register(struct i2c_client* client, __u8* buffer, __u8 buffer_size, __u16 register_address)
{
	int ret = 0;
	__u8 l_read_bytes = 0;

	do {
		__be16 l_register_address = cpu_to_be16(register_address + l_read_bytes);

		ret = basler_read_register_chunk(client, (__u8*) buffer + l_read_bytes, (__u8) min(I2C_MAXIMUM_READ_BURST, ((int)buffer_size - l_read_bytes)), l_register_address);
		if (ret < 0)
		{
			pr_err("basler_read_register_chunk() failed: %d\n", ret);
			return ret;
		}
		else if (ret == 0)
		{
			pr_err("basler_read_register_chunk() read 0 bytes.\n");
			return -EIO;
		}

		l_read_bytes = l_read_bytes + ret;
	} while (l_read_bytes < buffer_size);

	return l_read_bytes;
}


static int basler_retrieve_device_information(struct i2c_client* client, struct basler_device_information* bdi)
{
	int ret = 0;
	__u64 deviceCapabilities = 0;
	__be64 deviceCapabilitiesBe = 0;
	__u32 gencpVersionBe = 0;

	bdi->_magic = BDI_MAGIC;

	ret = basler_read_register(client, (__u8*) &deviceCapabilitiesBe, sizeof(deviceCapabilitiesBe), ABRM_DEVICE_CAPABILITIES);
	if (ret < 0)
	{
		pr_err("basler_read_register() failed: %d\n", ret);
		return ret;
	}
	else if (ret == sizeof(deviceCapabilitiesBe))
	{
		deviceCapabilities = be64_to_cpu(deviceCapabilitiesBe);
		pr_debug("deviceCapabilities = 0x%llx\n", deviceCapabilities);
		pr_debug("String Encoding = 0x%llx\n", (deviceCapabilities & ABRM_DEVICE_CAPABILITIES_STRING_ENCODING) >> 4);
	}
	else {
		pr_err("basler_read_register() not read full buffer = %d bytes\n", ret);
		return -EIO;
	}

	ret = basler_read_register(client, (__u8*) &gencpVersionBe, sizeof(gencpVersionBe), ABRM_GENCP_VERSION);
	if (ret < 0)
	{
		pr_err("basler_read_register() failed: %d\n", ret);
		return ret;
	}
	else if (ret == sizeof(gencpVersionBe))
	{
		bdi->gencpVersion = be32_to_cpu(gencpVersionBe);
		pr_debug("l_gencpVersion = %d.%d\n", (bdi->gencpVersion & 0xffff0000) >> 16, bdi->gencpVersion & 0xffff);
	}
	else {
		pr_err("basler_read_register() not read full buffer = %d bytes\n", ret);
		return -EIO;
	}

	ret = basler_read_register(client, bdi->deviceVersion, GENCP_STRING_BUFFER_SIZE, ABRM_DEVICE_VERSION);
	if (ret < 0)
	{
		pr_err("basler_read_register() failed: %d\n", ret);
		return ret;
	}
	else if (ret == GENCP_STRING_BUFFER_SIZE)
	{
		pr_debug("bdi->deviceVersion = %s\n", bdi->deviceVersion);
	}
	else {
		pr_err("basler_read_register() not read full buffer = %d bytes\n", ret);
		return -EIO;
	}

	ret = basler_read_register(client, bdi->serialNumber, GENCP_STRING_BUFFER_SIZE, ABRM_SERIAL_NUMBER);
	if (ret < 0)
	{
		pr_err("basler_read_register() failed: %d\n", ret);
		return ret;
	}
	else if (ret == GENCP_STRING_BUFFER_SIZE)
	{
		pr_debug("bdi->serialNumber = %s\n", bdi->serialNumber);
	}
	else {
		pr_err("basler_read_register() not read full buffer = %d bytes\n", ret);
		return -EIO;
	}

	ret = basler_read_register(client, bdi->manufacturerName, GENCP_STRING_BUFFER_SIZE, ABRM_MANUFACTURER_NAME);
	if (ret < 0)
	{
		pr_err("basler_read_register() failed: %d\n", ret);
		return ret;
	}
	else if (ret == GENCP_STRING_BUFFER_SIZE)
	{
		pr_debug("bdi->manufacturerName = %s\n", bdi->manufacturerName);
	}
	else {
		pr_err("basler_read_register() not read full buffer = %d bytes\n", ret);
		return -EIO;
	}

	ret = basler_read_register(client, bdi->modelName, GENCP_STRING_BUFFER_SIZE, ABRM_MODEL_NAME);
	if (ret < 0)
	{
		pr_err("basler_read_register() failed: %d\n", ret);
		return ret;
	}
	else if (ret == GENCP_STRING_BUFFER_SIZE)
	{
		pr_debug("bdi->modelName = %s\n", bdi->modelName);
	}
	else {
		pr_err("basler_read_register() not read full buffer = %d bytes\n", ret);
		return -EIO;
	}

	if (deviceCapabilities & ABRM_DEVICE_CAPABILITIES_FAMILY_NAME)
	{
		ret = basler_read_register(client, bdi->familyName, GENCP_STRING_BUFFER_SIZE, ABRM_FAMILY_NAME);
		if (ret < 0)
		{
			pr_err("basler_read_register() failed: %d\n", ret);
			return ret;
		}
		else if (ret == GENCP_STRING_BUFFER_SIZE)
		{
			pr_debug("bdi->familyName = %s\n", bdi->familyName);
		}
		else {
			pr_err("basler_read_register() not read full buffer = %d bytes\n", ret);
			return -EIO;
		}
	}
	else
		pr_notice("ABRM FamilyName not supported\n");

	if (deviceCapabilities & ABRM_DEVICE_CAPABILITIES_USER_DEFINED_NAMES_SUPPORT)
	{
		ret = basler_read_register(client, bdi->userDefinedName, GENCP_STRING_BUFFER_SIZE, ABRM_USER_DEFINED_NAME);
		if (ret < 0)
		{
			pr_err("basler_read_register() failed: %d\n", ret);
			return ret;
		}
		else if (ret == GENCP_STRING_BUFFER_SIZE)
		{
			pr_debug("bdi->userDefinedName = %s\n", bdi->userDefinedName);
		}
		else {
			pr_err("basler_read_register() not read full buffer = %d bytes\n", ret);
			return -EIO;
		}
	}
	else
		pr_notice("ABRM UserDefinedName not supported\n");

	ret = basler_read_register(client, bdi->manufacturerInfo, GENCP_STRING_BUFFER_SIZE, ABRM_MANUFACTURER_INFO);
	if (ret < 0)
	{
		pr_err("basler_read_register() failed: %d\n", ret);
		return ret;
	}
	else if (ret == GENCP_STRING_BUFFER_SIZE)
	{
		pr_debug("bdi->manufacturerInfo = %s\n", bdi->manufacturerInfo);
	}
	else {
		pr_err("basler_read_register() not read full buffer = %d bytes\n", ret);
		return -EIO;
	}

	/*
	 * If the strings are in ASCII - print it.
	 */
	if (((deviceCapabilities & ABRM_DEVICE_CAPABILITIES_STRING_ENCODING) >> 4) == 0)
	{
		pr_info("ABRM: Manufacturer: %s, Model: %s, Device: %s, Serial: %s\n", bdi->manufacturerName, bdi->modelName, bdi->deviceVersion, bdi->serialNumber);
	}

	return 0;
}

static int basler_retrieve_csi_information(struct basler_camera_dev *sensor,
					   struct basler_csi_information* bci)
{
	struct v4l2_fwnode_endpoint bus_cfg = { .bus_type = V4L2_MBUS_CSI2_DPHY };
	struct device *dev = &sensor->i2c_client->dev;
	struct device_node *ep;
	int ret;

	/* We need a function that searches for the device that holds
	 * the csi-2 bus information. For now we put the bus information
	 * also into the sensor endpoint itself.
	 */
	ep = of_graph_get_next_endpoint(dev->of_node, NULL);
	if (!ep) {
		dev_err(dev, "missing endpoint node\n");
		return -ENODEV;
	}

        ret = v4l2_fwnode_endpoint_alloc_parse(of_fwnode_handle(ep), &bus_cfg);
	of_node_put(ep);
	if (ret) {
		dev_err(dev, "failed to parse endpoint\n");
		return ret;
	}

	if (bus_cfg.bus_type != V4L2_MBUS_CSI2_DPHY ||
	    bus_cfg.bus.mipi_csi2.num_data_lanes == 0 ||
	    bus_cfg.nr_of_link_frequencies == 0) {
		dev_err(dev, "missing CSI-2 properties in endpoint\n");
		ret = -ENODATA;
	} else {
		int i;
		bci->max_lane_frequency = bus_cfg.link_frequencies[0];
		bci->lane_count = bus_cfg.bus.mipi_csi2.num_data_lanes;
		for (i = 0; i < bus_cfg.bus.mipi_csi2.num_data_lanes; ++i) {
			bci->lane_assignment[i] = bus_cfg.bus.mipi_csi2.data_lanes[i];
		}
		ret = 0;
	}
	return ret;
}

static int basler_retrieve_capture_properties(struct basler_camera_dev *sensor,
					   struct basler_capture_properties* bcp)
{
	struct device *dev = &sensor->i2c_client->dev;
	__u64 mlf = 0;
	__u64 mpf = 0;
	__u64 mdr = 0;
	struct device_node *ep;

	int ret;

	/* Collecting the information about limits of capture path
	 * has been centralized to the sensor endpoint itself.
	 */
	ep = of_graph_get_next_endpoint(dev->of_node, NULL);
	if (!ep) {
		dev_err(dev, "missing endpoint node\n");
		return -ENODEV;
	}

	ret = fwnode_property_read_u64(of_fwnode_handle(ep),
		"max-lane-frequency", &mlf);
	if (ret || mlf == 0) {
		dev_dbg(dev, "no limit for max-lane-frequency\n");
		/* reset possible read error */
		ret = 0;
	}

	ret = fwnode_property_read_u64(of_fwnode_handle(ep),
		"max-pixel-frequency", &mpf);
#ifdef CONFIG_BASLER_CAMERA_VVCAM
	/* max-pixel-frequency is mandatory for vvcam */
	if (ret) {
		dev_err(dev, "failed to parse endpoint: max-pixel-frequency missing\n");
		return ret;
	}
#endif

	if (ret || mpf == 0) {
		dev_dbg(dev, "no limit for max-pixel-frequency\n");
		/* reset possible read error */
		ret = 0;
	}

	if (ret || mpf == 0) {
		dev_dbg(dev, "no limit for max-pixel-frequency\n");
		/* reset possible read error */
		ret = 0;
	}

	ret = fwnode_property_read_u64(of_fwnode_handle(ep),
		"max-data-rate", &mdr);
	if (ret || mdr == 0) {
		dev_dbg(dev, "no limit for max-data_rate\n");
		/* reset possible read error */
		ret = 0;
	}

	bcp->max_lane_frequency = mlf;
	bcp->max_pixel_frequency = mpf;
	bcp->max_data_rate = mdr;

	return ret;
}



static inline struct basler_camera_dev *to_basler_camera_dev(struct v4l2_subdev *sd)
{
	return container_of(sd, struct basler_camera_dev, sd);
}

static inline struct v4l2_subdev *ctrl_to_sd(struct v4l2_ctrl *ctrl)
{
	return &container_of(ctrl->handler, struct basler_camera_dev,
			     ctrl_handler)->sd;
}

/**
 * basler_camera_set_fmt - set format of the camera
 *
 * Note: Will be done in user space
 *
 * Returns always zero
 */
#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 12, 0)
static int basler_camera_set_fmt(struct v4l2_subdev *sd,
				 struct v4l2_subdev_state *state,
				 struct v4l2_subdev_format *format)
#else
static int basler_camera_set_fmt(struct v4l2_subdev *sd,
				 struct v4l2_subdev_pad_config *cfg,
				 struct v4l2_subdev_format *format)
#endif
{
	return 0;
}

/**
 * basler_camera_s_stream - start camera streaming
 *
 * Note: Will be done in user space
 *
 * Returns always zero
 */
static int basler_camera_s_stream(struct v4l2_subdev *sd, int enable)
{
	return 0;
}

static int basler_camera_s_power(struct v4l2_subdev *sd, int on)
{
	struct basler_camera_dev *sensor = to_basler_camera_dev(sd);

	mutex_lock(&sensor->lock);

	/* Update the power count. */
	sensor->power_count += on ? 1 : -1;
	WARN_ON(sensor->power_count < 0);

	mutex_unlock(&sensor->lock);

	return 0;
}

static int basler_ioc_qcap(struct basler_camera_dev *sensor, void *args)
{
	struct v4l2_capability *cap = (struct v4l2_capability *)args;
	strcpy((char *)cap->driver, SENSOR_NAME);

	sprintf((char *)cap->bus_info, "csi%d",sensor->csi); /* bus_info[0:7]-csi number */
	if(sensor->i2c_client->adapter)
	{
		/* bus_info[8]-i2c bus dev number */
		cap->bus_info[VVCAM_CAP_BUS_INFO_I2C_ADAPTER_NR_POS] = (__u8)sensor->i2c_client->adapter->nr;
	}
	else
	{
		cap->bus_info[VVCAM_CAP_BUS_INFO_I2C_ADAPTER_NR_POS] = 0xFF;
	}




	return 0;
}

/*
Use USER_TO_KERNEL/KERNEL_TO_USER to fix "uaccess" exception on run time.
Also, use "copy_ret" to fix the build issue as below.
error: ignoring return value of function declared with 'warn_unused_result' attribute.
*/

#ifdef CONFIG_HARDENED_USERCOPY
#define USER_TO_KERNEL(TYPE) \
	do {\
		TYPE tmp; \
		unsigned long copy_ret; \
		arg = (void *)(&tmp); \
		copy_ret = copy_from_user(arg, arg_user, sizeof(TYPE));\
	} while (0)

#define KERNEL_TO_USER(TYPE) \
	do {\
		unsigned long copy_ret; \
		copy_ret = copy_to_user(arg_user, arg, sizeof(TYPE));\
	} while (0)
#else
#define USER_TO_KERNEL(TYPE)
#define KERNEL_TO_USER(TYPE)
#endif


static long basler_camera_priv_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg_user)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct basler_camera_dev * sensor = to_basler_camera_dev(sd);
	void *arg = arg_user;

	int ret = -1;

	switch (cmd) {
	case VIDIOC_QUERYCAP:
		ret = basler_ioc_qcap(sensor, arg);
		break;

	case BASLER_IOC_G_INTERFACE_VERSION:
	{
		USER_TO_KERNEL(__u32);
		*((__u32*)arg) = (((__u32)BASLER_INTERFACE_VERSION_MAJOR) << 16) | BASLER_INTERFACE_VERSION_MINOR;
		KERNEL_TO_USER(__u32);
		ret = 0;
		break;
	}

	case BASLER_IOC_READ_REGISTER:
	{
		struct register_access *ra_p;
		USER_TO_KERNEL(struct register_access);
        ra_p = (struct register_access *)arg;
		ret = basler_read_register(client,
					   ra_p->data,
					   ra_p->data_size,
					   ra_p->address);
		if (ret < 0)
			ra_p->data_size = 0;
		else
			ra_p->data_size = ret;
		KERNEL_TO_USER(struct register_access);
	}
	break;

	case BASLER_IOC_WRITE_REGISTER:
	{
		struct register_access ra;
		struct register_access * ra_p;
		USER_TO_KERNEL(struct register_access);
        ra_p = (struct register_access *)arg;

		memcpy (&ra, ra_p, sizeof(ra));
		ra.address = cpu_to_be16(ra_p->address);

		ret = i2c_master_send(client, (char *)&ra, ra.data_size + sizeof(ra.address));
		if(ret) {
			ra_p->data_size = ret;
			ret = 0;
		}
		else
			ret = -EIO;
	}
	break;

	case BASLER_IOC_G_DEVICE_INFORMATION:
	{
		USER_TO_KERNEL(struct basler_device_information);
		memcpy((struct basler_device_information *)arg, &sensor->device_information, sizeof(struct basler_device_information));
		KERNEL_TO_USER(struct basler_device_information);
		ret = 0;
		break;
	}

	case BASLER_IOC_G_CSI_INFORMATION:
	{
		USER_TO_KERNEL(struct basler_csi_information);
		ret = basler_retrieve_csi_information(sensor, (struct basler_csi_information*)arg);
		KERNEL_TO_USER(struct basler_csi_information);
		break;
	}

	case BASLER_IOC_G_CAPTURE_PROPERTIES:
	{
		USER_TO_KERNEL(struct basler_capture_properties);
		ret = basler_retrieve_capture_properties(sensor, (struct basler_capture_properties*)arg);
		KERNEL_TO_USER(struct basler_capture_properties);
		break;
	}


	default:
		ret = -EINVAL;
		break;
	}
	return ret;
}

static const struct v4l2_subdev_core_ops basler_camera_core_ops = {
	.s_power = basler_camera_s_power,
	.ioctl = basler_camera_priv_ioctl,
};

static const struct v4l2_subdev_video_ops basler_camera_video_ops = {
	.s_stream = basler_camera_s_stream,
};

static const struct v4l2_subdev_pad_ops basler_camera_pad_ops = {
	.set_fmt = basler_camera_set_fmt,
};

static const struct v4l2_subdev_ops basler_camera_subdev_ops = {
	.core = &basler_camera_core_ops,
	.video = &basler_camera_video_ops,
	.pad = &basler_camera_pad_ops,
};

static const struct v4l2_ctrl_ops basler_camera_ctrl_ops = {
	.g_volatile_ctrl = basler_camera_g_volatile_ctrl,
	.s_ctrl = basler_camera_s_ctrl,
};

static const struct v4l2_ctrl_type_ops basler_camera_ctrl_type_ops = {
	.validate = basler_camera_validate,
	.init = basler_camera_init,
	.equal = basler_camera_equal,
};

/**
 * basler_camera_validate
 *
 * Note: Not needed by access-register control
 *
 * Returns always zero
 */

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
static int basler_camera_validate(const struct v4l2_ctrl *ctrl, u32 idx, union v4l2_ctrl_ptr ptr )
#else
static int basler_camera_validate(const struct v4l2_ctrl *ctrl, union v4l2_ctrl_ptr ptr)
#endif
{
	return 0;
}

/**
 * basler_camera_init
 *
 * Note: Not needed by access-register control
 *
 */

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
static void basler_camera_init(const struct v4l2_ctrl *ctrl, u32 idx, union v4l2_ctrl_ptr ptr )
#else
static void basler_camera_init(const struct v4l2_ctrl *ctrl, u32 tot_elems, union v4l2_ctrl_ptr ptr)
#endif
{
}
/**
 * basler_camera_equal
 *
 * Note: Not needed by access-register control
 *
 * Returns always zero
 */

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
static bool basler_camera_equal(const struct v4l2_ctrl *ctrl, u32 idx, union v4l2_ctrl_ptr ptr1, union v4l2_ctrl_ptr ptr2)
#else
static bool basler_camera_equal(const struct v4l2_ctrl *ctrl, union v4l2_ctrl_ptr ptr1, union v4l2_ctrl_ptr ptr2)
#endif
{
	return 0;
}

static const struct v4l2_ctrl_config ctrl_access_register = {
	.ops = &basler_camera_ctrl_ops,
	.type_ops = &basler_camera_ctrl_type_ops,
	.id = V4L2_CID_BASLER_ACCESS_REGISTER,
	.name = "basler-access-register",
	.type = V4L2_CTRL_TYPE_U32+1,
	.flags = V4L2_CTRL_FLAG_EXECUTE_ON_WRITE | V4L2_CTRL_FLAG_VOLATILE,
	.step = 1,
	.dims = {1},
	.elem_size = sizeof(struct register_access),
};

static const struct v4l2_ctrl_config ctrl_basler_device_information = {
	.ops = &basler_camera_ctrl_ops,
	.type_ops = &basler_camera_ctrl_type_ops,
	.id = V4L2_CID_BASLER_DEVICE_INFORMATION,
	.name = "basler-device-information",
	.type = V4L2_CTRL_TYPE_U32+1,
	.flags = V4L2_CTRL_FLAG_READ_ONLY | V4L2_CTRL_FLAG_VOLATILE,
	.step = 1,
	.dims = {1},
	.elem_size = sizeof(struct basler_device_information),
};

static const struct v4l2_ctrl_config ctrl_basler_interface_version = {
	.ops = &basler_camera_ctrl_ops,
	.type_ops = &basler_camera_ctrl_type_ops,
	.id = V4L2_CID_BASLER_INTERFACE_VERSION,
	.name = "basler-interface-version",
	.type = V4L2_CTRL_TYPE_INTEGER,
	.flags = V4L2_CTRL_FLAG_READ_ONLY,
	.min = 0x0,
	.max = 0xffffffff,
	.def = (BASLER_INTERFACE_VERSION_MAJOR << 16) | BASLER_INTERFACE_VERSION_MINOR,
	.step = 1,
};

static const struct v4l2_ctrl_config ctrl_basler_csi_information = {
	.ops = &basler_camera_ctrl_ops,
	.type_ops = &basler_camera_ctrl_type_ops,
	.id = V4L2_CID_BASLER_CSI_INFORMATION,
	.name = "basler-csi-information",
	.type = V4L2_CTRL_TYPE_U32+1,
	.flags = V4L2_CTRL_FLAG_READ_ONLY | V4L2_CTRL_FLAG_VOLATILE,
	.step = 1,
	.dims = {1},
	.elem_size = sizeof(struct basler_csi_information),
};

static int basler_camera_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct v4l2_subdev *sd = ctrl_to_sd(ctrl);
	struct basler_camera_dev *sensor = to_basler_camera_dev(sd);
	struct i2c_client *client = sensor->i2c_client;
	int ret;
	struct register_access *fp_ra_new;

	switch (ctrl->id) {
	case V4L2_CID_BASLER_ACCESS_REGISTER:

		if (ctrl->elem_size != sizeof(struct register_access))
			return -ENOMEM;

		fp_ra_new = (struct register_access*) ctrl->p_new.p;
		if(basler_write_burst(client, fp_ra_new))
			ret = 0;
		else
			ret = -EIO;

		break;

	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int basler_camera_g_volatile_ctrl(struct v4l2_ctrl *ctrl)
{
	struct v4l2_subdev *sd = ctrl_to_sd(ctrl);
	struct basler_camera_dev *sensor = to_basler_camera_dev(sd);
	struct i2c_client *client = sensor->i2c_client;
	int ret;
	struct register_access *fp_ra_new = NULL;
	struct basler_device_information* l_bdi = NULL;

	switch (ctrl->id) {
	case V4L2_CID_BASLER_ACCESS_REGISTER:

		fp_ra_new = (struct register_access*) ctrl->p_new.p;

		if (ctrl->elem_size == sizeof(struct register_access))
		{
			if(basler_read_burst(client, fp_ra_new))
				ret = 0;
			else
				ret = -EIO;
		}
		else {
			ret = -ENOMEM;
		}
		break;

	case V4L2_CID_BASLER_DEVICE_INFORMATION:

		l_bdi = (struct basler_device_information*) ctrl->p_new.p;

		if (ctrl->elem_size == sizeof(struct basler_device_information))
		{
			memcpy(l_bdi, &sensor->device_information, sizeof(struct basler_device_information));
			ret = 0;
		}
		else
		{
			ret = -ENOMEM;
		}
		break;

	case V4L2_CID_BASLER_CSI_INFORMATION:
		if (ctrl->elem_size == sizeof(struct basler_csi_information))
		{
			struct basler_csi_information* l_bci = NULL;
			l_bci = (struct basler_csi_information*) ctrl->p_new.p;
			ret = basler_retrieve_csi_information(sensor, l_bci);
		}
		else
		{
			ret = -ENOMEM;
		}
		break;

	case V4L2_CID_BASLER_CAPTURE_PROPERTIES:
		if (ctrl->elem_size == sizeof(struct basler_capture_properties))
		{
			struct basler_capture_properties* l_bcp = NULL;
			l_bcp = (struct basler_capture_properties*) ctrl->p_new.p;
			ret = basler_retrieve_capture_properties(sensor, l_bcp);
		}
		else
		{
			ret = -ENOMEM;
		}
		break;


	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}


/**
 * basler_camera_link_setup
 *
 * Note: Function is needed by imx8qm
 *
 * Returns always zero
 */
static int basler_camera_link_setup(struct media_entity *entity,
				    const struct media_pad *local,
				    const struct media_pad *remote, u32 flags)
{
	return 0;
}

static const struct media_entity_operations basler_camera_sd_media_ops = {
	.link_setup = basler_camera_link_setup,
};


static int basler_camera_init_controls(struct basler_camera_dev *sensor)
{
	struct v4l2_ctrl_handler *hdl = &sensor->ctrl_handler;
	int ret;

	v4l2_ctrl_handler_init(hdl, 32);

	/* we can use our own mutex for the ctrl lock */
	hdl->lock = &sensor->lock;

	v4l2_ctrl_new_custom(hdl, &ctrl_access_register, NULL);
	if (hdl->error)
	{
		dev_err(&sensor->i2c_client->dev, "Register ctrl access_register failed: %d\n", hdl->error);
		ret = hdl->error;
		goto free_ctrls;
	}

	v4l2_ctrl_new_custom(hdl, &ctrl_basler_device_information, NULL);
	if (hdl->error)
	{
		dev_err(&sensor->i2c_client->dev, "Register ctrl device_information failed: %d\n", hdl->error);
		ret = hdl->error;
		goto free_ctrls;
	}

	v4l2_ctrl_new_custom(hdl, &ctrl_basler_interface_version, NULL);
	if (hdl->error)
	{
		dev_err(&sensor->i2c_client->dev, "Register ctrl interface_version failed: %d\n", hdl->error);
		ret = hdl->error;
		goto free_ctrls;
	}

	v4l2_ctrl_new_custom(hdl, &ctrl_basler_csi_information, NULL);
	if (hdl->error)
	{
		dev_err(&sensor->i2c_client->dev, "Register ctrl csi_information failed: %d\n", hdl->error);
		ret = hdl->error;
		goto free_ctrls;
	}

	sensor->sd.ctrl_handler = hdl;
	return 0;

free_ctrls:
	v4l2_ctrl_handler_free(hdl);
	dev_dbg(sensor->sd.v4l2_dev->dev, "%s: ctrl handler error.\n", __func__);
	return ret;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 3, 0)
static int basler_camera_probe(struct i2c_client *client)
#else
static int basler_camera_probe(struct i2c_client *client,
			       const struct i2c_device_id *id)
#endif
{
	struct device *dev = &client->dev;
	struct basler_camera_dev *sensor;
	int ret;

	dev_dbg(dev, " %s driver start probing\n", SENSOR_NAME);

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(dev, "I2C_FUNC_I2C not supported\n");
		return -ENODEV;
	}

	sensor = devm_kzalloc(dev, sizeof(*sensor), GFP_KERNEL);
	if (!sensor)
		return -ENOMEM;

	sensor->i2c_client = client;

	ret = of_property_read_u32(dev->of_node, "csi_id", &(sensor->csi));
	if (ret) {
		dev_err(dev, "csi id missing or invalid\n");
		return ret;
	}

	v4l2_i2c_subdev_init(&sensor->sd, client, &basler_camera_subdev_ops);

	ret = basler_retrieve_device_information(client, &sensor->device_information);
	if (ret)
		return ret;

	sensor->sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	sensor->sd.entity.function = MEDIA_ENT_F_CAM_SENSOR;
	sensor->pad.flags = MEDIA_PAD_FL_SOURCE;

	sensor->sd.entity.ops = &basler_camera_sd_media_ops;
	ret = media_entity_pads_init(&sensor->sd.entity, 1, &sensor->pad);
	if (ret)
		return ret;

	mutex_init(&sensor->lock);

	ret = basler_camera_init_controls(sensor);
	if (ret)
		goto entity_cleanup;
#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 12, 0)
	ret = v4l2_async_register_subdev_sensor(&sensor->sd);
#else
	ret = v4l2_async_register_subdev_sensor_common(&sensor->sd);
#endif
	if (ret)
		goto entity_cleanup;

	dev_dbg(dev, " %s driver probed\n", SENSOR_NAME);
	return 0;

entity_cleanup:
	mutex_destroy(&sensor->lock);
	media_entity_cleanup(&sensor->sd.entity);
	return ret;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
static int basler_camera_remove(struct i2c_client *client)
#else
static void basler_camera_remove(struct i2c_client *client)
#endif
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct basler_camera_dev *sensor = to_basler_camera_dev(sd);

	v4l2_async_unregister_subdev(&sensor->sd);
	sensor->sd.ctrl_handler = NULL;
	v4l2_ctrl_handler_free(&sensor->ctrl_handler);
	mutex_destroy(&sensor->lock);
	media_entity_cleanup(&sensor->sd.entity);

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
	return 0;
#else
#endif
}

static const struct i2c_device_id basler_camera_id[] = {
	{ "basler-camera-vvcam", 0 },
	{ },
};
MODULE_DEVICE_TABLE(i2c, basler_camera_id);

static const struct of_device_id basler_camera_dt_ids[] = {
#ifdef CONFIG_BASLER_CAMERA_VVCAM
	{ .compatible = "basler,basler-camera-vvcam" },
#else
	{ .compatible = "basler,basler-camera" },
#endif
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, basler_camera_dt_ids);

static struct
#ifdef CONFIG_BASLER_CAMERA_VVCAM
    i2c_driver basler_camera_i2c_driver_vvcam
#else
    i2c_driver basler_camera_i2c_driver
#endif
   = {
	.driver = {
		.owner = THIS_MODULE,
#ifdef CONFIG_BASLER_CAMERA_VVCAM
		.name  = "basler-camera-vvcam",
#else
		.name  = "basler-camera",
#endif
		.of_match_table	= basler_camera_dt_ids,
	},
	.id_table = basler_camera_id,
	.probe    = basler_camera_probe,
	.remove   = basler_camera_remove,
};

#ifdef CONFIG_BASLER_CAMERA_VVCAM
module_i2c_driver(basler_camera_i2c_driver_vvcam);
#else
module_i2c_driver(basler_camera_i2c_driver);
#endif

MODULE_DESCRIPTION("Basler camera subdev driver for vvcam");
MODULE_AUTHOR("Sebastian Suesens <sebastian.suesens@baslerweb.com>");
MODULE_LICENSE("GPL");
