#ifndef _VVSENSOR_PUBLIC_HEADER_H_
#define _VVSENSOR_PUBLIC_HEADER_H_

#ifdef __KERNEL__
#include <media/v4l2-subdev.h>
#else
#include <linux/videodev2.h>
#include <linux/v4l2-subdev.h>
#endif

enum {
	VVSENSORIOC_G_BLS = 0x100,
	VVSENSORIOC_G_GAIN,
	VVSENSORIOC_S_GAIN,
	VVSENSORIOC_S_VSGAIN,
	VVSENSORIOC_S_EXP,
	VVSENSORIOC_S_VSEXP,
	VVSENSORIOC_S_PATTERN,
	VVSENSORIOC_S_BLS,
	VVSENSORIOC_S_POWER,
	VVSENSORIOC_G_VERSION,
	VVSENSORIOC_STREAMON,
	VVSENSORIOC_STREAMOFF,
	VVSENSORIOC_READ_REG,
	VVSENSORIOC_WRITE_REG,
	VVSENSORIOC_S_HDR,
	VVSENSORIOC_S_FPS,
};

/* priv ioctl */
struct vvsensor_reg_setting_t {
	__u16 addr;
	__u8 val;
};

/* init settings */
struct vvsensor_reg_value_t {
	__u16 addr;
	__u8 val;
	__u8 mask;
	__u32 delay;
};

/* priv ioctl */
struct vvsensor_gain_context {
	__u32 again;
	__u32 dgain;
};

#endif
