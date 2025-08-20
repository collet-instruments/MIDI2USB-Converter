/**
  * @file           : uart_midi_task.h
  * @brief          : Header for UART MIDI tasks
  */

#ifndef __UART_MIDI_TASK_H
#define __UART_MIDI_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#ifndef TESTING
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "midi_common.h"
#else
#include "FreeRTOS.h"
#include "queue.h"
#include "midi_common.h"
#endif

/* Exported constants --------------------------------------------------------*/
#define UART_TX_BUFFER_SIZE 512     // Size for DMA TX buffer (enough for SysEx)
#define UART_TX_QUEUE_LENGTH 32     // Number of TX buffer entries in queue

/* Exported types ------------------------------------------------------------*/
// UART TX buffer structure for DMA
typedef struct {
    uint8_t data[UART_TX_BUFFER_SIZE];
    uint16_t length;
    volatile uint8_t in_use;  // Flag to indicate buffer is being transmitted
} UartTxBuffer_t;

/* Exported variables --------------------------------------------------------*/
extern SemaphoreHandle_t xUartTxCompleteSemaphore;  // Semaphore for TX completion
extern volatile uint8_t uart_tx_dma_busy;           // DMA busy flag

/* Exported function prototypes ---------------------------------------------*/
void vUartRxMidiTask(void *pvParameters);
void vUartToUsbTask(void *pvParameters);
void vUsbToUartTask(void *pvParameters);
void UART_TX_DMA_Init(void);
BaseType_t UART_TX_SendDMA(const uint8_t *data, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* __UART_MIDI_TASK_H */
