#ifndef __MOCK_HAL_H__
#define __MOCK_HAL_H__

#include <stdint.h>
#include "mode_manager_hal.h"

// Mock HAL state structure
typedef struct {
    HAL_PinState_t setup_pin_state;
    HAL_PinState_t m1_led_state;
    HAL_PinState_t m2_led_state;
    int read_setup_pin_call_count;
    int set_m1_led_call_count;
    int set_m2_led_call_count;
} MockHAL_State_t;

// Initialize mock HAL
void MockHAL_Init(void);

// Reset mock state
void MockHAL_Reset(void);

// Set mock return values
void MockHAL_SetSetupPinState(HAL_PinState_t state);

// Get mock state
const MockHAL_State_t* MockHAL_GetState(void);

// Get mock HAL interface
const ModeManagerHAL_t* MockHAL_GetInterface(void);

#endif /* __MOCK_HAL_H__ */