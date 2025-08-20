
#ifndef USB_DESCRIPTORS_H
#define USB_DESCRIPTORS_H

#include <stdint.h>
#include <stddef.h>

// USB Vendor and Product IDs
#define USB_VID           0x6666  // Vendor ID for both MIDI modes
#define USB_PID_MIDI10    0x6602  // Product ID for MIDI 1.0
#define USB_PID_MIDI20    0x6666  // Product ID for MIDI 2.0

// USB String definitions
#define USB_MANUFACTURER_STRING     "MIDI2USB"
#define USB_PRODUCT_STRING_MIDI10   "MIDI2USB Converter (MIDI 1.0)"
#define USB_PRODUCT_STRING_MIDI20   "MIDI2USB Converter (MIDI 2.0)"
#define USB_SERIAL_STRING           "001"
#define USB_INTERFACE_STRING_ALT0   "MIDI2USB MIDI1.0"
#define USB_INTERFACE_STRING_ALT1   "MIDI2USB MIDI2.0"

// String descriptor indices
enum {
    STRID_LANGID       = 0,
    STRID_MANUFACTURER = 1,
    STRID_PRODUCT      = 2,
    STRID_SERIAL       = 3,
    STRID_ITFNAME      = 4,
    STRID_ITFNAME_ALT0 = 5,
    STRID_ITFNAME_ALT1 = 6
};

// USB Standard Descriptor Types
#define DESC_TYPE_DEVICE                0x01
#define DESC_TYPE_CONFIGURATION          0x02
#define DESC_TYPE_STRING                 0x03
#define DESC_TYPE_INTERFACE              0x04
#define DESC_TYPE_ENDPOINT               0x05
#define DESC_TYPE_DEVICE_QUALIFIER       0x06
#define DESC_TYPE_INTERFACE_ASSOCIATION  0x0B

// USB Class Codes
#define USB_CLASS_AUDIO                  0x01
#define USB_CLASS_MISC                   0xEF

// USB Audio Subclass Codes
#define AUDIO_SUBCLASS_CONTROL           0x01
#define AUDIO_SUBCLASS_STREAMING         0x03

// USB Misc Subclass and Protocol
#define MISC_SUBCLASS_COMMON             0x02
#define MISC_PROTOCOL_IAD                0x01

// USB Audio Class-Specific Descriptor Types
#define CS_INTERFACE                     0x24
#define CS_ENDPOINT                      0x25
#define CS_GR_TRM_BLOCK                  0x26

// USB Audio Class-Specific AC Interface Descriptor Subtypes
#define AC_DESCRIPTOR_HEADER             0x01

// USB Audio Class-Specific MS Interface Descriptor Subtypes
#define MS_DESCRIPTOR_HEADER             0x01
#define MS_MIDI_IN_JACK                  0x02
#define MS_MIDI_OUT_JACK                 0x03

// USB Audio Class-Specific MS Endpoint Descriptor Subtypes
#define MS_GENERAL                       0x01
#define MS_GENERAL_2_0                   0x02

// MIDI Jack Types
#define JACK_TYPE_EMBEDDED               0x01
#define JACK_TYPE_EXTERNAL               0x02

// Group Terminal Block Descriptor Subtypes
#define GR_TRM_BLOCK_HEADER              0x01
#define GR_TRM_BLOCK                     0x02

// Group Terminal Block Types
#define GTB_TYPE_BIDIRECTIONAL           0x00

// MIDI Protocol
#define MIDI_PROTOCOL_1_0_UP_TO_64       0x01
#define MIDI_PROTOCOL_1_0_UP_TO_128      0x02
#define MIDI_PROTOCOL_2_0                0x11
#define MIDI_PROTOCOL_2_0_WITH_JR        0x12

// Endpoint Attributes
#define EP_ATTR_BULK                     0x02

// USB Speeds and Packet Sizes
#define EP0_MAX_PACKET_SIZE              0x40
#define FS_MAX_PACKET_SIZE               0x40
#define HS_MAX_PACKET_SIZE               0x200

// Device Release Number
#define USB_BCD_DEVICE                   0x0100
#define USB_BCD_USB_2_0                  0x0200
#define USB_BCD_MSC_1_0                  0x0100
#define USB_BCD_MSC_2_0                  0x0200

// Configuration Attributes
#define CONFIG_ATTR_BUS_POWERED          0x80
#define CONFIG_MAX_POWER_MA(ma)          ((ma)/2)

// Descriptor Length Constants
#define GTB_HEADER_LENGTH                0x05
#define GTB_BLOCK_LENGTH                 0x0D
#define GTB_TOTAL_LENGTH                 (GTB_HEADER_LENGTH + GTB_BLOCK_LENGTH)

// Function prototypes for testable string operations
const char* USB_GetManufacturerString(void);
const char* USB_GetProductString(void);
const char* USB_GetSerialString(void);
const char* USB_GetInterfaceString(void);
size_t USB_ConvertASCIItoUTF16(const char* ascii_str, uint16_t* utf16_buf, size_t max_chars);

// Function prototypes for USB descriptor values
uint16_t USB_GetVendorID(void);
uint16_t USB_GetProductID(void);

// Function prototypes for USB descriptors
extern uint8_t const desc_device[];
extern uint8_t const desc_device_qualifier[];
extern uint8_t const desc_fs_configuration[];
extern uint8_t const gtb0[];

extern uint8_t const gtbLengths[];
extern uint8_t const epInterface[];
extern uint8_t const *group_descr[];
extern char const* string_desc_arr[];
extern uint8_t const string_desc_arr_length;

#endif // USB_DESCRIPTORS_H