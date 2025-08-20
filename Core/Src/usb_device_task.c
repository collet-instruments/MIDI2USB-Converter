/**
  * @file           : usb_device_task.c
  * @brief          : USB device management task implementation
  */

/* Includes ------------------------------------------------------------------*/
#include "usb_device_task.h"

/* External variables --------------------------------------------------------*/
extern UART_HandleTypeDef huart1;

/* Public functions ----------------------------------------------------------*/
/**
  * @brief USB Device Driver task
  * @note This top level thread process all usb events and invoke callbacks
  * @param pvParameters: Task parameters
  * @retval None
  */
void vUSBDeviceTask(void *pvParameters) {
  (void) pvParameters;

  // init device stack on configured roothub port
  // This should be called after scheduler/kernel is started.
  // Otherwise it could cause kernel issue since USB IRQ handler does use RTOS queue API.
  tusb_rhport_init_t dev_init = {
    .role = TUSB_ROLE_DEVICE,
    .speed = TUSB_SPEED_AUTO
  };
  tusb_rhport_init(0, &dev_init);

  while (1) {
    tud_task(); // TinyUSB device task
    
    vTaskDelay(1);  // Allow lower priority tasks to run
  }
}
