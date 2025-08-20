#ifndef __MOCK_FREERTOS_H__
#define __MOCK_FREERTOS_H__

#include <stdint.h>
#include <stddef.h>

#ifndef NULL
#define NULL ((void*)0)
#endif

// Mock FreeRTOS types
typedef void* QueueHandle_t;
typedef void* SemaphoreHandle_t;
typedef uint32_t TickType_t;
typedef int32_t BaseType_t;

#define pdTRUE  1
#define pdFALSE 0
#define pdPASS  pdTRUE
#define pdFAIL  pdFALSE

#define portMAX_DELAY ((TickType_t) 0xffffffffUL)
#define portTICK_PERIOD_MS ((TickType_t) 1)
#define pdMS_TO_TICKS(ms) ((TickType_t)(ms))

// Mock queue functions
QueueHandle_t xQueueCreate(uint32_t uxQueueLength, uint32_t uxItemSize);
BaseType_t xQueueSend(QueueHandle_t xQueue, const void* pvItemToQueue, TickType_t xTicksToWait);
BaseType_t xQueueSendToBack(QueueHandle_t xQueue, const void* pvItemToQueue, TickType_t xTicksToWait);
BaseType_t xQueueReceive(QueueHandle_t xQueue, void* pvBuffer, TickType_t xTicksToWait);
BaseType_t xQueueReset(QueueHandle_t xQueue);

// Mock semaphore functions
SemaphoreHandle_t xSemaphoreCreateMutex(void);
BaseType_t xSemaphoreTake(SemaphoreHandle_t xSemaphore, TickType_t xTicksToWait);
BaseType_t xSemaphoreGive(SemaphoreHandle_t xSemaphore);

// Mock task functions
TickType_t xTaskGetTickCount(void);
void vTaskDelay(const TickType_t xTicksToDelay);

// Mock critical section macros
#define taskENTER_CRITICAL() do {} while(0)
#define taskEXIT_CRITICAL() do {} while(0)

#endif /* __MOCK_FREERTOS_H__ */