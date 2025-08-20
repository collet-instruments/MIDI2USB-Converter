#include "test_common.h"
#include <stdint.h>

// Extract only the GetUmpWordCount function from Core/Src/ump_task.c for testing
// This is lines 116-137 of the actual implementation
#ifdef TESTING
uint8_t GetUmpWordCount(uint32_t first_word) {
#else
static uint8_t GetUmpWordCount(uint32_t first_word) {
#endif
  uint8_t message_type = (first_word >> 28) & 0xF;
  
  switch (message_type) {
    case 0x0:  // Utility messages (32-bit)
      return 1;
    case 0x1:  // System messages (32-bit) - includes F8 Clock
      return 1;
    case 0x2:  // MIDI 1.0 Channel Voice messages (32-bit)
      return 1;
    case 0x3:  // Data messages (64-bit)
      return 2;
    case 0x4:  // MIDI 2.0 Channel Voice messages (64-bit)
      return 2;
    case 0x5:  // Data messages (128-bit)
      return 4;
    case 0xF:  // Stream messages (128-bit) - UMP Discovery, Protocol negotiation etc
      return 4;
    default:
      // Default to 1 word for unknown message types
      return 1;
  }
}

// Now include the test file
#include "test_ump_task.c"