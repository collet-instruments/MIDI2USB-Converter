/**
  ******************************************************************************
  * @file           : mode_manager_hal.h
  * @brief          : Hardware abstraction layer for mode manager
  ******************************************************************************
  * @attention
  *
  * This file provides an abstraction layer for hardware operations,
  * following the Dependency Injection pattern for better testability.
  *
  ******************************************************************************
  */

#ifndef __MODE_MANAGER_HAL_H__
#define __MODE_MANAGER_HAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* GPIO pin state abstraction */
typedef enum {
    HAL_PIN_RESET = 0,
    HAL_PIN_SET = 1
} HAL_PinState_t;

/* Hardware abstraction interface */
typedef struct {
    /* Read the setup pin state */
    HAL_PinState_t (*read_setup_pin)(void);
    
    /* Control M1 LED */
    void (*set_m1_led)(HAL_PinState_t state);
    
    /* Control M2 LED */
    void (*set_m2_led)(HAL_PinState_t state);
} ModeManagerHAL_t;

/* Production implementation (defined in mode_manager.c) */
const ModeManagerHAL_t* ModeManagerHAL_GetProduction(void);

#ifdef __cplusplus
}
#endif

#endif /* __MODE_MANAGER_HAL_H__ */