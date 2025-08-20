/**
  * @file           : usb_midi_task.h
  * @brief          : Header for USB MIDI tasks
  */

#ifndef __USB_MIDI_TASK_H
#define __USB_MIDI_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#ifndef TESTING
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "tusb.h"
#include "midi_common.h"
#else
#include "FreeRTOS.h"
#include "queue.h"
#include "tusb.h"
#include "midi_common.h"
#endif

/* Exported function prototypes ---------------------------------------------*/
void vUsbRxMidiTask(void *pvParameters);
void vUsbToUartTask(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif /* __USB_MIDI_TASK_H */
