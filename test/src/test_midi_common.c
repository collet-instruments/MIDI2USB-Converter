#include "test_common.h"
#include "mock_freertos.h"
#include <string.h>

// Include the header file
#include "midi_common.h"

// Declare external variables for testing
extern QueueHandle_t xUartToUsbQueue;
extern QueueHandle_t xUsbToUartQueue;
extern MIDIStats_t midi_stats;

void setUp(void)
{
    // Reset global state
    memset(&midi_stats, 0, sizeof(midi_stats));
}

void tearDown(void)
{
}

// Test MIDI_GetCIN function
void test_MIDI_GetCIN_NoteOff(void)
{
    TEST_ASSERT_EQUAL_UINT8(USB_MIDI_CIN_NOTE_OFF, MIDI_GetCIN(0x80, 3));
    TEST_ASSERT_EQUAL_UINT8(USB_MIDI_CIN_NOTE_OFF, MIDI_GetCIN(0x8F, 3));
}

void test_MIDI_GetCIN_NoteOn(void)
{
    TEST_ASSERT_EQUAL_UINT8(USB_MIDI_CIN_NOTE_ON, MIDI_GetCIN(0x90, 3));
    TEST_ASSERT_EQUAL_UINT8(USB_MIDI_CIN_NOTE_ON, MIDI_GetCIN(0x9F, 3));
}

void test_MIDI_GetCIN_PolyKeyPressure(void)
{
    TEST_ASSERT_EQUAL_UINT8(USB_MIDI_CIN_POLY_KEYPRESS, MIDI_GetCIN(0xA0, 3));
    TEST_ASSERT_EQUAL_UINT8(USB_MIDI_CIN_POLY_KEYPRESS, MIDI_GetCIN(0xAF, 3));
}

void test_MIDI_GetCIN_ControlChange(void)
{
    TEST_ASSERT_EQUAL_UINT8(USB_MIDI_CIN_CTRL_CHANGE, MIDI_GetCIN(0xB0, 3));
    TEST_ASSERT_EQUAL_UINT8(USB_MIDI_CIN_CTRL_CHANGE, MIDI_GetCIN(0xBF, 3));
}

void test_MIDI_GetCIN_ProgramChange(void)
{
    TEST_ASSERT_EQUAL_UINT8(USB_MIDI_CIN_PROG_CHANGE, MIDI_GetCIN(0xC0, 2));
    TEST_ASSERT_EQUAL_UINT8(USB_MIDI_CIN_PROG_CHANGE, MIDI_GetCIN(0xCF, 2));
}

void test_MIDI_GetCIN_ChannelPressure(void)
{
    TEST_ASSERT_EQUAL_UINT8(USB_MIDI_CIN_CHAN_PRESSURE, MIDI_GetCIN(0xD0, 2));
    TEST_ASSERT_EQUAL_UINT8(USB_MIDI_CIN_CHAN_PRESSURE, MIDI_GetCIN(0xDF, 2));
}

void test_MIDI_GetCIN_PitchBend(void)
{
    TEST_ASSERT_EQUAL_UINT8(USB_MIDI_CIN_PITCH_BEND, MIDI_GetCIN(0xE0, 3));
    TEST_ASSERT_EQUAL_UINT8(USB_MIDI_CIN_PITCH_BEND, MIDI_GetCIN(0xEF, 3));
}

void test_MIDI_GetCIN_SystemRealTime(void)
{
    TEST_ASSERT_EQUAL_UINT8(USB_MIDI_CIN_1BYTE, MIDI_GetCIN(0xF8, 1)); // Timing Clock
    TEST_ASSERT_EQUAL_UINT8(USB_MIDI_CIN_1BYTE, MIDI_GetCIN(0xFA, 1)); // Start
    TEST_ASSERT_EQUAL_UINT8(USB_MIDI_CIN_1BYTE, MIDI_GetCIN(0xFB, 1)); // Continue
    TEST_ASSERT_EQUAL_UINT8(USB_MIDI_CIN_1BYTE, MIDI_GetCIN(0xFC, 1)); // Stop
    TEST_ASSERT_EQUAL_UINT8(USB_MIDI_CIN_1BYTE, MIDI_GetCIN(0xFE, 1)); // Active Sensing
    TEST_ASSERT_EQUAL_UINT8(USB_MIDI_CIN_1BYTE, MIDI_GetCIN(0xFF, 1)); // System Reset
}

void test_MIDI_GetCIN_SystemCommon(void)
{
    TEST_ASSERT_EQUAL_UINT8(USB_MIDI_CIN_3BYTE_SYSCOM, MIDI_GetCIN(0xF2, 3)); // Song Position
    TEST_ASSERT_EQUAL_UINT8(USB_MIDI_CIN_2BYTE_SYSCOM, MIDI_GetCIN(0xF3, 2)); // Song Select
    TEST_ASSERT_EQUAL_UINT8(USB_MIDI_CIN_1BYTE, MIDI_GetCIN(0xF6, 1)); // Tune Request
}

void test_MIDI_GetCIN_InvalidStatus(void)
{
    // Non-status bytes with length=1 are treated as single-byte messages
    TEST_ASSERT_EQUAL_UINT8(USB_MIDI_CIN_1BYTE, MIDI_GetCIN(0x00, 1)); // Non-status byte
    TEST_ASSERT_EQUAL_UINT8(USB_MIDI_CIN_1BYTE, MIDI_GetCIN(0x7F, 1)); // Non-status byte
}

// Test MIDI_GetLengthFromCIN function
void test_MIDI_GetLengthFromCIN_2Byte(void)
{
    TEST_ASSERT_EQUAL_UINT8(2, MIDI_GetLengthFromCIN(USB_MIDI_CIN_2BYTE_SYSCOM));
    TEST_ASSERT_EQUAL_UINT8(2, MIDI_GetLengthFromCIN(USB_MIDI_CIN_PROG_CHANGE));
    TEST_ASSERT_EQUAL_UINT8(2, MIDI_GetLengthFromCIN(USB_MIDI_CIN_CHAN_PRESSURE));
}

void test_MIDI_GetLengthFromCIN_3Byte(void)
{
    TEST_ASSERT_EQUAL_UINT8(3, MIDI_GetLengthFromCIN(USB_MIDI_CIN_3BYTE_SYSCOM));
    TEST_ASSERT_EQUAL_UINT8(3, MIDI_GetLengthFromCIN(USB_MIDI_CIN_NOTE_OFF));
    TEST_ASSERT_EQUAL_UINT8(3, MIDI_GetLengthFromCIN(USB_MIDI_CIN_NOTE_ON));
    TEST_ASSERT_EQUAL_UINT8(3, MIDI_GetLengthFromCIN(USB_MIDI_CIN_POLY_KEYPRESS));
    TEST_ASSERT_EQUAL_UINT8(3, MIDI_GetLengthFromCIN(USB_MIDI_CIN_CTRL_CHANGE));
    TEST_ASSERT_EQUAL_UINT8(3, MIDI_GetLengthFromCIN(USB_MIDI_CIN_PITCH_BEND));
}

void test_MIDI_GetLengthFromCIN_1Byte(void)
{
    TEST_ASSERT_EQUAL_UINT8(1, MIDI_GetLengthFromCIN(USB_MIDI_CIN_1BYTE));
}

void test_MIDI_GetLengthFromCIN_Unknown(void)
{
    // Unknown CINs default to 3 bytes for safety
    TEST_ASSERT_EQUAL_UINT8(3, MIDI_GetLengthFromCIN(USB_MIDI_CIN_MISC));
    TEST_ASSERT_EQUAL_UINT8(3, MIDI_GetLengthFromCIN(USB_MIDI_CIN_CABLE_EVENT));
    TEST_ASSERT_EQUAL_UINT8(3, MIDI_GetLengthFromCIN(0xFF)); // Invalid CIN
}

// Test MIDI_GetExpectedLength function
void test_MIDI_GetExpectedLength_ChannelVoice(void)
{
    // Note Off
    TEST_ASSERT_EQUAL_UINT8(3, MIDI_GetExpectedLength(0x80));
    TEST_ASSERT_EQUAL_UINT8(3, MIDI_GetExpectedLength(0x8F));
    
    // Note On
    TEST_ASSERT_EQUAL_UINT8(3, MIDI_GetExpectedLength(0x90));
    TEST_ASSERT_EQUAL_UINT8(3, MIDI_GetExpectedLength(0x9F));
    
    // Poly Key Pressure
    TEST_ASSERT_EQUAL_UINT8(3, MIDI_GetExpectedLength(0xA0));
    TEST_ASSERT_EQUAL_UINT8(3, MIDI_GetExpectedLength(0xAF));
    
    // Control Change
    TEST_ASSERT_EQUAL_UINT8(3, MIDI_GetExpectedLength(0xB0));
    TEST_ASSERT_EQUAL_UINT8(3, MIDI_GetExpectedLength(0xBF));
    
    // Program Change
    TEST_ASSERT_EQUAL_UINT8(2, MIDI_GetExpectedLength(0xC0));
    TEST_ASSERT_EQUAL_UINT8(2, MIDI_GetExpectedLength(0xCF));
    
    // Channel Pressure
    TEST_ASSERT_EQUAL_UINT8(2, MIDI_GetExpectedLength(0xD0));
    TEST_ASSERT_EQUAL_UINT8(2, MIDI_GetExpectedLength(0xDF));
    
    // Pitch Bend
    TEST_ASSERT_EQUAL_UINT8(3, MIDI_GetExpectedLength(0xE0));
    TEST_ASSERT_EQUAL_UINT8(3, MIDI_GetExpectedLength(0xEF));
}

void test_MIDI_GetExpectedLength_SystemCommon(void)
{
    TEST_ASSERT_EQUAL_UINT8(1, MIDI_GetExpectedLength(0xF0)); // SysEx Start (returns 1 for the start byte)
    TEST_ASSERT_EQUAL_UINT8(2, MIDI_GetExpectedLength(0xF1)); // MTC Quarter Frame
    TEST_ASSERT_EQUAL_UINT8(3, MIDI_GetExpectedLength(0xF2)); // Song Position
    TEST_ASSERT_EQUAL_UINT8(2, MIDI_GetExpectedLength(0xF3)); // Song Select
    TEST_ASSERT_EQUAL_UINT8(1, MIDI_GetExpectedLength(0xF6)); // Tune Request
    TEST_ASSERT_EQUAL_UINT8(1, MIDI_GetExpectedLength(0xF7)); // SysEx End
}

void test_MIDI_GetExpectedLength_SystemRealTime(void)
{
    TEST_ASSERT_EQUAL_UINT8(1, MIDI_GetExpectedLength(0xF8)); // Timing Clock
    TEST_ASSERT_EQUAL_UINT8(1, MIDI_GetExpectedLength(0xFA)); // Start
    TEST_ASSERT_EQUAL_UINT8(1, MIDI_GetExpectedLength(0xFB)); // Continue
    TEST_ASSERT_EQUAL_UINT8(1, MIDI_GetExpectedLength(0xFC)); // Stop
    TEST_ASSERT_EQUAL_UINT8(1, MIDI_GetExpectedLength(0xFE)); // Active Sensing
    TEST_ASSERT_EQUAL_UINT8(1, MIDI_GetExpectedLength(0xFF)); // System Reset
}

void test_MIDI_GetExpectedLength_Invalid(void)
{
    // Non-status bytes default to 3 bytes for safety
    TEST_ASSERT_EQUAL_UINT8(3, MIDI_GetExpectedLength(0x00)); // Not a status byte
    TEST_ASSERT_EQUAL_UINT8(3, MIDI_GetExpectedLength(0x7F)); // Not a status byte
}



int main(void)
{
    UNITY_BEGIN();
    
    // MIDI_GetCIN tests
    RUN_TEST(test_MIDI_GetCIN_NoteOff);
    RUN_TEST(test_MIDI_GetCIN_NoteOn);
    RUN_TEST(test_MIDI_GetCIN_PolyKeyPressure);
    RUN_TEST(test_MIDI_GetCIN_ControlChange);
    RUN_TEST(test_MIDI_GetCIN_ProgramChange);
    RUN_TEST(test_MIDI_GetCIN_ChannelPressure);
    RUN_TEST(test_MIDI_GetCIN_PitchBend);
    RUN_TEST(test_MIDI_GetCIN_SystemRealTime);
    RUN_TEST(test_MIDI_GetCIN_SystemCommon);
    RUN_TEST(test_MIDI_GetCIN_InvalidStatus);
    
    // MIDI_GetLengthFromCIN tests
    RUN_TEST(test_MIDI_GetLengthFromCIN_2Byte);
    RUN_TEST(test_MIDI_GetLengthFromCIN_3Byte);
    RUN_TEST(test_MIDI_GetLengthFromCIN_1Byte);
    RUN_TEST(test_MIDI_GetLengthFromCIN_Unknown);
    
    // MIDI_GetExpectedLength tests
    RUN_TEST(test_MIDI_GetExpectedLength_ChannelVoice);
    RUN_TEST(test_MIDI_GetExpectedLength_SystemCommon);
    RUN_TEST(test_MIDI_GetExpectedLength_SystemRealTime);
    RUN_TEST(test_MIDI_GetExpectedLength_Invalid);
    
    return UNITY_END();
}
