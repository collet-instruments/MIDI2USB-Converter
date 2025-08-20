/**
  * @file           : mode_manager.h
  * @brief          : MIDI mode manager header
  */

#ifndef __MODE_MANAGER_H__
#define __MODE_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include "mode_manager_hal.h"

#ifndef TESTING
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#endif

/* Exported types ------------------------------------------------------------*/
typedef enum {
    MIDI_MODE_1_0 = 0,  // SETUP pin HIGH (traditional MIDI)
    MIDI_MODE_2_0 = 1   // SETUP pin LOW (MIDI 2.0 with UMP)
} MidiMode_t;

/* Exported variables --------------------------------------------------------*/
// Mode is determined at startup and remains constant during runtime
extern MidiMode_t g_midi_mode;

/* Exported functions prototypes ---------------------------------------------*/
// Initialize mode manager with dependency injection
void ModeManager_InitWithHAL(const ModeManagerHAL_t* hal);

// Initialize mode manager - reads SETUP pin ONCE at startup only
void ModeManager_Init(void);

// Get current mode - returns cached value, never re-reads pin
MidiMode_t ModeManager_GetMode(void);

// Set LEDs according to current mode (testable pure function)
void ModeManager_SetLEDsWithHAL(MidiMode_t mode, const ModeManagerHAL_t* hal);

// Set LEDs according to current mode
void ModeManager_SetLEDs(MidiMode_t mode);

// Determine mode from pin state (pure function for testing)
MidiMode_t ModeManager_DetermineModeFromPin(HAL_PinState_t pin_state);

/**
 * @brief Check if MIDI 2.0 mode is active
 * @return true: MIDI 2.0 mode, false: MIDI 1.0 mode
 */
bool is_midi2_mode(void);

#ifdef __cplusplus
}
#endif

#endif /* __MODE_MANAGER_H__ */
