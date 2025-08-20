#include "mock_hal.h"
#include <string.h>

static MockHAL_State_t mock_state;

// Mock function implementations
static HAL_PinState_t mock_read_setup_pin(void)
{
    mock_state.read_setup_pin_call_count++;
    return mock_state.setup_pin_state;
}

static void mock_set_m1_led(HAL_PinState_t state)
{
    mock_state.set_m1_led_call_count++;
    mock_state.m1_led_state = state;
}

static void mock_set_m2_led(HAL_PinState_t state)
{
    mock_state.set_m2_led_call_count++;
    mock_state.m2_led_state = state;
}

// Mock HAL interface
static const ModeManagerHAL_t mock_hal = {
    .read_setup_pin = mock_read_setup_pin,
    .set_m1_led = mock_set_m1_led,
    .set_m2_led = mock_set_m2_led
};

// Public functions
void MockHAL_Init(void)
{
    MockHAL_Reset();
}

void MockHAL_Reset(void)
{
    memset(&mock_state, 0, sizeof(mock_state));
}

void MockHAL_SetSetupPinState(HAL_PinState_t state)
{
    mock_state.setup_pin_state = state;
}

const MockHAL_State_t* MockHAL_GetState(void)
{
    return &mock_state;
}

const ModeManagerHAL_t* MockHAL_GetInterface(void)
{
    return &mock_hal;
}