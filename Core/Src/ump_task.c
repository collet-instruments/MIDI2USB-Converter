/**
  * @file           : ump_task.c
  * @brief          : UMP USB communication tasks for MIDI 2.0
  */

/* Includes ------------------------------------------------------------------*/
#include "ump_task.h"
#include "app_ump_device.h"
#include "midi_common.h"
#include "midi2_task.h"
#include "ump_discovery.h"

/* Private function prototypes -----------------------------------------------*/
#ifdef TESTING
// For testing, make the function non-static
uint8_t GetUmpWordCount(uint32_t first_word);
#else
static uint8_t GetUmpWordCount(uint32_t first_word);
#endif

/* External variables --------------------------------------------------------*/
extern QueueHandle_t xUmpTxQueue;
extern QueueHandle_t xUmpRxQueue;

/* Public functions ----------------------------------------------------------*/

/**
  * @brief UMP to USB Task - sends UMP packets to USB
  * @param pvParameters: Task parameters
  * @retval None
  */
void vUmpToUsbTask(void *pvParameters) {
  (void) pvParameters;
  uint32_t ump_data[4];  // UMP message buffer (up to 16 bytes)
  
  while (1) {
    // Wait for UMP packet from conversion task
    if (xQueueReceive(xUmpTxQueue, ump_data, portMAX_DELAY) == pdTRUE) {
      // Send UMP packet to USB
      if (tud_ump_n_mounted(0)) {
        // Try to write UMP packet with correct word count
        uint8_t word_count = GetUmpWordCount(ump_data[0]);
        
        uint16_t written = tud_ump_write(0, ump_data, word_count);
        if (written > 0) {
          midi_stats.usb_tx_count++;
        } else {
          midi_stats.usb_errors++;
          // Add delay when buffer is full
          vTaskDelay(pdMS_TO_TICKS(1));
        }
      } else {
        midi_stats.usb_errors++;
      }
    }
  }
}

/**
  * @brief USB to UMP Task - receives UMP packets from USB
  * @param pvParameters: Task parameters
  * @retval None
  */
void vUsbToUmpTask(void *pvParameters) {
  (void) pvParameters;
  uint32_t ump_data[4];  // UMP message buffer
  
  while (1) {
    // Check for incoming UMP data
    if (tud_ump_n_mounted(0) && tud_ump_n_available(0) > 0) {
      // Read UMP packet from USB
      uint16_t words_read = tud_ump_read(0, ump_data, 4);
      if (words_read > 0) {
        midi_stats.usb_rx_count++;
        
        // Check message type
        uint8_t message_type = (ump_data[0] >> 28) & 0xF;
        uint8_t word_count = GetUmpWordCount(ump_data[0]);
        
        if (message_type == 0xF) {
          // Process Stream messages for Discovery
          UMP_ProcessStreamMessage(ump_data, word_count);
        } else if (message_type == 0x3) {
          // Process Data messages (SysEx) for MIDI-CI
          UMP_ProcessDataMessage(ump_data, word_count);
        } else {
          // Send UMP packet to conversion task for normal MIDI messages
          if (xQueueSend(xUmpRxQueue, ump_data, 0) != pdTRUE) {
            midi_stats.queue_full_errors++;
          }
        }
      }
    }
    
    // Small delay to prevent overwhelming the CPU
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

/* Private functions ---------------------------------------------------------*/

/**
  * @brief Determine the number of words in a UMP message based on message type
  * @param first_word: First word of the UMP message
  * @retval Number of words (1-4)
  */
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
