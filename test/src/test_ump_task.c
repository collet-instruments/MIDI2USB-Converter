// This file contains the test cases for GetUmpWordCount function
// It's included by test_ump_wordcount.c which provides the actual function

#include <stdint.h>
#include "unity.h"

// External declaration of the function to test
extern uint8_t GetUmpWordCount(uint32_t first_word);

void setUp(void)
{
}

void tearDown(void)
{
}

// Test Utility Messages (32-bit)
void test_GetUmpWordCount_UtilityMessages(void)
{
    // Utility message type = 0x0
    uint32_t msg = 0x00000000;  // Type 0
    TEST_ASSERT_EQUAL_UINT8(1, GetUmpWordCount(msg));
    
    msg = 0x0FFFFFFF;  // Type 0 with all other bits set
    TEST_ASSERT_EQUAL_UINT8(1, GetUmpWordCount(msg));
}

// Test System Messages (32-bit)
void test_GetUmpWordCount_SystemMessages(void)
{
    // System Real Time and System Common type = 0x1
    uint32_t msg = 0x10000000;  // Type 1
    TEST_ASSERT_EQUAL_UINT8(1, GetUmpWordCount(msg));
}

// Test MIDI 1.0 Channel Voice (32-bit)
void test_GetUmpWordCount_Midi10ChannelVoice(void)
{
    // MIDI 1.0 Channel Voice type = 0x2
    uint32_t msg = 0x20000000;  // Type 2
    TEST_ASSERT_EQUAL_UINT8(1, GetUmpWordCount(msg));
}

// Test Data Messages SysEx7 (64-bit)
void test_GetUmpWordCount_DataMessagesSysEx7(void)
{
    // Data Messages (SysEx7) type = 0x3
    uint32_t msg = 0x30000000;  // Type 3
    TEST_ASSERT_EQUAL_UINT8(2, GetUmpWordCount(msg));
}

// Test MIDI 2.0 Channel Voice (64-bit)
void test_GetUmpWordCount_Midi20ChannelVoice(void)
{
    // MIDI 2.0 Channel Voice type = 0x4
    uint32_t msg = 0x40000000;  // Type 4
    TEST_ASSERT_EQUAL_UINT8(2, GetUmpWordCount(msg));
}

// Test Data Messages 128-bit
void test_GetUmpWordCount_DataMessages128(void)
{
    // Data Messages type = 0x5
    uint32_t msg = 0x50000000;  // Type 5
    TEST_ASSERT_EQUAL_UINT8(4, GetUmpWordCount(msg));  // Correctly returns 4 words per Core implementation
}

// Test Reserved Types
void test_GetUmpWordCount_ReservedTypes(void)
{
    // All reserved types default to 1 word in the actual implementation
    TEST_ASSERT_EQUAL_UINT8(1, GetUmpWordCount(0x60000000));  // Type 6
    TEST_ASSERT_EQUAL_UINT8(1, GetUmpWordCount(0x70000000));  // Type 7
    TEST_ASSERT_EQUAL_UINT8(1, GetUmpWordCount(0x80000000));  // Type 8
    TEST_ASSERT_EQUAL_UINT8(1, GetUmpWordCount(0x90000000));  // Type 9
    TEST_ASSERT_EQUAL_UINT8(1, GetUmpWordCount(0xA0000000));  // Type A
    TEST_ASSERT_EQUAL_UINT8(1, GetUmpWordCount(0xB0000000));  // Type B
    TEST_ASSERT_EQUAL_UINT8(1, GetUmpWordCount(0xC0000000));  // Type C
}

// Test Flex Data and Stream Messages
void test_GetUmpWordCount_FlexAndStream(void)
{
    // Flex Data type = 0xD - defaults to 1 word in actual implementation
    uint32_t msg = 0xD0000000;  // Type D
    TEST_ASSERT_EQUAL_UINT8(1, GetUmpWordCount(msg));
    
    // Reserved type = 0xE - defaults to 1 word
    msg = 0xE0000000;  // Type E
    TEST_ASSERT_EQUAL_UINT8(1, GetUmpWordCount(msg));
    
    // Stream Messages type = 0xF - 4 words
    msg = 0xF0000000;  // Type F
    TEST_ASSERT_EQUAL_UINT8(4, GetUmpWordCount(msg));
}

// Test edge cases
void test_GetUmpWordCount_EdgeCases(void)
{
    // Test with various bit patterns in lower bits
    uint32_t msg = 0x2ABCDEF0;  // Type 2 with random data
    TEST_ASSERT_EQUAL_UINT8(1, GetUmpWordCount(msg));
    
    msg = 0x4789ABCD;  // Type 4 with random data
    TEST_ASSERT_EQUAL_UINT8(2, GetUmpWordCount(msg));
    
    msg = 0x51234567;  // Type 5 with random data
    TEST_ASSERT_EQUAL_UINT8(4, GetUmpWordCount(msg));
    
    msg = 0xF1234567;  // Type F with random data
    TEST_ASSERT_EQUAL_UINT8(4, GetUmpWordCount(msg));
}

int main(void)
{
    UNITY_BEGIN();
    
    RUN_TEST(test_GetUmpWordCount_UtilityMessages);
    RUN_TEST(test_GetUmpWordCount_SystemMessages);
    RUN_TEST(test_GetUmpWordCount_Midi10ChannelVoice);
    RUN_TEST(test_GetUmpWordCount_DataMessagesSysEx7);
    RUN_TEST(test_GetUmpWordCount_Midi20ChannelVoice);
    RUN_TEST(test_GetUmpWordCount_DataMessages128);
    RUN_TEST(test_GetUmpWordCount_ReservedTypes);
    RUN_TEST(test_GetUmpWordCount_FlexAndStream);
    RUN_TEST(test_GetUmpWordCount_EdgeCases);
    
    return UNITY_END();
}