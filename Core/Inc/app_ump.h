/**
 * @file           : app_ump.h
 * @brief          : UMP (Universal MIDI Packet) application interface
 */

/** \ingroup group_class
 *  \defgroup ClassDriver_CDC Communication Device Class (CDC)
 *            Currently only Abstract Control Model subclass is supported
 *  @{ */

#ifndef _TUSB_UMP_H__
#define _TUSB_UMP_H__

//#include "common/tusb_common.h"

/* Define TU_ATTR_PACKED if not already defined */
#ifndef TU_ATTR_PACKED
  #if defined(__GNUC__)
    #define TU_ATTR_PACKED __attribute__((packed))
  #elif defined(_MSC_VER)
    #define TU_ATTR_PACKED
    #pragma warning(push)
    #pragma warning(disable : 4103)
    #pragma pack(push, 1)
  #else
    #define TU_ATTR_PACKED
  #endif
#endif

#ifdef __cplusplus
 extern "C" {
#endif

//--------------------------------------------------------------------+
// UMP Protocol definitions
//--------------------------------------------------------------------+
// Message Types
#define UMP_MT_MASK       0xf0
#define UMP_MT_UTILITY    0x00
#define UMP_MT_SYSTEM     0x10
#define UMP_MT_MIDI1_CV   0x20
#define UMP_MT_DATA_64    0x30
#define UMP_MT_MIDI2_CV   0x40
#define UMP_MT_DATA_128   0x50
#define UMP_MT_RESERVED_6 0x60 // 32bits reserved future
#define UMP_MT_RESERVED_7 0x70 // 32bits reserved future
#define UMP_MT_RESERVED_8 0x80 // 64bits reserved future
#define UMP_MT_RESERVED_9 0x90 // 64bits reserved future
#define UMP_MT_RESERVED_A 0xA0 // 64bits reserved future
#define UMP_MT_RESERVED_B 0xB0 // 96bits reserved future
#define UMP_MT_RESERVED_C 0xC0 // 96bits reserved future
#define UMP_MT_FLEX_128   0xd0
#define UMP_MT_RESERVED_E 0xE0 // 128bits reserved future
#define UMP_MT_STREAM_128 0xf0

// Group Number
#define UMP_GROUP_MASK    0x0f

// System Exclusive 7-Bit Status
#define UMP_SYSEX7_STATUS_MASK  0xf0
#define UMP_SYSEX7_COMPLETE     0x00
#define UMP_SYSEX7_START        0x10
#define UMP_SYSEX7_CONTINUE     0x20
#define UMP_SYSEX7_END          0x30
#define UMP_SYSEX7_SIZE_MASK    0x0f

// System Common Status
#define UMP_SYSTEM_MTC          0xf1  // 2 bytes incl status
#define UMP_SYSTEM_SONG_POS_PTR 0xf2  // 3 bytes incl status
#define UMP_SYSTEM_SONG_SELECT  0xf3  // 2 bytes incl status
#define UMP_SYSTEM_UNDEFINED_F4 0xf4  // undefined
#define UMP_SYSTEM_UNDEFINED_F5 0xf5  // undefined
#define UMP_SYSTEM_TUNE_REQ     0xf6  // status byte only
#define UMP_SYSTEM_TIMING_CLK   0xf8  // status byte only
#define UMP_SYSTEM_UNDEFINED_F9 0xf9  // undefined
#define UMP_SYSTEM_START        0xfa  // status byte only
#define UMP_SYSTEM_CONTINUE     0xfb  // status byte only
#define UMP_SYSTEM_STOP         0xfc  // status byte only
#define UMP_SYSTEM_UNDEFINED_FD 0xfd  // undefined
#define UMP_SYSTEM_ACTIVE_SENSE 0xfe  // status byte only
#define UMP_SYSTEM_RESET        0xff  // status byte only

//--------------------------------------------------------------------+
// Class Specific Descriptor
//--------------------------------------------------------------------+

// Only define these if TinyUSB MIDI hasn't been included
#ifndef TUSB_MIDI_H_

typedef enum
{
  MIDI_CS_INTERFACE_HEADER    = 0x01,
  MIDI_CS_INTERFACE_IN_JACK   = 0x02,
  MIDI_CS_INTERFACE_OUT_JACK  = 0x03,
  MIDI_CS_INTERFACE_ELEMENT   = 0x04,
  MIDI_CS_INTERFACE_GR_TRM_BLOCK = 0x26,
} midi_cs_interface_subtype_t;

typedef enum
{
  MIDI_CS_ENDPOINT_GENERAL = 0x01,
  MIDI20_CS_ENDPOINT_GENERAL = 0x02,
} midi_cs_endpoint_subtype_t;

typedef enum
{
  MIDI_JACK_EMBEDDED = 0x01,
  MIDI_JACK_EXTERNAL = 0x02
} midi_jack_type_t;

#endif // TUSB_MIDI_H_

typedef enum
{
  MIDI_GR_TRM_BLOCK_HEADER = 0x01,
  MIDI_GR_TRM_BLOCK = 0x02
} midi_group_terminal_block_type_t;

// Only define MIDI CIN if not already defined by TinyUSB
#ifndef TUSB_MIDI_H_

typedef enum
{
  MIDI_CIN_MISC              = 0,
  MIDI_CIN_CABLE_EVENT       = 1,
  MIDI_CIN_SYSCOM_2BYTE      = 2, // 2 byte system common message e.g MTC, SongSelect
  MIDI_CIN_SYSCOM_3BYTE      = 3, // 3 byte system common message e.g SPP
  MIDI_CIN_SYSEX_START       = 4, // SysEx starts or continue
  MIDI_CIN_SYSEX_END_1BYTE   = 5, // SysEx ends with 1 data, or 1 byte system common message
  MIDI_CIN_SYSEX_END_2BYTE   = 6, // SysEx ends with 2 data
  MIDI_CIN_SYSEX_END_3BYTE   = 7, // SysEx ends with 3 data
  MIDI_CIN_NOTE_ON           = 9,  // Fixed: TinyUSB uses 9 for NOTE_ON
  MIDI_CIN_NOTE_OFF          = 8,  // Fixed: TinyUSB uses 8 for NOTE_OFF
  MIDI_CIN_POLY_KEYPRESS     = 10,
  MIDI_CIN_CONTROL_CHANGE    = 11,
  MIDI_CIN_PROGRAM_CHANGE    = 12,
  MIDI_CIN_CHANNEL_PRESSURE  = 13,
  MIDI_CIN_PITCH_BEND_CHANGE = 14,
  MIDI_CIN_1BYTE_DATA = 15
} midi_code_index_number_t;

#endif // TUSB_MIDI_H_

// Only define MIDI status if not already defined by TinyUSB
#ifndef TUSB_MIDI_H_

// MIDI 1.0 status byte
enum
{
  //------------- System Exclusive -------------//
  MIDI_STATUS_SYSEX_START                    = 0xF0,
  MIDI_STATUS_SYSEX_END                      = 0xF7,

  //------------- System Common -------------//
  MIDI_STATUS_SYSCOM_TIME_CODE_QUARTER_FRAME = 0xF1,
  MIDI_STATUS_SYSCOM_SONG_POSITION_POINTER   = 0xF2,
  MIDI_STATUS_SYSCOM_SONG_SELECT             = 0xF3,
  // F4, F5 is undefined
  MIDI_STATUS_SYSCOM_TUNE_REQUEST            = 0xF6,

  //------------- System RealTime  -------------//
  MIDI_STATUS_SYSREAL_TIMING_CLOCK           = 0xF8,
  // 0xF9 is undefined
  MIDI_STATUS_SYSREAL_START                  = 0xFA,
  MIDI_STATUS_SYSREAL_CONTINUE               = 0xFB,
  MIDI_STATUS_SYSREAL_STOP                   = 0xFC,
  // 0xFD is undefined
  MIDI_STATUS_SYSREAL_ACTIVE_SENSING         = 0xFE,
  MIDI_STATUS_SYSREAL_SYSTEM_RESET           = 0xFF,
};

#endif // TUSB_MIDI_H_

// Only define MIDI descriptor structures if not already defined by TinyUSB
#ifndef TUSB_MIDI_H_

/// MIDI Interface Header Descriptor
typedef struct TU_ATTR_PACKED
{
  uint8_t bLength            ; ///< Size of this descriptor in bytes.
  uint8_t bDescriptorType    ; ///< Descriptor Type, must be Class-Specific
  uint8_t bDescriptorSubType ; ///< Descriptor SubType
  uint16_t bcdMSC            ; ///< MidiStreaming SubClass release number in Binary-Coded Decimal
  uint16_t wTotalLength      ;
} midi_desc_header_t;

/// MIDI In Jack Descriptor
typedef struct TU_ATTR_PACKED
{
  uint8_t bLength            ; ///< Size of this descriptor in bytes.
  uint8_t bDescriptorType    ; ///< Descriptor Type, must be Class-Specific
  uint8_t bDescriptorSubType ; ///< Descriptor SubType
  uint8_t bJackType          ; ///< Embedded or External
  uint8_t bJackID            ; ///< Unique ID for MIDI IN Jack
  uint8_t iJack              ; ///< string descriptor
} midi_desc_in_jack_t;


/// MIDI Out Jack Descriptor with single pin
typedef struct TU_ATTR_PACKED
{
  uint8_t bLength            ; ///< Size of this descriptor in bytes.
  uint8_t bDescriptorType    ; ///< Descriptor Type, must be Class-Specific
  uint8_t bDescriptorSubType ; ///< Descriptor SubType
  uint8_t bJackType          ; ///< Embedded or External
  uint8_t bJackID            ; ///< Unique ID for MIDI IN Jack
  uint8_t bNrInputPins;

  uint8_t baSourceID;
  uint8_t baSourcePin;

  uint8_t iJack              ; ///< string descriptor
} midi_desc_out_jack_t ;

/// MIDI Out Jack Descriptor with multiple pins
#define midi_desc_out_jack_n_t(input_num) \
  struct TU_ATTR_PACKED { \
    uint8_t bLength            ; \
    uint8_t bDescriptorType    ; \
    uint8_t bDescriptorSubType ; \
    uint8_t bJackType          ; \
    uint8_t bJackID            ; \
    uint8_t bNrInputPins       ; \
    struct TU_ATTR_PACKED {      \
        uint8_t baSourceID;      \
        uint8_t baSourcePin;     \
    } pins[input_num];           \
   uint8_t iJack              ;  \
  }

/// MIDI Element Descriptor
typedef struct TU_ATTR_PACKED
{
  uint8_t bLength            ; ///< Size of this descriptor in bytes.
  uint8_t bDescriptorType    ; ///< Descriptor Type, must be Class-Specific
  uint8_t bDescriptorSubType ; ///< Descriptor SubType
  uint8_t bElementID;

  uint8_t bNrInputPins;
  uint8_t baSourceID;
  uint8_t baSourcePin;

  uint8_t bNrOutputPins;
  uint8_t bInTerminalLink;
  uint8_t bOutTerminalLink;
  uint8_t bElCapsSize;

  uint16_t bmElementCaps;
  uint8_t  iElement;
} midi_desc_element_t;

#endif // TUSB_MIDI_H_

/// MIDI Element Descriptor with multiple pins
#define midi_desc_element_n_t(input_num) \
  struct TU_ATTR_PACKED {       \
    uint8_t bLength;            \
    uint8_t bDescriptorType;    \
    uint8_t bDescriptorSubType; \
    uint8_t bElementID;         \
    uint8_t bNrInputPins;       \
    struct TU_ATTR_PACKED {     \
        uint8_t baSourceID;     \
        uint8_t baSourcePin;    \
    } pins[input_num];          \
    uint8_t bNrOutputPins;      \
    uint8_t bInTerminalLink;    \
    uint8_t bOutTerminalLink;   \
    uint8_t bElCapsSize;        \
    uint16_t bmElementCaps;     \
    uint8_t  iElement;          \
 }

/// MIDI 2 Streaming Data Endpoint Descriptor with one group terminal block
typedef struct TU_ATTR_PACKED
{
  uint8_t bLength            ; ///< Size of this descriptor in bytes: 4+n
  uint8_t bDescriptorType    ; ///< Descriptor Type: CS_ENDPOINT
  uint8_t bDescriptorSubType ; ///< Descriptor SubType: MIDI20_CS_ENDPOINT_GENERAL
  uint8_t bNumGrpTrmBlock    ; ///< Number of Group Terminal Blocks: 1
  uint8_t bAssoGrpTrmBlkID   ; ///< ID of the Group Terminal Block that is associated with this endpoint
} midi2_desc_streaming_data_endpoint_t;

/// MIDI 2 Streaming Data Endpoint Descriptor with multiple group terminal blocks
#define midi2_desc_streaming_data_endpoint_n_t(group_terminal_block_num) \
  struct TU_ATTR_PACKED {       \
    uint8_t bLength;            \
    uint8_t bDescriptorType;    \
    uint8_t bDescriptorSubType; \
    uint8_t bNumGrpTrmBlock;    \
    uint8_t bNrInputPins;       \
    uint8_t baAssoGrpTrmBlkID[group_terminal_block_num]; \
  }

/// MIDI 2 Group Terminal Block Header Descriptor
typedef struct TU_ATTR_PACKED
{
  uint8_t  bLength            ; ///< Size of this descriptor in bytes: 5
  uint8_t  bDescriptorType    ; ///< Descriptor Type: MIDI_CS_INTERFACE_GR_TRM_BLOCK
  uint8_t  bDescriptorSubType ; ///< Descriptor SubType: MIDI_GR_TRM_BLOCK_HEADER
  uint16_t wTotalLength       ; ///< Total number of bytes returned for the class-specific Group Terminal Block descriptors. Includes the combined length of this header descriptor and all Group Terminal Block descriptors.
} midi2_desc_group_terminal_block_header_t;

/// MIDI 2 Group Terminal Block Descriptor
typedef struct TU_ATTR_PACKED
{
  uint8_t  bLength            ; ///< Size of this descriptor in bytes: 13
  uint8_t  bDescriptorType    ; ///< Descriptor Type: MIDI_CS_INTERFACE_GR_TRM_BLOCK
  uint8_t  bDescriptorSubType ; ///< Descriptor SubType: MIDI_GR_TRM_BLOCK
  uint8_t  bGrpTrmBlkID       ; ///< ID of this Group Terminal Block
  uint8_t  bGrpTrmBlkType     ; ///< Group Terminal Block Type
  uint8_t  nGroupTrm          ; ///< The first member Group Terminal in this Block
  uint8_t  nNumGroupTrm       ; ///< Number of member Group Terminals spanned
  uint8_t  iBlockItem         ; ///< ID of STRING descriptor for UI representation of Block item
  uint8_t  bMIDIProtocol      ; ///< Default MIDI protocol
  uint16_t wMaxInputBandwidth ; ///< Maximum Input Bandwidth Capability in 4KB/second
  uint16_t wMaxOutputBandwidth; ///< Maximum Output Bandwidth Capability in 4KB/second
} midi2_desc_group_terminal_block_t;

/// MIDI 2 Group Terminal Blocks Descriptor with one group terminal block
typedef struct TU_ATTR_PACKED
{
  midi2_desc_group_terminal_block_header_t header;
  midi2_desc_group_terminal_block_t block;
} midi2_cs_interface_desc_group_terminal_blocks_t;

/// MIDI 2 Group Terminal Blocks Descriptor with one group terminal block
#define midi2_cs_interface_desc_group_terminal_blocks_n_t(group_terminal_block_num) \
  struct TU_ATTR_PACKED {       \
    midi2_desc_group_terminal_block_header_t header;            \
    midi2_desc_group_terminal_block_t aBlock[group_terminal_block_num]; \
  }

/** @} */

#ifdef __cplusplus
 }
#endif

/* Restore MSC pack settings if needed */
#if defined(_MSC_VER) && defined(TU_ATTR_PACKED)
  #pragma pack(pop)
  #pragma warning(pop)
#endif

#endif

/** @} */