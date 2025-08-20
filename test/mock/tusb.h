#ifndef __MOCK_TUSB_H__
#define __MOCK_TUSB_H__

#include <stdint.h>
#include <stdbool.h>

// TinyUSB mock definitions
#define TUSB_DESC_STRING 0x03
#define TUSB_DESC_DEVICE 0x01
#define CFG_TUD_ENDPOINT0_SIZE 64

// Mock descriptor types
typedef struct {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdUSB;
    uint8_t bDeviceClass;
    uint8_t bDeviceSubClass;
    uint8_t bDeviceProtocol;
    uint8_t bMaxPacketSize0;
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;
    uint8_t iManufacturer;
    uint8_t iProduct;
    uint8_t iSerialNumber;
    uint8_t bNumConfigurations;
} tusb_desc_device_t;

// Mock control transfer structures
typedef struct {
    uint8_t bmRequestType;
    uint8_t bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
} tusb_control_request_t;

// Mock macros
#define TUD_CONFIG_DESC_LEN 9
#define TUD_MIDI_DESC_LEN 65
#define TUD_CONFIG_DESCRIPTOR(...) 0x09, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#define TUD_MIDI_DESCRIPTOR(...) 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#define TU_U16_LOW(u16)  ((uint8_t) ((u16) & 0xFF))
#define TU_U16_HIGH(u16) ((uint8_t) (((u16) >> 8) & 0xFF))
#define TU_MIN(a, b) ((a) < (b) ? (a) : (b))

// Mock functions
static inline bool tud_control_xfer(uint8_t rhport, const tusb_control_request_t* request, void* buffer, uint16_t len)
{
    (void)rhport;
    (void)request;
    (void)buffer;
    (void)len;
    return true;
}

// Mock alternate setting
static inline uint8_t tud_alt_setting(uint8_t itf)
{
    (void)itf;
    return 0;
}

#endif /* __MOCK_TUSB_H__ */