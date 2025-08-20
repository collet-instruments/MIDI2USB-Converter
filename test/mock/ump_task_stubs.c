#include <stdint.h>
#include "ump_task.h"

// Mock global variables
MIDIStats_t midi_stats = {0};
QueueHandle_t xUmpTxQueue = NULL;
QueueHandle_t xUmpRxQueue = NULL;