#ifndef __UMP_TASK_H__
#define __UMP_TASK_H__

#include <stdint.h>
#include "mock_freertos.h"

// Include midi_common.h to get MIDIStats_t
#include "midi_common.h"

// Mock TinyUSB functions
#define tud_ump_n_mounted(x) (1)
#define tud_ump_write(itf, data, count) (count)
#define tud_ump_n_available(x) (0)
#define tud_ump_read(itf, data, count) (0)

// Mock pdMS_TO_TICKS
#define pdMS_TO_TICKS(x) (x)

// Function declarations
void vUmpToUsbTask(void *pvParameters);
void vUsbToUmpTask(void *pvParameters);

#ifdef TESTING
uint8_t GetUmpWordCount(uint32_t first_word);
#endif

#endif /* __UMP_TASK_H__ */