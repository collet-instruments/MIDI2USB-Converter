/*
 * TinyUSB MIDI callbacks for MIDI 1.0 mode
 */

#include "tusb.h"
#include "mode_manager.h"
#include "ump_discovery.h"
#include "FreeRTOS.h"
#include "task.h"

/* External variables --------------------------------------------------------*/
extern UART_HandleTypeDef huart1;

// Invoked when device is mounted
void tud_mount_cb(void)
{
  // No debug message - keep it minimal
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
  // No debug message - keep it minimal
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  // No debug message - keep it minimal
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
  // No debug message - keep it minimal
}

// MIDI interface ready callback (only for MIDI 1.0 mode)
void tud_midi_mount_cb(uint8_t itf)
{
  (void)itf; // Suppress unused parameter warning
  if (ModeManager_GetMode() == MIDI_MODE_1_0) {
    // Add delay to prevent callback issues
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

// Forward declaration for UMP mount callback (defined in usb_descriptors.c)
extern void tud_ump_mount_cb(uint8_t itf, uint8_t alt_setting);

// Optional: MIDI receive callback for debugging
void tud_midi_rx_cb(uint8_t itf)
{
  (void) itf;
  // Don't process data here - just signal the callback occurred
  // Actual data processing should be done in the USB RX task
}
