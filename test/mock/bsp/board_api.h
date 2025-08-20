#ifndef __MOCK_BOARD_API_H__
#define __MOCK_BOARD_API_H__

#include <stdint.h>

// Mock function for board serial number
static inline uint32_t board_usb_get_serial(uint16_t* buf, uint32_t max_len)
{
    // Return a fixed value for testing
    const char* serial = "TEST1234";
    uint32_t len = 8;
    if (len > max_len) len = max_len;
    
    for (uint32_t i = 0; i < len; i++) {
        buf[i] = serial[i];
    }
    
    return len;
}

#endif /* __MOCK_BOARD_API_H__ */