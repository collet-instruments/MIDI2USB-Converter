#ifndef __UMP_MOCKS_H__
#define __UMP_MOCKS_H__

#include <stdint.h>
#include <stdbool.h>

// UMP Protocol definitions
#define UMP_PROTOCOL_MIDI_1_0 0x01
#define UMP_PROTOCOL_MIDI_2_0 0x02

// Number of function blocks
#define NUM_FUNCTION_BLOCKS 2

// Function Block configuration (needed for USB descriptors)
#define FB0_FIRST_GROUP      0
#define FB0_NUM_GROUPS       1

// Mock UMP notification functions
void UMP_SendEndpointInfoNotification(void);
void UMP_SendDeviceIdentityNotification(void);
void UMP_SendEndpointNameNotification(void);
void UMP_SendProductInstanceIdNotification(void);
void UMP_SendFunctionBlockInfoNotification(uint8_t block_index);
void UMP_SendFunctionBlockNameNotification(uint8_t block_index);
void UMP_SendProtocolNotification(uint8_t protocol, bool jr_timestamp, bool jr_receive);
void UMP_SendStreamConfigNotification(uint8_t protocol, bool jr_timestamp, bool jr_receive);

#endif /* __UMP_MOCKS_H__ */