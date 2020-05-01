#ifndef BASLER_CAMERA_DRIVER_IOCTL_H
#define BASLER_CAMERA_DRIVER_IOCTL_H

#include "basler-camera-driver.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
  BASLER_IOC_G_INTERFACE_VERSION = 0x100,
  BASLER_IOC_READ_REGISTER,
  BASLER_IOC_WRITE_REGISTER,
  BASLER_IOC_G_DEVICE_INFORMATION,
  BASLER_IOC_G_CSI_INFORMATION
};
int  basler_hw_register(struct v4l2_device * vdev);
void basler_hw_unregister(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* BASLER_CAMERA_DRIVER_IOCTL_H */
