/**
  * @file           : led_task.c
  * @brief          : LED control task implementation
  */

/* Includes ------------------------------------------------------------------*/
#include "led_task.h"
#include "mode_manager.h"
#include "midi_common.h"

/* External variables --------------------------------------------------------*/
extern UART_HandleTypeDef huart1;

/* Public functions ----------------------------------------------------------*/
/**
  * @brief  FreeRTOS Task for LED Status Indication
  * @param  pvParameters: Task parameters
  * @retval None
  */
void vLEDBlinkTask(void *pvParameters)
{
  /* Prevent unused parameter warning */
  (void)pvParameters;
  
  /* Task implementation */
  for(;;)
  {
    /* Set LEDs according to current mode - no blinking */
    ModeManager_SetLEDs(ModeManager_GetMode());
    
    /* Delay for 100ms for stable operation */
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
