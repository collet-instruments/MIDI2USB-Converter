/**
  * @file           : usb_callbacks.h
  * @brief          : Header for USB callbacks
  */

#ifndef __USB_CALLBACKS_H
#define __USB_CALLBACKS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tusb.h"

/* Exported function prototypes ---------------------------------------------*/
void tud_mount_cb(void);
void tud_umount_cb(void);
void tud_suspend_cb(bool remote_wakeup_en);
void tud_resume_cb(void);
void tud_midi_mount_cb(uint8_t itf);
void tud_midi_rx_cb(uint8_t itf);

#ifdef __cplusplus
}
#endif

#endif /* __USB_CALLBACKS_H */
