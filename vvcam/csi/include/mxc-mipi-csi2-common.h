/*
 * Copyright 2019 NXP
 */

#ifndef _MIPI_CSI2_COMMON_H_
#define _MIPI_CSI2_COMMON_H_

#include <linux/sizes.h>
#include <media/v4l2-device.h>

#define CSIS_MAX_ENTITIES		2
#define CSIS0_MAX_LANES			4
#define CSIS1_MAX_LANES			2

#define MIPI_CSIS_OF_NODE_NAME		"csi"

#define MIPI_CSIS_VC0_PAD_SINK		0
#define MIPI_CSIS_VC1_PAD_SINK		1
#define MIPI_CSIS_VC2_PAD_SINK		2
#define MIPI_CSIS_VC3_PAD_SINK		3

#define MIPI_CSIS_VC0_PAD_SOURCE	4
#define MIPI_CSIS_VC1_PAD_SOURCE	5
#define MIPI_CSIS_VC2_PAD_SOURCE	6
#define MIPI_CSIS_VC3_PAD_SOURCE	7
#define MIPI_CSIS_VCX_PADS_NUM		8


#define MIPI_CSIS_DEF_PIX_WIDTH		640
#define MIPI_CSIS_DEF_PIX_HEIGHT	480

#define VIV_MIPI_CSI_STREAM_CFG \
	_IOW('V', BASE_VIDIOC_PRIVATE + 35, int)

/* Non-image packet data buffers */
#define MIPI_CSIS_PKTDATA_SIZE		SZ_4K

#define DEFAULT_SCLK_CSIS_FREQ		166000000UL

#define MIPI_CSIS_NUM_EVENTS 13

struct mipi_csis_event {
	u32 mask;
	const char * const name;
	unsigned int counter;
};

struct csis_pktbuf {
	u32 *data;
	unsigned int len;
};

struct csis_hw_reset1 {
	struct regmap *src;
	u8 req_src;
	u8 rst_bit;
};

/**
 * struct csi_state - the driver's internal state data structure
 * @lock: mutex serializing the subdev and power management operations,
 *        protecting @format and @flags members
 * @sd: v4l2_subdev associated with CSIS device instance
 * @index: the hardware instance index
 * @pdev: CSIS platform device
 * @phy: pointer to the CSIS generic PHY
 * @regs: mmaped I/O registers memory
 * @supplies: CSIS regulator supplies
 * @clock: CSIS clocks
 * @irq: requested s5p-mipi-csis irq number
 * @flags: the state variable for power and streaming control
 * @clock_frequency: device bus clock frequency
 * @hs_settle: HS-RX settle time
 * @clk_settle: Clk settle time
 * @num_lanes: number of MIPI-CSI data lanes used
 * @max_num_lanes: maximum number of MIPI-CSI data lanes supported
 * @wclk_ext: CSI wrapper clock: 0 - bus clock, 1 - external SCLK_CAM
 * @csis_fmt: current CSIS pixel format
 * @format: common media bus format for the source and sink pad
 * @slock: spinlock protecting structure members below
 * @pkt_buf: the frame embedded (non-image) data buffer
 * @events: MIPI-CSIS event (error) counters
 */
struct csi_state {
	struct mutex lock;
	struct device		*dev;
	struct v4l2_subdev	sd;
	struct v4l2_device	v4l2_dev;

	struct media_pad pads[MIPI_CSIS_VCX_PADS_NUM];

	u8 index;
	struct platform_device *pdev;
	struct phy *phy;
	void __iomem *regs;
	struct clk *mipi_clk;
	struct clk *phy_clk;
	struct clk *disp_axi;
	struct clk *disp_apb;
	int irq;
	u32 flags;

	u32 clk_frequency;
	u32 hs_settle;
	u32 clk_settle;
	u32 num_lanes;
	u32 max_num_lanes;
	int	 id;
	u8 wclk_ext;

	u8 vchannel;
	const struct csis_pix_format *csis_fmt;
	struct v4l2_mbus_framefmt format;

	spinlock_t slock;
	struct csis_pktbuf pkt_buf;
	struct mipi_csis_event events[MIPI_CSIS_NUM_EVENTS];

	struct v4l2_async_subdev    asd;
	struct v4l2_async_notifier  subdev_notifier;
	struct v4l2_async_subdev    *async_subdevs[2];

	struct csis_hw_reset1 hw_reset;
	struct regulator     *mipi_phy_regulator;

	struct regmap *gasket;
	struct reset_control *soft_resetn;
	struct reset_control *clk_enable;
	struct reset_control *mipi_reset;
};

/**
 * struct csis_pix_format - CSIS pixel format description
 * @pix_width_alignment: horizontal pixel alignment, width will be
 *                       multiple of 2^pix_width_alignment
 * @code: corresponding media bus code
 * @fmt_reg: MIPI_CSIS_CONFIG register value
 * @data_alignment: MIPI-CSI data alignment in bits
 */
struct csis_pix_format {
	unsigned int pix_width_alignment;
	u32 code;
	u32 fmt_reg;
	u8 data_alignment;
};

#endif // _MIPI_CSI2_COMMON_H_