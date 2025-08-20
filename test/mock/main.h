#ifndef __MAIN_H__
#define __MAIN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// Additional test environment declarations
void SystemClock_Config(void);
void MX_GPIO_Init(void);
void Error_Handler(void);

// Test environment definitions
typedef enum {
    GPIO_PIN_RESET = 0,
    GPIO_PIN_SET
} GPIO_PinState;

#define GPIO_PIN_0 ((uint16_t)0x0001)
#define GPIO_PIN_1 ((uint16_t)0x0002)
#define GPIO_PIN_2 ((uint16_t)0x0004)
#define GPIO_PIN_3 ((uint16_t)0x0008)
#define GPIO_PIN_4 ((uint16_t)0x0010)
#define GPIO_PIN_5 ((uint16_t)0x0020)
#define GPIO_PIN_6 ((uint16_t)0x0040)

#define RxMIDI_Pin GPIO_PIN_0
#define RxMIDI_GPIO_Port ((void*)0x1000)
#define TxMIDI_Pin GPIO_PIN_1
#define TxMIDI_GPIO_Port ((void*)0x1000)
#define MIDI_OUT_Pin GPIO_PIN_2
#define MIDI_OUT_GPIO_Port ((void*)0x1000)
#define MIDI_IN_Pin GPIO_PIN_3
#define MIDI_IN_GPIO_Port ((void*)0x1000)
#define M1LED_Pin GPIO_PIN_4
#define M1LED_GPIO_Port ((void*)0x1000)
#define SETUP_Pin GPIO_PIN_5
#define SETUP_GPIO_Port ((void*)0x1000)
#define M2LED_Pin GPIO_PIN_6
#define M2LED_GPIO_Port ((void*)0x1000)

// Mock HAL functions
void HAL_GPIO_WritePin(void* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);
GPIO_PinState HAL_GPIO_ReadPin(void* GPIOx, uint16_t GPIO_Pin);

// UART handle typedef for testing
typedef struct {
    void* Instance;
} UART_HandleTypeDef;

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H__ */