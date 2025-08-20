/**
  * @file           : ump_task.h
  * @brief          : UMP USB communication tasks header
  */

#ifndef __UMP_TASK_H__
#define __UMP_TASK_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#ifndef TESTING
#include "FreeRTOS.h"
#include "task.h"
#endif

/* Exported functions prototypes ---------------------------------------------*/
void vUmpToUsbTask(void *pvParameters);
void vUsbToUmpTask(void *pvParameters);

#ifdef TESTING
// Expose GetUmpWordCount for testing
uint8_t GetUmpWordCount(uint32_t first_word);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __UMP_TASK_H__ */
