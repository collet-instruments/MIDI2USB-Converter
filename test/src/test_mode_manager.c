#include "test_common.h"
#include "mode_manager.h"
#include "mock_hal.h"
#include "mock_hal.c"  // Include implementation for test build

// Include the source file under test
#include "../../Core/Src/mode_manager.c"

void setUp(void)
{
    MockHAL_Init();
    // Reset global state
    g_midi_mode = MIDI_MODE_1_0;
    current_hal = NULL;
}

void tearDown(void)
{
    MockHAL_Reset();
}

// Test ModeManager_DetermineModeFromPin pure function
void test_ModeManager_DetermineModeFromPin_ReturnsMidi10_WhenPinReset(void)
{
    MidiMode_t mode = ModeManager_DetermineModeFromPin(HAL_PIN_RESET);
    TEST_ASSERT_EQUAL(MIDI_MODE_1_0, mode);
}

void test_ModeManager_DetermineModeFromPin_ReturnsMidi20_WhenPinSet(void)
{
    MidiMode_t mode = ModeManager_DetermineModeFromPin(HAL_PIN_SET);
    TEST_ASSERT_EQUAL(MIDI_MODE_2_0, mode);
}

// Test ModeManager_InitWithHAL
void test_ModeManager_InitWithHAL_SetsMidi10Mode_WhenPinReset(void)
{
    MockHAL_SetSetupPinState(HAL_PIN_RESET);
    const ModeManagerHAL_t* hal = MockHAL_GetInterface();
    
    ModeManager_InitWithHAL(hal);
    
    TEST_ASSERT_EQUAL(MIDI_MODE_1_0, g_midi_mode);
    TEST_ASSERT_EQUAL(1, MockHAL_GetState()->read_setup_pin_call_count);
}

void test_ModeManager_InitWithHAL_SetsMidi20Mode_WhenPinSet(void)
{
    MockHAL_SetSetupPinState(HAL_PIN_SET);
    const ModeManagerHAL_t* hal = MockHAL_GetInterface();
    
    ModeManager_InitWithHAL(hal);
    
    TEST_ASSERT_EQUAL(MIDI_MODE_2_0, g_midi_mode);
    TEST_ASSERT_EQUAL(1, MockHAL_GetState()->read_setup_pin_call_count);
}

void test_ModeManager_InitWithHAL_HandlesNullHAL(void)
{
    g_midi_mode = MIDI_MODE_2_0; // Set to non-default
    
    ModeManager_InitWithHAL(NULL);
    
    TEST_ASSERT_EQUAL(MIDI_MODE_2_0, g_midi_mode); // Should remain unchanged
}

// Test ModeManager_SetLEDsWithHAL
void test_ModeManager_SetLEDsWithHAL_SetsMidi10LEDs(void)
{
    const ModeManagerHAL_t* hal = MockHAL_GetInterface();
    
    ModeManager_SetLEDsWithHAL(MIDI_MODE_1_0, hal);
    
    TEST_ASSERT_EQUAL(HAL_PIN_SET, MockHAL_GetState()->m1_led_state);
    TEST_ASSERT_EQUAL(HAL_PIN_RESET, MockHAL_GetState()->m2_led_state);
    TEST_ASSERT_EQUAL(1, MockHAL_GetState()->set_m1_led_call_count);
    TEST_ASSERT_EQUAL(1, MockHAL_GetState()->set_m2_led_call_count);
}

void test_ModeManager_SetLEDsWithHAL_SetsMidi20LEDs(void)
{
    const ModeManagerHAL_t* hal = MockHAL_GetInterface();
    
    ModeManager_SetLEDsWithHAL(MIDI_MODE_2_0, hal);
    
    TEST_ASSERT_EQUAL(HAL_PIN_RESET, MockHAL_GetState()->m1_led_state);
    TEST_ASSERT_EQUAL(HAL_PIN_SET, MockHAL_GetState()->m2_led_state);
    TEST_ASSERT_EQUAL(1, MockHAL_GetState()->set_m1_led_call_count);
    TEST_ASSERT_EQUAL(1, MockHAL_GetState()->set_m2_led_call_count);
}

void test_ModeManager_SetLEDsWithHAL_HandlesNullHAL(void)
{
    ModeManager_SetLEDsWithHAL(MIDI_MODE_1_0, NULL);
    
    // Should not crash and no calls should be made
    TEST_ASSERT_EQUAL(0, MockHAL_GetState()->set_m1_led_call_count);
    TEST_ASSERT_EQUAL(0, MockHAL_GetState()->set_m2_led_call_count);
}

// Test ModeManager_GetMode
void test_ModeManager_GetMode_ReturnsCurrentMode(void)
{
    g_midi_mode = MIDI_MODE_2_0;
    TEST_ASSERT_EQUAL(MIDI_MODE_2_0, ModeManager_GetMode());
    
    g_midi_mode = MIDI_MODE_1_0;
    TEST_ASSERT_EQUAL(MIDI_MODE_1_0, ModeManager_GetMode());
}

// Test is_midi2_mode
void test_is_midi2_mode_ReturnsTrueForMidi20(void)
{
    g_midi_mode = MIDI_MODE_2_0;
    TEST_ASSERT_TRUE(is_midi2_mode());
}

void test_is_midi2_mode_ReturnsFalseForMidi10(void)
{
    g_midi_mode = MIDI_MODE_1_0;
    TEST_ASSERT_FALSE(is_midi2_mode());
}

// Test ModeManager_SetLEDs with current_hal
void test_ModeManager_SetLEDs_UsesCurrentHAL(void)
{
    const ModeManagerHAL_t* hal = MockHAL_GetInterface();
    current_hal = hal;
    
    ModeManager_SetLEDs(MIDI_MODE_2_0);
    
    TEST_ASSERT_EQUAL(HAL_PIN_RESET, MockHAL_GetState()->m1_led_state);
    TEST_ASSERT_EQUAL(HAL_PIN_SET, MockHAL_GetState()->m2_led_state);
}

int main(void)
{
    UNITY_BEGIN();
    
    // Pure function tests
    RUN_TEST(test_ModeManager_DetermineModeFromPin_ReturnsMidi10_WhenPinReset);
    RUN_TEST(test_ModeManager_DetermineModeFromPin_ReturnsMidi20_WhenPinSet);
    
    // Dependency injection tests
    RUN_TEST(test_ModeManager_InitWithHAL_SetsMidi10Mode_WhenPinReset);
    RUN_TEST(test_ModeManager_InitWithHAL_SetsMidi20Mode_WhenPinSet);
    RUN_TEST(test_ModeManager_InitWithHAL_HandlesNullHAL);
    
    // LED control tests
    RUN_TEST(test_ModeManager_SetLEDsWithHAL_SetsMidi10LEDs);
    RUN_TEST(test_ModeManager_SetLEDsWithHAL_SetsMidi20LEDs);
    RUN_TEST(test_ModeManager_SetLEDsWithHAL_HandlesNullHAL);
    
    // Getter tests
    RUN_TEST(test_ModeManager_GetMode_ReturnsCurrentMode);
    RUN_TEST(test_is_midi2_mode_ReturnsTrueForMidi20);
    RUN_TEST(test_is_midi2_mode_ReturnsFalseForMidi10);
    
    // Integration tests
    RUN_TEST(test_ModeManager_SetLEDs_UsesCurrentHAL);
    
    return UNITY_END();
}