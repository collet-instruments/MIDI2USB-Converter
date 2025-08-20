/**
  * @file           : midi2_task.h
  * @brief          : MIDI 2.0 UMP task header
  */

#ifndef __MIDI2_TASK_H__
#define __MIDI2_TASK_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

#ifndef TESTING
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#endif

/* Exported functions prototypes ---------------------------------------------*/
void vMidi2UartToUmpTask(void *pvParameters);
void vMidi2UmpToUartTask(void *pvParameters);
BaseType_t MIDI2_InitQueues(void);

/* Exported variables --------------------------------------------------------*/
extern QueueHandle_t xUmpTxQueue;
extern QueueHandle_t xUmpRxQueue;

#ifdef __cplusplus
}
#endif

#endif /* __MIDI2_TASK_H__ */
