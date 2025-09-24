#include "test_common.h"
#include "usb_descriptors.h"
#include "mode_manager.h"
#include <string.h>

// Mock global variable from mode_manager
static MidiMode_t mock_midi_mode = MIDI_MODE_2_0;

// Mock implementation of ModeManager_GetMode
MidiMode_t ModeManager_GetMode(void)
{
    return mock_midi_mode;
}

// Test setup/teardown
void setUp(void)
{
    // Reset to default mode
    mock_midi_mode = MIDI_MODE_2_0;
}

void tearDown(void)
{
}

// Test USB_GetManufacturerString
void test_USB_GetManufacturerString(void)
{
    const char* manufacturer = USB_GetManufacturerString();
    TEST_ASSERT_NOT_NULL(manufacturer);
    TEST_ASSERT_EQUAL_STRING("MIDI2USB", manufacturer);
}

// Test USB_GetProductString for MIDI 1.0 mode
void test_USB_GetProductString_MIDI10(void)
{
    mock_midi_mode = MIDI_MODE_1_0;
    const char* product = USB_GetProductString();
    TEST_ASSERT_NOT_NULL(product);
    TEST_ASSERT_EQUAL_STRING("MIDI2USB Converter (MIDI 1.0)", product);
}

// Test USB_GetProductString for MIDI 2.0 mode
void test_USB_GetProductString_MIDI20(void)
{
    mock_midi_mode = MIDI_MODE_2_0;
    const char* product = USB_GetProductString();
    TEST_ASSERT_NOT_NULL(product);
    TEST_ASSERT_EQUAL_STRING("MIDI2USB Converter (MIDI 2.0)", product);
}

// Test USB_GetSerialString
void test_USB_GetSerialString(void)
{
    const char* serial = USB_GetSerialString();
    TEST_ASSERT_NOT_NULL(serial);
    TEST_ASSERT_EQUAL_STRING("001", serial);
}

// Test USB_ConvertASCIItoUTF16
void test_USB_ConvertASCIItoUTF16_Basic(void)
{
    const char* test_str = "Hello";
    uint16_t utf16_buf[10] = {0};
    
    size_t count = USB_ConvertASCIItoUTF16(test_str, utf16_buf, 10);
    
    TEST_ASSERT_EQUAL(5, count);
    TEST_ASSERT_EQUAL('H', utf16_buf[0]);
    TEST_ASSERT_EQUAL('e', utf16_buf[1]);
    TEST_ASSERT_EQUAL('l', utf16_buf[2]);
    TEST_ASSERT_EQUAL('l', utf16_buf[3]);
    TEST_ASSERT_EQUAL('o', utf16_buf[4]);
}

// Test USB_ConvertASCIItoUTF16 with truncation
void test_USB_ConvertASCIItoUTF16_Truncated(void)
{
    const char* test_str = "Hello World";
    uint16_t utf16_buf[5] = {0};
    
    size_t count = USB_ConvertASCIItoUTF16(test_str, utf16_buf, 5);
    
    TEST_ASSERT_EQUAL(5, count);
    TEST_ASSERT_EQUAL('H', utf16_buf[0]);
    TEST_ASSERT_EQUAL('e', utf16_buf[1]);
    TEST_ASSERT_EQUAL('l', utf16_buf[2]);
    TEST_ASSERT_EQUAL('l', utf16_buf[3]);
    TEST_ASSERT_EQUAL('o', utf16_buf[4]);
}

// Test USB_ConvertASCIItoUTF16 with NULL input
void test_USB_ConvertASCIItoUTF16_NullInput(void)
{
    uint16_t utf16_buf[10] = {0};
    
    size_t count = USB_ConvertASCIItoUTF16(NULL, utf16_buf, 10);
    TEST_ASSERT_EQUAL(0, count);
    
    count = USB_ConvertASCIItoUTF16("Test", NULL, 10);
    TEST_ASSERT_EQUAL(0, count);
    
    count = USB_ConvertASCIItoUTF16("Test", utf16_buf, 0);
    TEST_ASSERT_EQUAL(0, count);
}

// Test USB_ConvertASCIItoUTF16 with empty string
void test_USB_ConvertASCIItoUTF16_EmptyString(void)
{
    const char* test_str = "";
    uint16_t utf16_buf[10] = {0};
    
    size_t count = USB_ConvertASCIItoUTF16(test_str, utf16_buf, 10);
    
    TEST_ASSERT_EQUAL(0, count);
}

// Test USB_GetVendorID
void test_USB_GetVendorID(void)
{
    uint16_t vid = USB_GetVendorID();
    TEST_ASSERT_EQUAL_HEX16(0x1d50, vid);
}

// Test USB_GetProductID for MIDI 1.0
void test_USB_GetProductID_MIDI10(void)
{
    mock_midi_mode = MIDI_MODE_1_0;
    uint16_t pid = USB_GetProductID();
    TEST_ASSERT_EQUAL_HEX16(0x6194, pid);
}

// Test USB_GetProductID for MIDI 2.0
void test_USB_GetProductID_MIDI20(void)
{
    mock_midi_mode = MIDI_MODE_2_0;
    uint16_t pid = USB_GetProductID();
    TEST_ASSERT_EQUAL_HEX16(0x6195, pid);
}

int main(void)
{
    UNITY_BEGIN();
    
    RUN_TEST(test_USB_GetManufacturerString);
    RUN_TEST(test_USB_GetProductString_MIDI10);
    RUN_TEST(test_USB_GetProductString_MIDI20);
    RUN_TEST(test_USB_GetSerialString);
    RUN_TEST(test_USB_ConvertASCIItoUTF16_Basic);
    RUN_TEST(test_USB_ConvertASCIItoUTF16_Truncated);
    RUN_TEST(test_USB_ConvertASCIItoUTF16_NullInput);
    RUN_TEST(test_USB_ConvertASCIItoUTF16_EmptyString);
    RUN_TEST(test_USB_GetVendorID);
    RUN_TEST(test_USB_GetProductID_MIDI10);
    RUN_TEST(test_USB_GetProductID_MIDI20);
    
    return UNITY_END();
}