/**
  * @file           : ump_discovery.h
  * @brief          : UMP Endpoint Discovery and MIDI-CI implementation
  */

#ifndef __UMP_DISCOVERY_H__
#define __UMP_DISCOVERY_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/

// UMP Stream Message Types (MT=0xF)
typedef enum {
    UMP_STREAM_MSG_STATUS = 0x00,
    UMP_STREAM_MSG_ENDPOINT_DISCOVERY = 0x00,
    UMP_STREAM_MSG_ENDPOINT_INFO = 0x01,
    UMP_STREAM_MSG_DEVICE_IDENTITY = 0x02,
    UMP_STREAM_MSG_ENDPOINT_NAME = 0x03,
    UMP_STREAM_MSG_PRODUCT_INSTANCE_ID = 0x04,
    UMP_STREAM_MSG_STREAM_CONFIG_REQUEST = 0x05,
    UMP_STREAM_MSG_STREAM_CONFIG_NOTIFY = 0x06,
    UMP_STREAM_MSG_FUNCTION_BLOCK_DISCOVERY = 0x10,
    UMP_STREAM_MSG_FUNCTION_BLOCK_INFO = 0x11,
    UMP_STREAM_MSG_FUNCTION_BLOCK_NAME = 0x12
} ump_stream_msg_type_t;

// Endpoint Info capabilities
typedef struct {
    uint8_t ump_version_major;
    uint8_t ump_version_minor;
    uint8_t num_function_blocks;
    bool supports_midi_2_0;
    bool supports_midi_1_0;
    bool supports_rx_jitter_reduction;
    bool supports_tx_jitter_reduction;
} ump_endpoint_info_t;

// Device Identity
typedef struct {
    uint8_t manufacturer_id[3];  // 3-byte manufacturer SysEx ID
    uint8_t family_id[2];         // Device family
    uint8_t model_id[2];          // Device model
    uint8_t sw_revision[4];       // Software revision
} ump_device_identity_t;

// Function Block info
typedef struct {
    uint8_t block_id;             // Function Block number (0-31)
    uint8_t active;               // Active state
    uint8_t direction;            // 0=Unknown, 1=Input, 2=Output, 3=Bidirectional
    uint8_t midi1_port;           // MIDI 1.0 port (0=Not available, 1-255)
    uint8_t ui_hint;              // UI hint
    uint8_t first_group;          // First group (0-15)
    uint8_t num_groups;           // Number of groups spanned
    uint8_t midi_ci_version;      // MIDI-CI message version
    uint8_t max_sysex_size;       // Max SysEx size (0=Unknown, 1=128, 2=256, 3=512, etc.)
} ump_function_block_info_t;

// Protocol capabilities and negotiation
typedef enum {
    UMP_PROTOCOL_MIDI_1_0 = 0x01,   // MIDI 1.0 protocol
    UMP_PROTOCOL_MIDI_2_0 = 0x02    // MIDI 2.0 protocol
} ump_protocol_type_t;

typedef struct {
    uint8_t protocol;              // Current protocol (1=MIDI 1.0, 2=MIDI 2.0)
    bool rx_jitter_reduction;      // RX Jitter Reduction enabled
    bool tx_jitter_reduction;      // TX Jitter Reduction enabled
} ump_protocol_status_t;

// Stream Configuration
typedef struct {
    uint8_t protocol;              // Requested protocol
    bool rx_jitter_reduction;      // Request RX JR
    bool tx_jitter_reduction;      // Request TX JR
} ump_stream_config_t;

// MIDI-CI MUID (Manufacturer Unique ID)
typedef uint32_t muid_t;

/* Exported constants --------------------------------------------------------*/
#define UMP_VERSION_MAJOR     0x01
#define UMP_VERSION_MINOR     0x02

// Device constants
#define MANUFACTURER_ID_BYTE1 0x7D  // Educational/Development ID
#define MANUFACTURER_ID_BYTE2 0x00
#define MANUFACTURER_ID_BYTE3 0x00

#define DEVICE_FAMILY_ID_LSB  0x01
#define DEVICE_FAMILY_ID_MSB  0x00

#define DEVICE_MODEL_ID_LSB   0x01
#define DEVICE_MODEL_ID_MSB   0x00

// Software Version - Easy to manage (change these values to update version)
#define SW_VERSION_MAJOR      1     // Major version (0-127)
#define SW_VERSION_MINOR      0     // Minor version (0-127)  
#define SW_VERSION_PATCH      0     // Patch version (0-127)
#define SW_VERSION_BUILD      0     // Build number (0-127)

// Software Revision bytes for MIDI-CI (automatically derived from version numbers)
#define SW_REVISION_LEVEL1    (SW_VERSION_MAJOR & 0x7F)
#define SW_REVISION_LEVEL2    (SW_VERSION_MINOR & 0x7F)
#define SW_REVISION_LEVEL3    (SW_VERSION_PATCH & 0x7F)
#define SW_REVISION_LEVEL4    (SW_VERSION_BUILD & 0x7F)

// MIDI-CI constants
#define MIDI_CI_VERSION       0x02  // Version 1.2
#define MIDI_CI_CATEGORY      0x7E  // Universal System Exclusive Non-Real Time
#define MIDI_CI_SUB_ID        0x0D  // MIDI-CI

// MIDI-CI Sub-ID2 values
#define MIDI_CI_SUB_ID2_DISCOVERY           0x70
#define MIDI_CI_SUB_ID2_DISCOVERY_REPLY     0x71
#define MIDI_CI_SUB_ID2_INVALIDATE_MUID     0x7E
#define MIDI_CI_SUB_ID2_NAK                 0x7F

// MIDI-CI NAK Status Codes
#define MIDI_CI_NAK_STATUS_UNSUPPORTED      0x01  // CI message not supported
#define MIDI_CI_NAK_STATUS_CHANNEL_MSG      0x02  // Message received on wrong channel
#define MIDI_CI_NAK_STATUS_LIMIT_EXCEEDED   0x03  // Resource limitation
#define MIDI_CI_NAK_STATUS_UNKNOWN_MESSAGE  0x04  // Unknown CI message

// Function Block configuration
#define FB0_STATIC            1
#define NUM_FUNCTION_BLOCKS   1
#define FB0_FIRST_GROUP      0
#define FB0_NUM_GROUPS       1     // Single group only (used for nNumGroupTrm in USB descriptor)

// Device name strings - Easy to customize
#define UMP_ENDPOINT_NAME       "USB MIDI 2.0 Converter"   // Main endpoint name
#define UMP_PRODUCT_INSTANCE_ID "MIDI2USB-001"             // Unique product instance
#define UMP_FB0_NAME            "Main Port"                // Function Block 0 name

// MUID Configuration
#define MUID_FALLBACK_VALUE     0x12345678  // Default MUID if generation fails
#define MUID_USE_TICK_COUNT     1            // Use system tick count for MUID generation

/* Exported functions prototypes ---------------------------------------------*/

// Initialization
void UMP_Discovery_Init(void);

// Process incoming UMP Stream messages
void UMP_ProcessStreamMessage(uint32_t* ump_data, uint8_t word_count);
void UMP_ProcessDataMessage(uint32_t* ump_data, uint8_t word_count);

// Send discovery notifications
void UMP_SendEndpointInfoNotification(void);
void UMP_SendDeviceIdentityNotification(void);
void UMP_SendEndpointNameNotification(void);
void UMP_SendProductInstanceIdNotification(void);
void UMP_SendStreamConfigNotification(uint8_t protocol, bool rx_jr, bool tx_jr);

void UMP_SendFunctionBlockInfoNotification(uint8_t function_block_id);
void UMP_SendFunctionBlockNameNotification(uint8_t function_block_id);

// MIDI-CI functions
void MIDICI_ProcessDiscovery(uint8_t* sysex_data, uint16_t length, muid_t source_muid);
void MIDICI_SendDiscoveryReply(muid_t destination_muid);
void MIDICI_SendInvalidateMUID(muid_t old_muid);
void MIDICI_SendNAK(muid_t destination_muid, uint8_t original_sub_id, uint8_t status_code, uint8_t status_data);
muid_t MIDICI_GenerateMUID(void);


#ifdef __cplusplus
}
#endif

#endif /* __UMP_DISCOVERY_H__ */