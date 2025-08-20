/**
  * @file           : led_task.h
  * @brief          : LED control task header
  */

#ifndef __LED_TASK_H__
#define __LED_TASK_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#ifndef TESTING
#include "FreeRTOS.h"
#include "task.h"
#else
#include "FreeRTOS.h"
#endif

/* Exported functions prototypes ---------------------------------------------*/
void vLEDBlinkTask(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif /* __LED_TASK_H__ */
