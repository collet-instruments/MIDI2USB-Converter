/**
  * @file           : app_ump_device.h
  * @brief          : Wrapper for ump_device.h that uses app_ump.h
  */

#ifndef __APP_UMP_DEVICE_H__
#define __APP_UMP_DEVICE_H__

/* Include necessary headers */
#include <stdint.h>

/* First include our custom ump.h */
#include "app_ump.h"

/* Prevent ump_device.h from including the original ump.h */
#define _TUSB_UMP_H__

/* Now include the original ump_device.h */
#include "../../submodules/tusb_ump/ump_device.h"

/* Undefine the guard so it doesn't affect other includes */
#undef _TUSB_UMP_H__

#endif /* __APP_UMP_DEVICE_H__ */