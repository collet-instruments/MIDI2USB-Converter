#ifndef __UMP_DISCOVERY_H__
#define __UMP_DISCOVERY_H__

#include <stdint.h>
#include <stdbool.h>

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
    uint8_t manufacturer_id[3];
    uint8_t family_id[2];
    uint8_t model_id[2];
    uint8_t sw_revision[4];
} ump_device_identity_t;

// Function Block info
typedef struct {
    uint8_t block_id;
    uint8_t active;
    uint8_t direction;
    uint8_t midi1_port;
    uint8_t ui_hint;
    uint8_t first_group;
    uint8_t num_groups;
    uint8_t midi_ci_version;
    uint8_t max_sysex_size;
} ump_function_block_info_t;

// Protocol types
typedef enum {
    UMP_PROTOCOL_MIDI_1_0 = 0x01,
    UMP_PROTOCOL_MIDI_2_0 = 0x02
} ump_protocol_type_t;

typedef struct {
    uint8_t protocol;
    bool rx_jitter_reduction;
    bool tx_jitter_reduction;
} ump_protocol_status_t;

// MIDI-CI MUID
typedef uint32_t muid_t;

// Constants
#define UMP_VERSION_MAJOR     0x01
#define UMP_VERSION_MINOR     0x02

#define MANUFACTURER_ID_BYTE1 0x7D
#define MANUFACTURER_ID_BYTE2 0x00
#define MANUFACTURER_ID_BYTE3 0x00

#define DEVICE_FAMILY_ID_LSB  0x01
#define DEVICE_FAMILY_ID_MSB  0x00

// Function declarations
void UMP_Discovery_Init(void);
void UMP_ProcessStreamMessage(uint32_t* ump_data, uint8_t word_count);
void UMP_ProcessDataMessage(uint32_t* ump_data, uint8_t word_count);
void UMP_SendEndpointInfoNotification(void);
void UMP_SendDeviceIdentityNotification(void);
void UMP_SendEndpointNameNotification(void);
void UMP_SendProductInstanceIdNotification(void);
void UMP_SendStreamConfigNotification(uint8_t protocol, bool rx_jr, bool tx_jr);
void UMP_SendFunctionBlockInfoNotification(uint8_t fb_id);
void UMP_SendFunctionBlockNameNotification(uint8_t fb_id);

// MIDI-CI functions
muid_t MIDICI_GenerateMUID(void);

#endif /* __UMP_DISCOVERY_H__ */