/**
  * @file           : ump_discovery.c
  * @brief          : UMP Endpoint Discovery and MIDI-CI implementation
  */

/* Includes ------------------------------------------------------------------*/
#include "ump_discovery.h"
#include "app_ump_device.h"
#include "midi2_task.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>

/* Private variables ---------------------------------------------------------*/

// SysEx reassembly buffer for MIDI-CI
static uint8_t sysex_buffer[256];
static uint16_t sysex_length = 0;
static bool sysex_in_progress = false;

// Static device information
static const ump_endpoint_info_t endpoint_info = {
    .ump_version_major = UMP_VERSION_MAJOR,
    .ump_version_minor = UMP_VERSION_MINOR,
    .num_function_blocks = NUM_FUNCTION_BLOCKS,
    .supports_midi_2_0 = true,       // MIDI 2.0 only as specified
    .supports_midi_1_0 = false,      // No MIDI 1.0 support
    .supports_rx_jitter_reduction = false,  // No JR support
    .supports_tx_jitter_reduction = false   // No JR support
};

static const ump_device_identity_t device_identity = {
    .manufacturer_id = {MANUFACTURER_ID_BYTE1, MANUFACTURER_ID_BYTE2, MANUFACTURER_ID_BYTE3},
    .family_id = {DEVICE_FAMILY_ID_LSB, DEVICE_FAMILY_ID_MSB},
    .model_id = {DEVICE_MODEL_ID_LSB, DEVICE_MODEL_ID_MSB},
    .sw_revision = {SW_REVISION_LEVEL1, SW_REVISION_LEVEL2, SW_REVISION_LEVEL3, SW_REVISION_LEVEL4}
};

static const ump_function_block_info_t function_blocks[NUM_FUNCTION_BLOCKS] = {
    {
        .block_id = 0,  // Function Block IDs are 0-based
        .active = 1,
        .direction = 3,              // Bidirectional
        .midi1_port = 0,             // Not available (MIDI 2.0 only)
        .ui_hint = 0,                // Unknown
        .first_group = FB0_FIRST_GROUP,
        .num_groups = FB0_NUM_GROUPS,
        .midi_ci_version = MIDI_CI_VERSION,
        .max_sysex_size = 3          // 512 bytes
    }
};

// MIDI-CI state
static muid_t device_muid = 0;
static bool muid_initialized = false;
static bool discovery_reply_sent = false;  // Track if Discovery Reply has been sent

// Endpoint name and product instance ID
static const char endpoint_name[] = UMP_ENDPOINT_NAME;
static const char product_instance_id[] = UMP_PRODUCT_INSTANCE_ID;

// Current protocol status (MIDI 2.0 only, no JR support)
static ump_protocol_status_t current_protocol = {
    .protocol = UMP_PROTOCOL_MIDI_2_0,
    .rx_jitter_reduction = false,
    .tx_jitter_reduction = false
};

/* Private function prototypes -----------------------------------------------*/
static void SendUMPStreamMessage(uint32_t* data, uint8_t word_count);
static void SendSysExAsUMP(uint8_t* sysex_data, uint16_t length);
static uint32_t EncodeString7bit(const char* str, uint8_t* output, uint32_t max_bytes);

/* Public functions ----------------------------------------------------------*/

/**
  * @brief Initialize UMP Discovery module
  * @retval None
  */
void UMP_Discovery_Init(void)
{
    // Generate initial MUID
    device_muid = MIDICI_GenerateMUID();
    muid_initialized = true;
}

/**
  * @brief Process incoming UMP Stream messages
  * @param ump_data: Pointer to UMP message data
  * @param word_count: Number of 32-bit words
  * @retval None
  */
void UMP_ProcessStreamMessage(uint32_t* ump_data, uint8_t word_count)
{
    if (word_count < 1) return;
    
    // Extract message type from first word
    uint8_t mt = (ump_data[0] >> 28) & 0xF;
    if (mt != 0xF) return;  // Not a stream message
    
    uint16_t status = (ump_data[0] >> 16) & 0x3FF;
    uint8_t format = (ump_data[0] >> 26) & 0x3;
    
    switch (status) {
        case UMP_STREAM_MSG_ENDPOINT_DISCOVERY:
            {
                // Endpoint Discovery Request (4-word message)
                // Word 0: MT, Format, Status, UMP Version Major, UMP Version Minor
                uint8_t ump_major = (ump_data[0] >> 8) & 0xFF;
                uint8_t ump_minor = ump_data[0] & 0xFF;
                
                // Word 1: Reserved (bits 31-5), Filter bitmap (bits 4-0)
                uint8_t filter = ump_data[1] & 0x1F;  // Lower 5 bits
                
                // TODO: Could validate UMP version here if needed
                
                // Respond with requested notifications based on filter bitmap
                if (filter & 0x01) {  // Endpoint Info
                    UMP_SendEndpointInfoNotification();
                }
                if (filter & 0x02) {  // Device Identity
                    UMP_SendDeviceIdentityNotification();
                }
                if (filter & 0x04) {  // Endpoint Name
                    UMP_SendEndpointNameNotification();
                }
                if (filter & 0x08) {  // Product Instance ID
                    UMP_SendProductInstanceIdNotification();
                }
                if (filter & 0x10) {  // Stream Configuration
                    UMP_SendStreamConfigNotification(current_protocol.protocol, 
                                                    current_protocol.rx_jitter_reduction,
                                                    current_protocol.tx_jitter_reduction);
                }
                // Note: bits 0x20, 0x40, 0x80 are reserved for future use
            }
            break;

        case UMP_STREAM_MSG_STREAM_CONFIG_REQUEST:
            {
                UMP_SendStreamConfigNotification(current_protocol.protocol, 
                                                    current_protocol.rx_jitter_reduction,
                                                    current_protocol.tx_jitter_reduction);
            
            }
            break;
            
        case UMP_STREAM_MSG_FUNCTION_BLOCK_DISCOVERY:
            {
                // Function Block Discovery Request
                uint8_t fb_id = (ump_data[0] >> 8) & 0xFF;
                uint8_t filter = ump_data[0] & 0xFF;
                
                if (fb_id == 0xFF) {
                    // Request for all function blocks
                    for (uint8_t i = 0; i < NUM_FUNCTION_BLOCKS; i++) {
                        uint8_t fb_index = function_blocks[i].block_id;
                        if (filter & 0x01) {
                            UMP_SendFunctionBlockInfoNotification(fb_index);
                        }
                        if (filter & 0x02) {
                            UMP_SendFunctionBlockNameNotification(fb_index);
                        }
                    }
                } else if (fb_id < NUM_FUNCTION_BLOCKS) {
                    // Request for specific function block
                    if (filter & 0x01) {
                        UMP_SendFunctionBlockInfoNotification(fb_id);
                    }
                    if (filter & 0x02) {
                        UMP_SendFunctionBlockNameNotification(fb_id);
                    }
                }
            }
            break;
    }
}


/**
  * @brief Send Endpoint Info Notification
  * @retval None
  */
void UMP_SendEndpointInfoNotification(void)
{
    uint32_t ump_msg[4] = {0};
    
    // Format: MT=F, Format=0, Status=0x001, UMP Major, UMP Minor
    ump_msg[0] = (0xF << 28) | (0 << 26) | (UMP_STREAM_MSG_ENDPOINT_INFO << 16) |
                 (UMP_VERSION_MAJOR << 8) | UMP_VERSION_MINOR;
    
    // Data: Number of Function Blocks, Protocols supported, JR support
    ump_msg[1] = (FB0_STATIC << 31) | (NUM_FUNCTION_BLOCKS << 24) |
                 (endpoint_info.supports_midi_2_0 ? (1 << 9) : 0) |
                 (endpoint_info.supports_midi_1_0 ? (1 << 8) : 0) |
                 (endpoint_info.supports_rx_jitter_reduction ? (1 << 1) : 0) |
                 (endpoint_info.supports_tx_jitter_reduction ? (1 << 0) : 0);
    
    // Endpoint Info Notification is a 2-word message
    SendUMPStreamMessage(ump_msg, 4);
}

/**
  * @brief Send Device Identity Notification
  * @retval None
  */
void UMP_SendDeviceIdentityNotification(void)
{
    uint32_t ump_msg[4] = {0};
    
    // Format: MT=F, Format=0, Status=0x002
    ump_msg[0] = (0xF << 28) | (0 << 26) | (UMP_STREAM_MSG_DEVICE_IDENTITY << 16);
    
    // Manufacturer ID (3 bytes) + Family ID (2 bytes) + Model ID (2 bytes)
    ump_msg[1] = (device_identity.manufacturer_id[0] << 16) |
                 (device_identity.manufacturer_id[1] << 8) |
                 device_identity.manufacturer_id[2];
    
    ump_msg[2] = (device_identity.family_id[1] << 24) |
                 (device_identity.family_id[0] << 16) |
                 (device_identity.model_id[1] << 8) |
                 device_identity.model_id[0];
    
    // Software revision
    ump_msg[3] = (device_identity.sw_revision[0] << 24) |
                 (device_identity.sw_revision[1] << 16) |
                 (device_identity.sw_revision[2] << 8) |
                 device_identity.sw_revision[3];
    
    SendUMPStreamMessage(ump_msg, 4);
}

/**
  * @brief Send Endpoint Name Notification
  * @retval None
  */
void UMP_SendEndpointNameNotification(void)
{
    uint32_t ump_msg[4] = {0};
    size_t name_len = strlen(endpoint_name);
    
    if (name_len <= 14) {
        // Short name - send as Complete message (Form = 0x0)
        uint8_t name_7bit[14] = {0};
        uint32_t encoded_len = EncodeString7bit(endpoint_name, name_7bit, 14);
        
        // Format: MT=F, Format=0, Status=0x003
        ump_msg[0] = (0xF << 28) | (0x0 << 26) | (UMP_STREAM_MSG_ENDPOINT_NAME << 16);
        if (encoded_len >= 1) ump_msg[0] |= (name_7bit[0] << 8);
        if (encoded_len >= 2) ump_msg[0] |= name_7bit[1];
        
        // Pack remaining bytes
        for (int i = 2; i < encoded_len; i++) {
            int word_idx = 1 + ((i - 2) / 4);
            int byte_idx = 3 - ((i - 2) % 4);
            ump_msg[word_idx] |= (name_7bit[i] << (byte_idx * 8));
        }
        
        SendUMPStreamMessage(ump_msg, 4);
    } else {
        // Long name - send as multiple messages
        size_t offset = 0;
        uint8_t form = 0x1;  // Start with Form = 0x1 (Start)
        
        while (offset < name_len && offset < 98) {  // Max 98 bytes total
            uint8_t chunk[14] = {0};
            size_t chunk_len = (name_len - offset < 14) ? (name_len - offset) : 14;
            
            // Copy chunk and encode to 7-bit
            for (size_t i = 0; i < chunk_len; i++) {
                chunk[i] = endpoint_name[offset + i] & 0x7F;
            }
            
            // Build message
            memset(ump_msg, 0, sizeof(ump_msg));
            ump_msg[0] = (0xF << 28) | (form << 26) | (UMP_STREAM_MSG_ENDPOINT_NAME << 16);
            if (chunk_len >= 1) ump_msg[0] |= (chunk[0] << 8);
            if (chunk_len >= 2) ump_msg[0] |= chunk[1];
            
            // Pack remaining bytes
            for (int i = 2; i < 14; i++) {
                int word_idx = 1 + ((i - 2) / 4);
                int byte_idx = 3 - ((i - 2) % 4);
                ump_msg[word_idx] |= (chunk[i] << (byte_idx * 8));
            }
            
            SendUMPStreamMessage(ump_msg, 4);
            
            offset += 14;
            
            // Update form for next message
            if (offset >= name_len) {
                break;  // Done
            } else if (form == 0x1) {
                form = 0x3;  // After Start, next is End (for 2-part messages)
                if (offset + 14 < name_len) {
                    form = 0x2;  // Actually need Continue
                }
            } else if (form == 0x2) {
                // Check if this is the last chunk
                if (offset + 14 >= name_len) {
                    form = 0x3;  // End
                }
            }
        }
    }
}

/**
  * @brief Send Product Instance ID Notification
  * @retval None
  */
void UMP_SendProductInstanceIdNotification(void)
{
    uint32_t ump_msg[4] = {0};
    size_t id_len = strlen(product_instance_id);
    
    if (id_len <= 14) {
        // Short ID - send as Complete message (Form = 0x0)
        uint8_t id_7bit[14] = {0};
        uint32_t encoded_len = EncodeString7bit(product_instance_id, id_7bit, 14);
        
        // Format: MT=F, Format=0, Status=0x004
        ump_msg[0] = (0xF << 28) | (0x0 << 26) | (UMP_STREAM_MSG_PRODUCT_INSTANCE_ID << 16);
        if (encoded_len >= 1) ump_msg[0] |= (id_7bit[0] << 8);
        if (encoded_len >= 2) ump_msg[0] |= id_7bit[1];
        
        // Pack remaining bytes
        for (int i = 2; i < encoded_len; i++) {
            int word_idx = 1 + ((i - 2) / 4);
            int byte_idx = 3 - ((i - 2) % 4);
            ump_msg[word_idx] |= (id_7bit[i] << (byte_idx * 8));
        }
        
        SendUMPStreamMessage(ump_msg, 4);
    } else {
        // Long ID - send as multiple messages (similar to Endpoint Name)
        size_t offset = 0;
        uint8_t form = 0x1;  // Start with Form = 0x1 (Start)
        
        while (offset < id_len && offset < 98) {  // Max 98 bytes total
            uint8_t chunk[14] = {0};
            size_t chunk_len = (id_len - offset < 14) ? (id_len - offset) : 14;
            
            // Copy chunk and encode to 7-bit
            for (size_t i = 0; i < chunk_len; i++) {
                chunk[i] = product_instance_id[offset + i] & 0x7F;
            }
            
            // Build message
            memset(ump_msg, 0, sizeof(ump_msg));
            ump_msg[0] = (0xF << 28) | (form << 26) | (UMP_STREAM_MSG_PRODUCT_INSTANCE_ID << 16);
            if (chunk_len >= 1) ump_msg[0] |= (chunk[0] << 8);
            if (chunk_len >= 2) ump_msg[0] |= chunk[1];
            
            // Pack remaining bytes
            for (int i = 2; i < 14; i++) {
                int word_idx = 1 + ((i - 2) / 4);
                int byte_idx = 3 - ((i - 2) % 4);
                ump_msg[word_idx] |= (chunk[i] << (byte_idx * 8));
            }
            
            SendUMPStreamMessage(ump_msg, 4);
            
            offset += 14;
            
            // Update form for next message
            if (offset >= id_len) {
                break;  // Done
            } else if (form == 0x1) {
                form = 0x3;  // After Start, next is End (for 2-part messages)
                if (offset + 14 < id_len) {
                    form = 0x2;  // Actually need Continue
                }
            } else if (form == 0x2) {
                // Check if this is the last chunk
                if (offset + 14 >= id_len) {
                    form = 0x3;  // End
                }
            }
        }
    }
}

/**
  * @brief Send Function Block Info Notification
  * @param function_block_id: Function block ID
  * @retval None
  */
void UMP_SendFunctionBlockInfoNotification(uint8_t function_block_id)
{
    // Function Block IDs are 0-based
    if (function_block_id >= NUM_FUNCTION_BLOCKS) return;
    
    const ump_function_block_info_t* fb = &function_blocks[function_block_id];
    uint32_t ump_msg[4] = {0};
    
    // Format: MT=F, Format=0, Status=0x011, Active, FB#, recv, sender, MIDI1, direction
    ump_msg[0] = (0xF << 28) | (0 << 26) | (UMP_STREAM_MSG_FUNCTION_BLOCK_INFO << 16) |
                 (fb->active << 15) | (fb->block_id << 8) |
                 (1 << 5) | (1 << 4) | (0 << 2) | (fb->direction & 0x3);
    
    // First Group, Number of Groups, MIDI-CI Support, Max S8 Streams
    ump_msg[1] = ((fb->first_group & 0xFF) << 24) | ((fb->num_groups & 0xFF) << 16) |
                 ((fb->midi_ci_version & 0xFF) << 8) | 0;
    
    // Word 2 and 3 are reserved
    ump_msg[2] = 0;
    ump_msg[3] = 0;
    
    SendUMPStreamMessage(ump_msg, 4);
}

/**
  * @brief Send Function Block Name Notification
  * @param function_block_id: Function block ID
  * @retval None
  */
void UMP_SendFunctionBlockNameNotification(uint8_t function_block_id)
{
    // Function Block IDs are 0-based
    if (function_block_id >= NUM_FUNCTION_BLOCKS) return;
    
    uint32_t ump_msg[4] = {0};
    uint8_t name_7bit[13] = {0};  // Max 13 bytes for FB name
    const char* fb_name = UMP_FB0_NAME;
    
    // Encode name to 7-bit
    uint32_t encoded_len = EncodeString7bit(fb_name, name_7bit, 13);
    
    // Get the function block info
    const ump_function_block_info_t* fb = &function_blocks[function_block_id];
    
    // Format: MT=F, Format=0, Status=0x012, FB#
    ump_msg[0] = (0xF << 28) | (0 << 26) | (UMP_STREAM_MSG_FUNCTION_BLOCK_NAME << 16) |
                 (fb->block_id << 8);
    
    // Pack name starting from byte 1 of first word
    if (encoded_len >= 1) ump_msg[0] |= name_7bit[0];
    
    // Pack remaining bytes
    uint32_t bytes_to_pack = (encoded_len < 13) ? encoded_len : 13;
    for (int i = 1; i < bytes_to_pack; i++) {
        int word_idx = 1 + ((i - 1) / 4);
        int byte_idx = 3 - ((i - 1) % 4);
        ump_msg[word_idx] |= (name_7bit[i] << (byte_idx * 8));
    }
    
    SendUMPStreamMessage(ump_msg, 4);
}

/**
  * @brief Process UMP Data Message (MT=3) containing SysEx
  * @param ump_data: UMP message data
  * @param word_count: Number of words (should be 2 for MT=3)
  * @retval None
  */
void UMP_ProcessDataMessage(uint32_t* ump_data, uint8_t word_count)
{
    if (word_count < 2) return;
    
    // Extract header from first word
    uint8_t mt = (ump_data[0] >> 28) & 0xF;
    if (mt != 0x3) return;  // Not a Data Message
    
    uint8_t group = (ump_data[0] >> 24) & 0xF;
    uint8_t status = (ump_data[0] >> 20) & 0xF;
    uint8_t num_bytes = (ump_data[0] >> 16) & 0xF;
    
    // Extract data bytes
    uint8_t data[6];
    data[0] = (ump_data[0] >> 8) & 0xFF;
    data[1] = ump_data[0] & 0xFF;
    data[2] = (ump_data[1] >> 24) & 0xFF;
    data[3] = (ump_data[1] >> 16) & 0xFF;
    data[4] = (ump_data[1] >> 8) & 0xFF;
    data[5] = ump_data[1] & 0xFF;
    
    // Process based on status
    switch (status) {
        case 0:  // Complete SysEx in one packet
            // Reset buffer
            sysex_length = 0;
            sysex_in_progress = false;
            
            // Copy data (F0 is implied, not included)
            for (uint8_t i = 0; i < num_bytes && i < 6; i++) {
                if (data[i] != 0xF7) {  // Don't copy F7
                    sysex_buffer[sysex_length++] = data[i];
                }
            }
            
            // Process complete SysEx for MIDI-CI
            if (sysex_length >= 4) {
                // Check for MIDI-CI message
                if (sysex_buffer[0] == MIDI_CI_CATEGORY &&  // Universal Non-Real Time
                    sysex_buffer[2] == MIDI_CI_SUB_ID) {     // MIDI-CI
                    
                    uint8_t sub_id2 = sysex_buffer[3];
                    
                    // Check for Discovery message (sub-ID 0x70)
                    if (sub_id2 == MIDI_CI_SUB_ID2_DISCOVERY && sysex_length >= 17) {
                        // Extract source MUID (bytes 5-8, LSB first)
                        muid_t source_muid = (sysex_buffer[5] & 0x7F) |          // LSB
                                           ((sysex_buffer[6] & 0x7F) << 7) |
                                           ((sysex_buffer[7] & 0x7F) << 14) |
                                           ((sysex_buffer[8] & 0x7F) << 21);     // MSB
                        
                        // Process Discovery
                        MIDICI_ProcessDiscovery(sysex_buffer, sysex_length, source_muid);
                    }
                    // Check for unsupported MIDI-CI messages
                    else if (sub_id2 != MIDI_CI_SUB_ID2_DISCOVERY_REPLY && 
                             sub_id2 != MIDI_CI_SUB_ID2_INVALIDATE_MUID &&
                             sub_id2 != MIDI_CI_SUB_ID2_NAK &&
                             sysex_length >= 13) {
                        // Extract source MUID for NAK response
                        muid_t source_muid = (sysex_buffer[5] & 0x7F) |          // LSB
                                           ((sysex_buffer[6] & 0x7F) << 7) |
                                           ((sysex_buffer[7] & 0x7F) << 14) |
                                           ((sysex_buffer[8] & 0x7F) << 21);     // MSB
                        
                        // Send NAK for unsupported message
                        MIDICI_SendNAK(source_muid, sub_id2, MIDI_CI_NAK_STATUS_UNSUPPORTED, 0x00);
                    }
                }
            }
            break;
            
        case 1:  // Start of SysEx
            // Reset buffer
            sysex_length = 0;
            sysex_in_progress = true;
            
            // Copy data (F0 is implied, not included)
            for (uint8_t i = 0; i < num_bytes && i < 6; i++) {
                sysex_buffer[sysex_length++] = data[i];
            }
            break;
            
        case 2:  // Continue SysEx
            if (sysex_in_progress && sysex_length < sizeof(sysex_buffer)) {
                // Continue adding data
                for (uint8_t i = 0; i < num_bytes && i < 6; i++) {
                    if (sysex_length < sizeof(sysex_buffer)) {
                        sysex_buffer[sysex_length++] = data[i];
                    }
                }
            }
            break;
            
        case 3:  // End of SysEx
            if (sysex_in_progress) {
                // Add final data (F7 is implied, not included)
                for (uint8_t i = 0; i < num_bytes && i < 6; i++) {
                    if (data[i] != 0xF7 && sysex_length < sizeof(sysex_buffer)) {
                        sysex_buffer[sysex_length++] = data[i];
                    }
                }
                
                // Process complete SysEx for MIDI-CI
                if (sysex_length >= 4) {
                    // Check for MIDI-CI message
                    if (sysex_buffer[0] == MIDI_CI_CATEGORY &&  // Universal Non-Real Time
                        sysex_buffer[2] == MIDI_CI_SUB_ID) {     // MIDI-CI
                        
                        // Check for Discovery message (sub-ID 0x70)
                        if (sysex_buffer[3] == 0x70 && sysex_length >= 17) {
                            // Extract source MUID (bytes 5-8)
                            muid_t source_muid = ((sysex_buffer[5] & 0x7F) << 21) |
                                               ((sysex_buffer[6] & 0x7F) << 14) |
                                               ((sysex_buffer[7] & 0x7F) << 7) |
                                               (sysex_buffer[8] & 0x7F);
                            
                            // Process Discovery
                            MIDICI_ProcessDiscovery(sysex_buffer, sysex_length, source_muid);
                        }
                    }
                }
                
                sysex_in_progress = false;
            }
            break;
    }
}

/**
  * @brief Process MIDI-CI Discovery message
  * @param sysex_data: SysEx data buffer
  * @param length: Data length
  * @param source_muid: Source MUID from message
  * @retval None
  */
void MIDICI_ProcessDiscovery(uint8_t* sysex_data, uint16_t length, muid_t source_muid)
{
    // Validate minimum length for discovery
    if (length < 17) return;  // Minimum CI message length
    
    // Check if source MUID conflicts with ours
    if (source_muid == device_muid) {
        // MUID conflict - generate new MUID
        muid_t old_muid = device_muid;
        device_muid = MIDICI_GenerateMUID();
        
        // Send Invalidate MUID message
        MIDICI_SendInvalidateMUID(old_muid);
    } else {
        // Send Discovery Reply
        MIDICI_SendDiscoveryReply(source_muid);
    }
}

/**
  * @brief Send MIDI-CI Discovery Reply
  * @param destination_muid: Destination MUID
  * @retval None
  */
void MIDICI_SendDiscoveryReply(muid_t destination_muid)
{
    // Clear any pending MIDI messages in the queue to ensure Discovery messages go first
    if (xUmpTxQueue != NULL) {
        xQueueReset(xUmpTxQueue);
    }
    
    // Mark that Discovery Reply has been sent
    discovery_reply_sent = true;
    
    // Prepare and send MIDI-CI Discovery Reply via UMP Data Messages
    uint8_t sysex_msg[40];  // Increased for MIDI-CI Version 2 fields
    uint16_t msg_len = 0;
    
    // SysEx header
    sysex_msg[msg_len++] = 0xF0;  // SysEx start
    sysex_msg[msg_len++] = MIDI_CI_CATEGORY;  // Universal Non-Real Time (0x7E)
    sysex_msg[msg_len++] = 0x7F;  // Device ID: 7F = from Function Block
    sysex_msg[msg_len++] = MIDI_CI_SUB_ID;  // MIDI-CI (0x0D)
    sysex_msg[msg_len++] = 0x71;  // Discovery Reply sub-ID
    sysex_msg[msg_len++] = MIDI_CI_VERSION;  // CI Version
    
    // Source MUID (our MUID) - LSB first
    sysex_msg[msg_len++] = device_muid & 0x7F;          // LSB
    sysex_msg[msg_len++] = (device_muid >> 7) & 0x7F;
    sysex_msg[msg_len++] = (device_muid >> 14) & 0x7F;
    sysex_msg[msg_len++] = (device_muid >> 21) & 0x7F;  // MSB
    
    // Destination MUID - LSB first
    sysex_msg[msg_len++] = destination_muid & 0x7F;          // LSB
    sysex_msg[msg_len++] = (destination_muid >> 7) & 0x7F;
    sysex_msg[msg_len++] = (destination_muid >> 14) & 0x7F;
    sysex_msg[msg_len++] = (destination_muid >> 21) & 0x7F;  // MSB
    
    // Device Manufacturer (3 bytes)
    sysex_msg[msg_len++] = device_identity.manufacturer_id[0];
    sysex_msg[msg_len++] = device_identity.manufacturer_id[1];
    sysex_msg[msg_len++] = device_identity.manufacturer_id[2];
    
    // Device Family (2 bytes - LSB first)
    sysex_msg[msg_len++] = device_identity.family_id[0];  // LSB
    sysex_msg[msg_len++] = device_identity.family_id[1];  // MSB
    
    // Device Model (2 bytes - LSB first)
    sysex_msg[msg_len++] = device_identity.model_id[0];  // LSB
    sysex_msg[msg_len++] = device_identity.model_id[1];  // MSB
    
    // Software Revision (4 bytes)
    sysex_msg[msg_len++] = device_identity.sw_revision[0] & 0x7F;
    sysex_msg[msg_len++] = device_identity.sw_revision[1] & 0x7F;
    sysex_msg[msg_len++] = device_identity.sw_revision[2] & 0x7F;
    sysex_msg[msg_len++] = device_identity.sw_revision[3] & 0x7F;
    
    // CI Category Supported - only Discovery
    sysex_msg[msg_len++] = 0x01;  // Discovery supported
    
    // Receivable Maximum SysEx Message Size (4 bytes - LSB first)
    // 512 bytes = 0x0200
    sysex_msg[msg_len++] = 0x00;  // LSB
    sysex_msg[msg_len++] = 0x02;  // 
    sysex_msg[msg_len++] = 0x00;  // 
    sysex_msg[msg_len++] = 0x00;  // MSB
    
    // Initiator's Output Path Instance Id (from Discovery message)
    sysex_msg[msg_len++] = 0x00;  // Use 0 as default
    
    // Function Block (0x7F = no Function Block, or 0x00 for FB0)
    sysex_msg[msg_len++] = 0x00;  // Function Block 0
    
    sysex_msg[msg_len++] = 0xF7;  // SysEx end
    
    // Convert SysEx to UMP and send
    SendSysExAsUMP(sysex_msg, msg_len);
}

/**
  * @brief Send MIDI-CI Invalidate MUID message
  * @param old_muid: Old MUID being invalidated
  * @retval None
  */
void MIDICI_SendInvalidateMUID(muid_t old_muid)
{
    // Prepare and send MIDI-CI Invalidate MUID via UMP Data Messages
    uint8_t sysex_msg[32];
    uint16_t msg_len = 0;
    
    // SysEx header
    sysex_msg[msg_len++] = 0xF0;  // SysEx start
    sysex_msg[msg_len++] = MIDI_CI_CATEGORY;  // Universal Non-Real Time
    sysex_msg[msg_len++] = 0x7F;  // Device ID (broadcast)
    sysex_msg[msg_len++] = MIDI_CI_SUB_ID;  // MIDI-CI
    sysex_msg[msg_len++] = 0x7E;  // Invalidate MUID sub-ID
    sysex_msg[msg_len++] = MIDI_CI_VERSION;  // CI Version
    
    // Source MUID (our new MUID) - LSB first
    sysex_msg[msg_len++] = device_muid & 0x7F;          // LSB
    sysex_msg[msg_len++] = (device_muid >> 7) & 0x7F;
    sysex_msg[msg_len++] = (device_muid >> 14) & 0x7F;
    sysex_msg[msg_len++] = (device_muid >> 21) & 0x7F;  // MSB
    
    // Broadcast MUID (all 7F)
    sysex_msg[msg_len++] = 0x7F;
    sysex_msg[msg_len++] = 0x7F;
    sysex_msg[msg_len++] = 0x7F;
    sysex_msg[msg_len++] = 0x7F;
    
    // Target MUID (old MUID being invalidated) - LSB first
    sysex_msg[msg_len++] = old_muid & 0x7F;          // LSB
    sysex_msg[msg_len++] = (old_muid >> 7) & 0x7F;
    sysex_msg[msg_len++] = (old_muid >> 14) & 0x7F;
    sysex_msg[msg_len++] = (old_muid >> 21) & 0x7F;  // MSB
    
    sysex_msg[msg_len++] = 0xF7;  // SysEx end
    
    // Convert SysEx to UMP and send
    SendSysExAsUMP(sysex_msg, msg_len);
}

/**
  * @brief Send MIDI-CI NAK message
  * @param destination_muid: Destination MUID
  * @param original_sub_id: The Sub-ID2 of the message being NAK'd
  * @param status_code: NAK status code
  * @param status_data: Additional status data
  * @retval None
  */
void MIDICI_SendNAK(muid_t destination_muid, uint8_t original_sub_id, uint8_t status_code, uint8_t status_data)
{
    // Prepare and send MIDI-CI NAK via UMP Data Messages
    uint8_t sysex_msg[32];
    uint16_t msg_len = 0;
    
    // SysEx header
    sysex_msg[msg_len++] = 0xF0;  // SysEx start
    sysex_msg[msg_len++] = MIDI_CI_CATEGORY;  // Universal Non-Real Time
    sysex_msg[msg_len++] = 0x7F;  // Device ID: to Function Block
    sysex_msg[msg_len++] = MIDI_CI_SUB_ID;  // MIDI-CI
    sysex_msg[msg_len++] = MIDI_CI_SUB_ID2_NAK;  // NAK sub-ID (0x7F)
    sysex_msg[msg_len++] = MIDI_CI_VERSION;  // CI Version
    
    // Source MUID (our MUID) - LSB first
    sysex_msg[msg_len++] = device_muid & 0x7F;          // LSB
    sysex_msg[msg_len++] = (device_muid >> 7) & 0x7F;
    sysex_msg[msg_len++] = (device_muid >> 14) & 0x7F;
    sysex_msg[msg_len++] = (device_muid >> 21) & 0x7F;  // MSB
    
    // Destination MUID - LSB first
    sysex_msg[msg_len++] = destination_muid & 0x7F;          // LSB
    sysex_msg[msg_len++] = (destination_muid >> 7) & 0x7F;
    sysex_msg[msg_len++] = (destination_muid >> 14) & 0x7F;
    sysex_msg[msg_len++] = (destination_muid >> 21) & 0x7F;  // MSB
    
    // Original Transaction Sub-ID#2 Classification
    sysex_msg[msg_len++] = original_sub_id;
    
    // ACK Status Code and Data
    sysex_msg[msg_len++] = status_code;
    sysex_msg[msg_len++] = status_data;
    
    // ACK details (5 bytes - set to 0 for now)
    for (int i = 0; i < 5; i++) {
        sysex_msg[msg_len++] = 0x00;
    }
    
    // Message Length (2 bytes LSB first) - no message text
    sysex_msg[msg_len++] = 0x00;  // LSB
    sysex_msg[msg_len++] = 0x00;  // MSB
    
    // No message text
    
    sysex_msg[msg_len++] = 0xF7;  // SysEx end
    
    // Convert SysEx to UMP and send
    SendSysExAsUMP(sysex_msg, msg_len);
}

/**
  * @brief Generate a new MUID
  * @retval New MUID
  */
muid_t MIDICI_GenerateMUID(void)
{
    uint32_t muid;
    
#if MUID_USE_TICK_COUNT
    // Generate a 28-bit MUID using system tick and task tick
    uint32_t tick = xTaskGetTickCount();
    muid = (tick & 0x0FFFFFFF);  // Use lower 28 bits
#else
    // Use static fallback value
    static uint32_t muid_counter = MUID_FALLBACK_VALUE;
    muid = (muid_counter++ & 0x0FFFFFFF);
#endif
    
    // Ensure non-zero and not broadcast
    if (muid == 0 || muid == 0x0FFFFFFF) {
        muid = MUID_FALLBACK_VALUE;  // Use configured fallback value
    }
    
    return muid;
}


/* Private functions ---------------------------------------------------------*/

/**
  * @brief Send UMP Stream message to USB
  * @param data: UMP message data
  * @param word_count: Number of 32-bit words
  * @retval None
  */
static void SendUMPStreamMessage(uint32_t* data, uint8_t word_count)
{
    // Send to UMP TX queue
    if (xUmpTxQueue != NULL) {
        // Use SendToBack to maintain correct message order
        // Discovery messages must be sent in sequence
        xQueueSendToBack(xUmpTxQueue, data, pdMS_TO_TICKS(10));
    }
}

/**
  * @brief Send SysEx message as UMP Data Messages (MT=3)
  * @param sysex_data: SysEx data including F0 and F7
  * @param length: Length of SysEx data
  * @retval None
  */
static void SendSysExAsUMP(uint8_t* sysex_data, uint16_t length)
{
    uint32_t ump_msg[2];
    uint8_t group = 0;  // Use Group 0 for MIDI-CI
    uint16_t pos = 0;
    
    while (pos < length) {
        uint8_t status;
        uint8_t bytes_in_packet = 0;
        uint8_t data_bytes[6] = {0};
        
        // Determine status and payload size
        if (pos == 0) {
            // First packet
            if (length <= 8) {  // F0 + up to 6 data bytes + F7
                // Complete SysEx in one packet
                status = 0;  // Complete
                bytes_in_packet = length - 2;  // Exclude F0 and F7
            } else {
                // Start of multi-packet SysEx
                status = 1;  // Start
                bytes_in_packet = 6;
            }
            pos++;  // Skip F0
        } else {
            // Calculate remaining bytes (excluding F7)
            uint16_t remaining = length - pos - 1;
            
            if (remaining <= 6) {
                // Last packet
                status = 3;  // End
                bytes_in_packet = remaining;
            } else {
                // Middle packet
                status = 2;  // Continue
                bytes_in_packet = 6;
            }
        }
        
        // Copy SysEx bytes (without F0/F7)
        for (uint8_t i = 0; i < bytes_in_packet; i++) {
            if (pos < length && sysex_data[pos] != 0xF7) {
                data_bytes[i] = sysex_data[pos++];
            }
        }
        
        // Skip F7 if at end
        if (pos < length && sysex_data[pos] == 0xF7) {
            pos++;
        }
        
        // Construct UMP Data Message (MT=3)
        // Word 0: MT, Group, Status, number of bytes, first 2 bytes
        ump_msg[0] = (0x3 << 28) |                    // MT = 3 (Data Message)
                     (group << 24) |                   // Group
                     (status << 20) |                  // Status
                     (bytes_in_packet << 16) |         // Number of valid bytes
                     (data_bytes[0] << 8) |            // Byte 1
                     data_bytes[1];                    // Byte 2
        
        // Word 1: remaining 4 bytes
        ump_msg[1] = (data_bytes[2] << 24) |          // Byte 3
                     (data_bytes[3] << 16) |          // Byte 4
                     (data_bytes[4] << 8) |           // Byte 5
                     data_bytes[5];                   // Byte 6
        
        // Send UMP message
        if (xUmpTxQueue != NULL) {
            xQueueSendToBack(xUmpTxQueue, ump_msg, pdMS_TO_TICKS(10));
        }
    }
}

/**
  * @brief Encode string to 7-bit format
  * @param str: Input string
  * @param output: Output buffer
  * @param max_bytes: Maximum output bytes
  * @retval Number of encoded bytes
  */
static uint32_t EncodeString7bit(const char* str, uint8_t* output, uint32_t max_bytes)
{
    uint32_t len = strlen(str);
    uint32_t encoded_len = 0;
    
    for (uint32_t i = 0; i < len && encoded_len < max_bytes; i++) {
        // Convert to 7-bit by masking MSB
        output[encoded_len++] = str[i] & 0x7F;
    }
    
    // Pad with zeros
    while (encoded_len < max_bytes) {
        output[encoded_len++] = 0;
    }
    
    return encoded_len;
}

/**
  * @brief Send Stream Configuration Notification message
  * @param protocol: Configured protocol
  * @param rx_jr: RX Jitter Reduction configured
  * @param tx_jr: TX Jitter Reduction configured
  * @retval None
  */
void UMP_SendStreamConfigNotification(uint8_t protocol, bool rx_jr, bool tx_jr)
{
    uint32_t ump_msg[4] = {0};
    
    // Format: MT=F, Format=0, Status=0x006, Protocol, JR flags
    // Stream Config Notification is a 1-word message
    ump_msg[0] = (0xF << 28) | (0 << 26) | (UMP_STREAM_MSG_STREAM_CONFIG_NOTIFY << 16) |
                 (protocol << 8) | (rx_jr ? 0x02 : 0) | (tx_jr ? 0x01 : 0);
    
    SendUMPStreamMessage(ump_msg, 4);
}