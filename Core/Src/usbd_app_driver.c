/*
 * Application driver implementation for UMP support
 * This file provides UMP driver to TinyUSB without modifying submodule files
 */

#include "tusb.h"
#include "device/usbd_pvt.h"
#include "app_ump_device.h"
#include "mode_manager.h"

// Application drivers array - UMP driver functions are implemented in ump_device.cpp
static usbd_class_driver_t const _app_drivers[] = {
    {
        .name = "UMP",
        .init = umpd_init,
        .deinit = umpd_deinit,
        .reset = umpd_reset,
        .open = umpd_open,
        .control_xfer_cb = umpd_control_xfer_cb,
        .xfer_cb = umpd_xfer_cb,
        .sof = NULL
    }
};

// TinyUSB callback to get application drivers
usbd_class_driver_t const* usbd_app_driver_get_cb(uint8_t* driver_count) {
    // Only register UMP driver in MIDI 2.0 mode
    if (ModeManager_GetMode() == MIDI_MODE_2_0) {
        *driver_count = TU_ARRAY_SIZE(_app_drivers);
        return _app_drivers;
    } else {
        // MIDI 1.0 mode: no application drivers, use standard MIDI only
        *driver_count = 0;
        return NULL;
    }
}
