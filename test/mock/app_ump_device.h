#ifndef __APP_UMP_DEVICE_H__
#define __APP_UMP_DEVICE_H__

#include <stdint.h>

// Mock definitions for app_ump_device.h

// Constants from app_ump_device.h needed by ump_discovery.c
#define NUM_FUNCTION_BLOCKS     1
#define FB0_FIRST_GROUP        0
#define FB0_NUM_GROUPS         1
#define FB0_STATIC             1    // Static function block
#define MIDI_CI_VERSION        0x02 // MIDI-CI version 1.2

#define DEVICE_MODEL_ID_LSB    0x01
#define DEVICE_MODEL_ID_MSB    0x00
#define SW_REVISION_LEVEL1     0x01
#define SW_REVISION_LEVEL2     0x00
#define SW_REVISION_LEVEL3     0x00
#define SW_REVISION_LEVEL4     0x00

#define UMP_ENDPOINT_NAME      "USB MIDI2 Converter"
#define UMP_PRODUCT_INSTANCE_ID "USBMIDI2-001"
#define UMP_FB0_NAME           "MIDI Port 1"

// MIDI-CI constants
#define MIDI_CI_CATEGORY           0x7E
#define MIDI_CI_SUB_ID            0x0D
#define MIDI_CI_SUB_ID2_DISCOVERY 0x70
#define MIDI_CI_SUB_ID2_DISCOVERY_REPLY 0x71
#define MIDI_CI_SUB_ID2_INVALIDATE_MUID 0x7E
#define MIDI_CI_SUB_ID2_NAK       0x7F
#define MIDI_CI_NAK_STATUS_UNSUPPORTED 0x01

// MUID fallback value
#define MUID_FALLBACK_VALUE        0x7E000000

// Function declarations
uint32_t MIDICI_GenerateMUID(void);
void MIDICI_ProcessDiscovery(uint8_t* sysex_data, uint16_t length, uint32_t source_muid);
void MIDICI_SendDiscoveryReply(uint32_t destination_muid);
void MIDICI_SendInvalidateMUID(uint32_t old_muid);
void MIDICI_SendNAK(uint32_t destination_muid, uint8_t sub_id2, uint8_t nak_status, uint8_t nak_details);

#endif /* __APP_UMP_DEVICE_H__ */