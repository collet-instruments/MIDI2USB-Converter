#include "mock_freertos.h"
#include <stdlib.h>

// Mock implementations for FreeRTOS functions

static uint32_t mock_queue_created = 0;
static uint32_t mock_mutex_created = 0;

QueueHandle_t xQueueCreate(uint32_t uxQueueLength, uint32_t uxItemSize)
{
    (void)uxQueueLength;
    (void)uxItemSize;
    mock_queue_created++;
    return (QueueHandle_t)(uintptr_t)(mock_queue_created);
}

BaseType_t xQueueSend(QueueHandle_t xQueue, const void* pvItemToQueue, TickType_t xTicksToWait)
{
    (void)xQueue;
    (void)pvItemToQueue;
    (void)xTicksToWait;
    return pdPASS;
}

BaseType_t xQueueSendToBack(QueueHandle_t xQueue, const void* pvItemToQueue, TickType_t xTicksToWait)
{
    (void)xQueue;
    (void)pvItemToQueue;
    (void)xTicksToWait;
    return pdPASS;
}

BaseType_t xQueueReceive(QueueHandle_t xQueue, void* pvBuffer, TickType_t xTicksToWait)
{
    (void)xQueue;
    (void)pvBuffer;
    (void)xTicksToWait;
    return pdFAIL; // No data available
}

BaseType_t xQueueReset(QueueHandle_t xQueue)
{
    (void)xQueue;
    return pdPASS;
}

SemaphoreHandle_t xSemaphoreCreateMutex(void)
{
    mock_mutex_created++;
    return (SemaphoreHandle_t)(uintptr_t)(mock_mutex_created);
}

BaseType_t xSemaphoreTake(SemaphoreHandle_t xSemaphore, TickType_t xTicksToWait)
{
    (void)xSemaphore;
    (void)xTicksToWait;
    return pdPASS;
}

BaseType_t xSemaphoreGive(SemaphoreHandle_t xSemaphore)
{
    (void)xSemaphore;
    return pdPASS;
}

TickType_t xTaskGetTickCount(void)
{
    static TickType_t tick_count = 0;
    return tick_count++;
}

void vTaskDelay(const TickType_t xTicksToDelay)
{
    (void)xTicksToDelay;
}