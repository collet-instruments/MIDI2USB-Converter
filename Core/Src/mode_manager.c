/**
  * @file           : mode_manager.c
  * @brief          : MIDI mode manager implementation
  */

/* Includes ------------------------------------------------------------------*/
#include "mode_manager.h"
#include <stdbool.h>
#include <stddef.h>

#ifndef TESTING
#include "main.h"
#endif

/* Global variables ----------------------------------------------------------*/
// MIDI mode is determined ONLY at startup and remains fixed during operation
// This prevents accidental mode changes during runtime
MidiMode_t g_midi_mode = MIDI_MODE_1_0;

/* Private variables ---------------------------------------------------------*/
static const ModeManagerHAL_t* current_hal = NULL;

#ifndef TESTING
/* Private function prototypes -----------------------------------------------*/
static HAL_PinState_t production_read_setup_pin(void);
static void production_set_m1_led(HAL_PinState_t state);
static void production_set_m2_led(HAL_PinState_t state);

/* Production HAL implementation */
static const ModeManagerHAL_t production_hal = {
    .read_setup_pin = production_read_setup_pin,
    .set_m1_led = production_set_m1_led,
    .set_m2_led = production_set_m2_led
};

/* Private function implementations ------------------------------------------*/
static HAL_PinState_t production_read_setup_pin(void)
{
    GPIO_PinState state = HAL_GPIO_ReadPin(SETUP_GPIO_Port, SETUP_Pin);
    return (state == GPIO_PIN_SET) ? HAL_PIN_SET : HAL_PIN_RESET;
}

static void production_set_m1_led(HAL_PinState_t state)
{
    HAL_GPIO_WritePin(M1LED_GPIO_Port, M1LED_Pin, 
                      (state == HAL_PIN_SET) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void production_set_m2_led(HAL_PinState_t state)
{
    HAL_GPIO_WritePin(M2LED_GPIO_Port, M2LED_Pin, 
                      (state == HAL_PIN_SET) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

const ModeManagerHAL_t* ModeManagerHAL_GetProduction(void)
{
    return &production_hal;
}
#endif /* TESTING */

/* Public functions ----------------------------------------------------------*/

/**
  * @brief  Determine mode from pin state (pure function for testing)
  * @param  pin_state: State of the setup pin
  * @retval MIDI mode
  */
MidiMode_t ModeManager_DetermineModeFromPin(HAL_PinState_t pin_state)
{
    // LOW = MIDI 1.0 mode, HIGH = MIDI 2.0 mode
    return (pin_state == HAL_PIN_SET) ? MIDI_MODE_2_0 : MIDI_MODE_1_0;
}

/**
  * @brief  Initialize mode manager with dependency injection
  * @param  hal: Hardware abstraction layer interface
  * @retval None
  */
void ModeManager_InitWithHAL(const ModeManagerHAL_t* hal)
{
    if (hal == NULL) {
        return;
    }
    
    current_hal = hal;
    
    // Read SETUP pin state ONCE at startup
    HAL_PinState_t setup_state = hal->read_setup_pin();
    
    // Determine mode using pure function
    g_midi_mode = ModeManager_DetermineModeFromPin(setup_state);
    
    // Set LEDs according to the determined mode
    ModeManager_SetLEDsWithHAL(g_midi_mode, hal);
}

/**
  * @brief  Initialize mode manager by reading SETUP pin (ONE TIME ONLY)
  * @note   SETUP pin is read only during initialization for safety.
  *         Mode cannot be changed during runtime to prevent accidental switching.
  * @retval None
  */
void ModeManager_Init(void)
{
#ifndef TESTING
    ModeManager_InitWithHAL(&production_hal);
#endif
}

/**
  * @brief  Get current MIDI mode (returns cached value, does NOT read pin)
  * @note   Mode is determined only at startup and never changes during runtime
  * @retval Current MIDI mode (cached from initialization)
  */
MidiMode_t ModeManager_GetMode(void)
{
    // Return cached mode - NEVER re-read the SETUP pin for safety
    return g_midi_mode;
}

/**
  * @brief  Set LEDs according to MIDI mode (testable version)
  * @param  mode: MIDI mode
  * @param  hal: Hardware abstraction layer interface
  * @retval None
  */
void ModeManager_SetLEDsWithHAL(MidiMode_t mode, const ModeManagerHAL_t* hal)
{
    if (hal == NULL) {
        return;
    }
    
    if (mode == MIDI_MODE_1_0) {
        // MIDI 1.0 mode: M1LED ON, M2LED OFF
        hal->set_m1_led(HAL_PIN_SET);
        hal->set_m2_led(HAL_PIN_RESET);
    } else {
        // MIDI 2.0 mode: M1LED OFF, M2LED ON
        hal->set_m1_led(HAL_PIN_RESET);
        hal->set_m2_led(HAL_PIN_SET);
    }
}

/**
  * @brief  Set LEDs according to MIDI mode
  * @note   LEDs reflect the fixed mode determined at startup
  * @param  mode: MIDI mode
  * @retval None
  */
void ModeManager_SetLEDs(MidiMode_t mode)
{
    if (current_hal != NULL) {
        ModeManager_SetLEDsWithHAL(mode, current_hal);
    }
#ifndef TESTING
    else {
        // Fallback to production HAL if not initialized
        ModeManager_SetLEDsWithHAL(mode, &production_hal);
    }
#endif
}

/**
 * @brief Check if MIDI 2.0 mode is active
 * @return true: MIDI 2.0 mode, false: MIDI 1.0 mode
 */
bool is_midi2_mode(void) {
    return ModeManager_GetMode() == MIDI_MODE_2_0;
}