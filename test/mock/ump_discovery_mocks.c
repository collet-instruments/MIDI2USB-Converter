#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "ump_discovery.h"
#include "app_ump_device.h"
#include "mock_freertos.h"

// Mock global queue for UMP TX
QueueHandle_t xUmpTxQueue = NULL;

// Note: MIDICI_* functions are implemented in ump_discovery.c itself