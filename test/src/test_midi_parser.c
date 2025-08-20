#include "test_common.h"
#include "midi_common.h"
#include <string.h>

// This test file tests the actual MIDI_GetExpectedLength function from Core/Src/midi_common.c
// The function is already exported in midi_common.h, so we can use it directly

// MIDI Parser State Machine Test
// This tests the MIDI parsing logic from uart_midi_task.c

// Parser state structure
typedef struct {
    uint8_t msg_buffer[3];
    uint8_t msg_index;
    uint8_t running_status;
    MIDIMessage_t last_message;
    bool message_ready;
} MidiParser_t;

// Initialize parser
static void MidiParser_Init(MidiParser_t* parser)
{
    memset(parser, 0, sizeof(MidiParser_t));
}

// Process a single MIDI byte (simplified from uart_midi_task.c)
static void MidiParser_ProcessByte(MidiParser_t* parser, uint8_t byte)
{
    parser->message_ready = false;
    
    // Real-time messages (0xF8-0xFF) - pass through immediately
    if (byte >= 0xF8) {
        parser->last_message.data[0] = byte;
        parser->last_message.length = 1;
        parser->message_ready = true;
        return;
    }
    
    // Status byte (0x80-0xF7)
    if (byte & 0x80) {
        // System Common messages (0xF0-0xF7) clear running status
        if (byte >= 0xF0) {
            parser->running_status = 0;
            parser->msg_index = 0;
            parser->msg_buffer[parser->msg_index++] = byte;
            
            // Single byte system common messages
            if (byte == 0xF6 || byte == 0xF7) {
                parser->last_message.data[0] = byte;
                parser->last_message.length = 1;
                parser->message_ready = true;
                parser->msg_index = 0;
            }
        } else {
            // Channel messages - set running status
            parser->running_status = byte;
            parser->msg_index = 0;
            parser->msg_buffer[parser->msg_index++] = byte;
        }
    } else {
        // Data byte
        if (parser->running_status && parser->msg_index == 0) {
            // Use running status
            parser->msg_buffer[parser->msg_index++] = parser->running_status;
        }
        
        if (parser->msg_index > 0 && parser->msg_index < 3) {
            parser->msg_buffer[parser->msg_index++] = byte;
            
            // Check if message is complete
            uint8_t expected_length = MIDI_GetExpectedLength(parser->msg_buffer[0]);
            if (parser->msg_index >= expected_length) {
                // Complete message
                memcpy(parser->last_message.data, parser->msg_buffer, parser->msg_index);
                parser->last_message.length = parser->msg_index;
                parser->message_ready = true;
                parser->msg_index = 0;
            }
        }
    }
}

// Test state
static MidiParser_t parser;

void setUp(void)
{
    MidiParser_Init(&parser);
}

void tearDown(void)
{
}

// Test MIDI_GetExpectedLength function from Core/Src/midi_common.c
void test_MIDI_GetExpectedLength_RealTimeMessages(void)
{
    // Real-time messages should return 1
    TEST_ASSERT_EQUAL_UINT8(1, MIDI_GetExpectedLength(0xF8));  // Timing Clock
    TEST_ASSERT_EQUAL_UINT8(1, MIDI_GetExpectedLength(0xFA));  // Start
    TEST_ASSERT_EQUAL_UINT8(1, MIDI_GetExpectedLength(0xFB));  // Continue
    TEST_ASSERT_EQUAL_UINT8(1, MIDI_GetExpectedLength(0xFC));  // Stop
    TEST_ASSERT_EQUAL_UINT8(1, MIDI_GetExpectedLength(0xFE));  // Active Sensing
    TEST_ASSERT_EQUAL_UINT8(1, MIDI_GetExpectedLength(0xFF));  // System Reset
}

void test_MIDI_GetExpectedLength_SystemCommon(void)
{
    TEST_ASSERT_EQUAL_UINT8(1, MIDI_GetExpectedLength(0xF0));  // SysEx Start
    TEST_ASSERT_EQUAL_UINT8(2, MIDI_GetExpectedLength(0xF1));  // MTC Quarter Frame
    TEST_ASSERT_EQUAL_UINT8(3, MIDI_GetExpectedLength(0xF2));  // Song Position Pointer
    TEST_ASSERT_EQUAL_UINT8(2, MIDI_GetExpectedLength(0xF3));  // Song Select
    TEST_ASSERT_EQUAL_UINT8(1, MIDI_GetExpectedLength(0xF4));  // Undefined
    TEST_ASSERT_EQUAL_UINT8(1, MIDI_GetExpectedLength(0xF5));  // Undefined
    TEST_ASSERT_EQUAL_UINT8(1, MIDI_GetExpectedLength(0xF6));  // Tune Request
    TEST_ASSERT_EQUAL_UINT8(1, MIDI_GetExpectedLength(0xF7));  // SysEx End
}

void test_MIDI_GetExpectedLength_ChannelVoice(void)
{
    // Note Off - 3 bytes
    TEST_ASSERT_EQUAL_UINT8(3, MIDI_GetExpectedLength(0x80));
    TEST_ASSERT_EQUAL_UINT8(3, MIDI_GetExpectedLength(0x8F));
    
    // Note On - 3 bytes
    TEST_ASSERT_EQUAL_UINT8(3, MIDI_GetExpectedLength(0x90));
    TEST_ASSERT_EQUAL_UINT8(3, MIDI_GetExpectedLength(0x9F));
    
    // Poly Key Pressure - 3 bytes
    TEST_ASSERT_EQUAL_UINT8(3, MIDI_GetExpectedLength(0xA0));
    TEST_ASSERT_EQUAL_UINT8(3, MIDI_GetExpectedLength(0xAF));
    
    // Control Change - 3 bytes
    TEST_ASSERT_EQUAL_UINT8(3, MIDI_GetExpectedLength(0xB0));
    TEST_ASSERT_EQUAL_UINT8(3, MIDI_GetExpectedLength(0xBF));
    
    // Program Change - 2 bytes
    TEST_ASSERT_EQUAL_UINT8(2, MIDI_GetExpectedLength(0xC0));
    TEST_ASSERT_EQUAL_UINT8(2, MIDI_GetExpectedLength(0xCF));
    
    // Channel Pressure - 2 bytes
    TEST_ASSERT_EQUAL_UINT8(2, MIDI_GetExpectedLength(0xD0));
    TEST_ASSERT_EQUAL_UINT8(2, MIDI_GetExpectedLength(0xDF));
    
    // Pitch Bend - 3 bytes
    TEST_ASSERT_EQUAL_UINT8(3, MIDI_GetExpectedLength(0xE0));
    TEST_ASSERT_EQUAL_UINT8(3, MIDI_GetExpectedLength(0xEF));
}

// Test real-time message processing
void test_MidiParser_RealTimeMessages(void)
{
    // Timing Clock (0xF8)
    MidiParser_ProcessByte(&parser, 0xF8);
    TEST_ASSERT_TRUE(parser.message_ready);
    TEST_ASSERT_EQUAL_UINT8(0xF8, parser.last_message.data[0]);
    TEST_ASSERT_EQUAL_UINT8(1, parser.last_message.length);
    
    // Start (0xFA)
    MidiParser_ProcessByte(&parser, 0xFA);
    TEST_ASSERT_TRUE(parser.message_ready);
    TEST_ASSERT_EQUAL_UINT8(0xFA, parser.last_message.data[0]);
    
    // Active Sensing (0xFE)
    MidiParser_ProcessByte(&parser, 0xFE);
    TEST_ASSERT_TRUE(parser.message_ready);
    TEST_ASSERT_EQUAL_UINT8(0xFE, parser.last_message.data[0]);
}

// Test Note On message
void test_MidiParser_NoteOn(void)
{
    // Note On, channel 1
    MidiParser_ProcessByte(&parser, 0x90);
    TEST_ASSERT_FALSE(parser.message_ready);
    
    // Note number
    MidiParser_ProcessByte(&parser, 0x3C);  // Middle C
    TEST_ASSERT_FALSE(parser.message_ready);
    
    // Velocity
    MidiParser_ProcessByte(&parser, 0x7F);
    TEST_ASSERT_TRUE(parser.message_ready);
    TEST_ASSERT_EQUAL_UINT8(0x90, parser.last_message.data[0]);
    TEST_ASSERT_EQUAL_UINT8(0x3C, parser.last_message.data[1]);
    TEST_ASSERT_EQUAL_UINT8(0x7F, parser.last_message.data[2]);
    TEST_ASSERT_EQUAL_UINT8(3, parser.last_message.length);
}

// Test Program Change (2-byte message)
void test_MidiParser_ProgramChange(void)
{
    // Program Change, channel 5
    MidiParser_ProcessByte(&parser, 0xC4);
    TEST_ASSERT_FALSE(parser.message_ready);
    
    // Program number
    MidiParser_ProcessByte(&parser, 0x42);
    TEST_ASSERT_TRUE(parser.message_ready);
    TEST_ASSERT_EQUAL_UINT8(0xC4, parser.last_message.data[0]);
    TEST_ASSERT_EQUAL_UINT8(0x42, parser.last_message.data[1]);
    TEST_ASSERT_EQUAL_UINT8(2, parser.last_message.length);
}

// Test Running Status
void test_MidiParser_RunningStatus(void)
{
    // First Note On
    MidiParser_ProcessByte(&parser, 0x90);
    MidiParser_ProcessByte(&parser, 0x3C);
    MidiParser_ProcessByte(&parser, 0x7F);
    TEST_ASSERT_TRUE(parser.message_ready);
    TEST_ASSERT_EQUAL_UINT8(0x90, parser.running_status);
    
    // Second Note On using running status (no status byte)
    parser.message_ready = false;
    MidiParser_ProcessByte(&parser, 0x3E);  // Note E
    TEST_ASSERT_FALSE(parser.message_ready);
    MidiParser_ProcessByte(&parser, 0x64);  // Velocity
    TEST_ASSERT_TRUE(parser.message_ready);
    TEST_ASSERT_EQUAL_UINT8(0x90, parser.last_message.data[0]);
    TEST_ASSERT_EQUAL_UINT8(0x3E, parser.last_message.data[1]);
    TEST_ASSERT_EQUAL_UINT8(0x64, parser.last_message.data[2]);
}

// Test System Common clears running status
void test_MidiParser_SystemCommonClearsRunningStatus(void)
{
    // Set running status with Note On
    MidiParser_ProcessByte(&parser, 0x90);
    MidiParser_ProcessByte(&parser, 0x3C);
    MidiParser_ProcessByte(&parser, 0x7F);
    TEST_ASSERT_EQUAL_UINT8(0x90, parser.running_status);
    
    // System Common message (Tune Request)
    MidiParser_ProcessByte(&parser, 0xF6);
    TEST_ASSERT_TRUE(parser.message_ready);
    TEST_ASSERT_EQUAL_UINT8(0xF6, parser.last_message.data[0]);
    TEST_ASSERT_EQUAL_UINT8(0, parser.running_status);  // Running status cleared
    
    // Try to use running status - should fail
    parser.message_ready = false;
    MidiParser_ProcessByte(&parser, 0x3E);  // Data byte
    TEST_ASSERT_FALSE(parser.message_ready);  // No message ready
    TEST_ASSERT_EQUAL_UINT8(0, parser.msg_index);  // Parser reset
}

// Test real-time messages don't affect parsing
void test_MidiParser_RealTimeDoesNotInterrupt(void)
{
    // Start Note On
    MidiParser_ProcessByte(&parser, 0x90);
    MidiParser_ProcessByte(&parser, 0x3C);
    
    // Real-time message in the middle
    MidiParser_ProcessByte(&parser, 0xF8);  // Timing Clock
    TEST_ASSERT_TRUE(parser.message_ready);
    TEST_ASSERT_EQUAL_UINT8(0xF8, parser.last_message.data[0]);
    
    // Continue Note On
    parser.message_ready = false;
    MidiParser_ProcessByte(&parser, 0x7F);  // Velocity
    TEST_ASSERT_TRUE(parser.message_ready);
    TEST_ASSERT_EQUAL_UINT8(0x90, parser.last_message.data[0]);
    TEST_ASSERT_EQUAL_UINT8(0x3C, parser.last_message.data[1]);
    TEST_ASSERT_EQUAL_UINT8(0x7F, parser.last_message.data[2]);
}

// Test Control Change
void test_MidiParser_ControlChange(void)
{
    // Control Change, channel 1
    MidiParser_ProcessByte(&parser, 0xB0);
    MidiParser_ProcessByte(&parser, 0x07);  // Volume
    MidiParser_ProcessByte(&parser, 0x64);  // Value 100
    
    TEST_ASSERT_TRUE(parser.message_ready);
    TEST_ASSERT_EQUAL_UINT8(0xB0, parser.last_message.data[0]);
    TEST_ASSERT_EQUAL_UINT8(0x07, parser.last_message.data[1]);
    TEST_ASSERT_EQUAL_UINT8(0x64, parser.last_message.data[2]);
    TEST_ASSERT_EQUAL_UINT8(3, parser.last_message.length);
}

// Test Pitch Bend
void test_MidiParser_PitchBend(void)
{
    // Pitch Bend, channel 1
    MidiParser_ProcessByte(&parser, 0xE0);
    MidiParser_ProcessByte(&parser, 0x00);  // LSB
    MidiParser_ProcessByte(&parser, 0x40);  // MSB (center)
    
    TEST_ASSERT_TRUE(parser.message_ready);
    TEST_ASSERT_EQUAL_UINT8(0xE0, parser.last_message.data[0]);
    TEST_ASSERT_EQUAL_UINT8(0x00, parser.last_message.data[1]);
    TEST_ASSERT_EQUAL_UINT8(0x40, parser.last_message.data[2]);
}

// Test invalid data bytes are ignored
void test_MidiParser_IgnoresInvalidDataBytes(void)
{
    // Send data bytes without status
    MidiParser_ProcessByte(&parser, 0x3C);
    MidiParser_ProcessByte(&parser, 0x7F);
    
    TEST_ASSERT_FALSE(parser.message_ready);
    TEST_ASSERT_EQUAL_UINT8(0, parser.msg_index);
}

// Main test runner
int main(void)
{
    UNITY_BEGIN();
    
    // Test MIDI_GetExpectedLength from Core
    RUN_TEST(test_MIDI_GetExpectedLength_RealTimeMessages);
    RUN_TEST(test_MIDI_GetExpectedLength_SystemCommon);
    RUN_TEST(test_MIDI_GetExpectedLength_ChannelVoice);
    
    // Test parser using Core's MIDI_GetExpectedLength
    RUN_TEST(test_MidiParser_RealTimeMessages);
    RUN_TEST(test_MidiParser_NoteOn);
    RUN_TEST(test_MidiParser_ProgramChange);
    RUN_TEST(test_MidiParser_RunningStatus);
    RUN_TEST(test_MidiParser_SystemCommonClearsRunningStatus);
    RUN_TEST(test_MidiParser_RealTimeDoesNotInterrupt);
    RUN_TEST(test_MidiParser_ControlChange);
    RUN_TEST(test_MidiParser_PitchBend);
    RUN_TEST(test_MidiParser_IgnoresInvalidDataBytes);
    
    return UNITY_END();
}