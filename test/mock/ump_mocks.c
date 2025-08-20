#include "ump_mocks.h"

// Mock implementations of UMP notification functions
void UMP_SendEndpointInfoNotification(void) {
    // Mock implementation - does nothing
}

void UMP_SendDeviceIdentityNotification(void) {
    // Mock implementation - does nothing
}

void UMP_SendEndpointNameNotification(void) {
    // Mock implementation - does nothing
}

void UMP_SendProductInstanceIdNotification(void) {
    // Mock implementation - does nothing
}

void UMP_SendFunctionBlockInfoNotification(uint8_t block_index) {
    (void)block_index; // Unused parameter
    // Mock implementation - does nothing
}

void UMP_SendFunctionBlockNameNotification(uint8_t block_index) {
    (void)block_index; // Unused parameter
    // Mock implementation - does nothing
}

void UMP_SendProtocolNotification(uint8_t protocol, bool jr_timestamp, bool jr_receive) {
    (void)protocol;     // Unused parameter
    (void)jr_timestamp; // Unused parameter
    (void)jr_receive;   // Unused parameter
    // Mock implementation - does nothing
}

void UMP_SendStreamConfigNotification(uint8_t protocol, bool jr_timestamp, bool jr_receive) {
    (void)protocol;     // Unused parameter
    (void)jr_timestamp; // Unused parameter
    (void)jr_receive;   // Unused parameter
    // Mock implementation - does nothing
}