/**
  * @file           : usb_device_task.h
  * @brief          : Header for USB device management task
  */

#ifndef __USB_DEVICE_TASK_H
#define __USB_DEVICE_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#ifndef TESTING
#include "FreeRTOS.h"
#include "task.h"
#include "tusb.h"
#else
#include "FreeRTOS.h"
#include "tusb.h"  // Include mock tusb.h for testing
#endif

/* Exported function prototypes ---------------------------------------------*/
void vUSBDeviceTask(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif /* __USB_DEVICE_TASK_H */
