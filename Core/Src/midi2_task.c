/**
  * @file           : midi2_task.c
  * @brief          : MIDI 2.0 conversion tasks using AM MIDI 2.0 Library
  */

/* Includes ------------------------------------------------------------------*/
#include "midi2_task.h"
#include "midi_common.h"
#include "mode_manager.h"
#include "midi2_wrapper.h"
#include "main.h"  // For LED pin definitions
#include "ump_discovery.h"  // For Discovery Reply tracking
#include "uart_midi_task.h"  // For UART_TX_SendDMA
#include <string.h>

/* Private includes ----------------------------------------------------------*/
#include "tusb.h"
// Note: Using AM MIDI 2.0 Library for conversion

/* Private defines -----------------------------------------------------------*/
#define UMP_QUEUE_LENGTH        16

/* Private variables ---------------------------------------------------------*/
// Global converter instances for 2-stage conversion
static midi2_converter_handle_t g_bs_to_ump_converter = NULL;      // MIDI1.0 → UMP
static midi2_converter_handle_t g_ump_to_midi2_converter = NULL;   // UMP → MIDI2.0
static midi2_converter_handle_t g_ump_to_midi1_converter = NULL;   // UMP → MIDI1.0

/* Exported variables --------------------------------------------------------*/
QueueHandle_t xUmpTxQueue;
QueueHandle_t xUmpRxQueue;

/* Private function prototypes -----------------------------------------------*/
static BaseType_t InitMIDI2Converters(void);

/* Public functions ----------------------------------------------------------*/

/**
  * @brief  Initialize MIDI 2.0 queues and converters
  * @retval pdPASS if successful, pdFAIL otherwise
  */
BaseType_t MIDI2_InitQueues(void)
{
  xUmpTxQueue = xQueueCreate(UMP_QUEUE_LENGTH, sizeof(uint32_t) * 4);  // 4 words per UMP packet
  if (xUmpTxQueue == NULL) {
    return pdFAIL;
  }
  
  xUmpRxQueue = xQueueCreate(UMP_QUEUE_LENGTH, sizeof(uint32_t) * 4);  // 4 words per UMP packet
  if (xUmpRxQueue == NULL) {
    vQueueDelete(xUmpTxQueue);
    return pdFAIL;
  }
  
  // Initialize converters
  if (InitMIDI2Converters() != pdPASS) {
    vQueueDelete(xUmpTxQueue);
    vQueueDelete(xUmpRxQueue);
    return pdFAIL;
  }
  
  return pdPASS;
}

/**
  * @brief  MIDI 2.0 Task: Convert UART MIDI 1.0 to UMP using AM MIDI 2.0 Library
  * @param  pvParameters: Task parameters
  * @retval None
  */
void vMidi2UartToUmpTask(void *pvParameters)
{
  /* Prevent unused parameter warning */
  (void)pvParameters;
  
  MIDIPacket_t midi_packet;
  uint32_t ump_data[4] = {0};  // UMP message buffer (up to 16 bytes)
  
  for(;;)
  {
    // Wait for MIDI 1.0 packet from UART
    if (xQueueReceive(xUartToUsbQueue, &midi_packet, portMAX_DELAY) == pdTRUE) {
      
      // Handle different MIDI message types based on reference implementation
      if (midi_packet.length == 1) {
        // Single-byte message (System Realtime like F8 Clock)
        uint8_t midi_byte = midi_packet.data[0];
        
        // Check if it's a system realtime message (F8-FF)
        if (midi_byte >= 0xF8) {
          // Process system realtime message directly
          midi2_bs_to_ump_process_byte(g_bs_to_ump_converter, midi_byte);
          
          // Process available UMP messages
          while (midi2_bs_to_ump_available(g_bs_to_ump_converter)) {
            uint32_t ump_midi1_word = midi2_bs_to_ump_read(g_bs_to_ump_converter);
            
            // Stage 2: Convert UMP (MIDI 1.0 Protocol) to UMP (MIDI 2.0 Protocol)  
            midi2_ump_to_midi2_process(g_ump_to_midi2_converter, ump_midi1_word);
            
            // Process converted MIDI 2.0 UMP messages
            uint8_t word_count = 0;
            while (midi2_ump_to_midi2_available(g_ump_to_midi2_converter) && word_count < 4) {
              ump_data[word_count] = midi2_ump_to_midi2_read(g_ump_to_midi2_converter);
              word_count++;
            }
            
            // Send MIDI 2.0 UMP message to USB if we have data
            if (word_count > 0) {
              // Clear unused words (but don't send them)
              for (uint8_t j = word_count; j < 4; j++) {
                ump_data[j] = 0;
              }
              
              // Send UMP message to USB - queue will contain proper word count info
              xQueueSend(xUmpTxQueue, ump_data, 0);
            }
          }
        }
      } else {
        // Multi-byte message (Note On/Off, CC, Pitch Bend, etc.)
        
        // Stage 1: Convert MIDI 1.0 bytes to UMP (MIDI 1.0 Protocol)
        for (uint8_t i = 0; i < midi_packet.length; i++) {
          midi2_bs_to_ump_process_byte(g_bs_to_ump_converter, midi_packet.data[i]);
        }
        
        // Process available UMP messages and convert to MIDI 2.0
        while (midi2_bs_to_ump_available(g_bs_to_ump_converter)) {
          uint32_t ump_midi1_word = midi2_bs_to_ump_read(g_bs_to_ump_converter);
          
          // Stage 2: Convert UMP (MIDI 1.0 Protocol) to UMP (MIDI 2.0 Protocol)
          midi2_ump_to_midi2_process(g_ump_to_midi2_converter, ump_midi1_word);
          
          // Process converted MIDI 2.0 UMP messages
          uint8_t word_count = 0;
          while (midi2_ump_to_midi2_available(g_ump_to_midi2_converter) && word_count < 4) {
            ump_data[word_count] = midi2_ump_to_midi2_read(g_ump_to_midi2_converter);
            word_count++;
          }
          
          // Send MIDI 2.0 UMP message to USB if we have data
          if (word_count > 0) {
            // Clear unused words (but don't send them)
            for (uint8_t j = word_count; j < 4; j++) {
              ump_data[j] = 0;
            }
            
            // Send UMP message to USB - queue will contain proper word count info
            xQueueSend(xUmpTxQueue, ump_data, 0);
          }
        }
      }
    }
  }
}

/**
  * @brief  MIDI 2.0 Task: Convert UMP to UART MIDI 1.0 using AM MIDI 2.0 Library
  * @param  pvParameters: Task parameters
  * @retval None
  */
void vMidi2UmpToUartTask(void *pvParameters)
{
  /* Prevent unused parameter warning */    
  (void)pvParameters;
  
  uint32_t ump_data[4];
  MIDIPacket_t midi_packet;
  TickType_t ledOnTime = 0;  // LED turn on time
  const TickType_t MIN_LED_ON_TIME = pdMS_TO_TICKS(MIDI_TX_LED_MIN_ON_TIME_MS);  // Minimum LED on time
  TickType_t lastActiveSensingTime = xTaskGetTickCount();  // Initialize for Active Sensing
  const TickType_t ACTIVE_SENSING_INTERVAL = pdMS_TO_TICKS(300);  // 300ms interval (MIDI standard)
  
  for(;;)
  {
    // Check if LED needs to be turned off
    if (ledOnTime != 0) {
      TickType_t currentTime = xTaskGetTickCount();
      if ((currentTime - ledOnTime) >= MIN_LED_ON_TIME) {
        if (xSemaphoreTake(xLedMutex, 0) == pdTRUE) {
          HAL_GPIO_WritePin(TxMIDI_GPIO_Port, TxMIDI_Pin, GPIO_PIN_RESET);
          xSemaphoreGive(xLedMutex);
        }
        ledOnTime = 0;
      }
    }
    
    // Wait for UMP message from USB (with timeout for LED update)
    if (xQueueReceive(xUmpRxQueue, ump_data, pdMS_TO_TICKS(10)) == pdTRUE) {
      
      // Process each UMP word through the converter
      for (int i = 0; i < 4; i++) {
        if (ump_data[i] != 0) {
          midi2_ump_to_midi1_process(g_ump_to_midi1_converter, ump_data[i]);
        }
      }
      
      // Read resulting MIDI 1.0 bytes and assemble complete messages
      static uint8_t midi_buffer[3];
      static uint8_t midi_index = 0;
      static uint8_t expected_length = 0;
      
      while (midi2_ump_to_midi1_available(g_ump_to_midi1_converter)) {
        uint8_t midi_byte = midi2_ump_to_midi1_read(g_ump_to_midi1_converter);
        
        // Handle running status and message assembly
        if (midi_byte & 0x80) {  // Status byte
          // If we had an incomplete message, send what we have
          if (midi_index > 0) {
            midi_packet.length = midi_index;
            memcpy(midi_packet.data, midi_buffer, midi_index);
            
            // Turn on LED before sending
            if (xSemaphoreTake(xLedMutex, 0) == pdTRUE) {
              HAL_GPIO_WritePin(TxMIDI_GPIO_Port, TxMIDI_Pin, GPIO_PIN_SET);
              xSemaphoreGive(xLedMutex);
            }
            ledOnTime = xTaskGetTickCount();
            
            UART_TX_SendDMA(midi_packet.data, midi_packet.length);
          }
          
          // Start new message
          midi_buffer[0] = midi_byte;
          midi_index = 1;
          
          // Determine expected message length
          uint8_t status = midi_byte & 0xF0;
          switch (status) {
            case 0x80:  // Note Off
            case 0x90:  // Note On
            case 0xA0:  // Aftertouch
            case 0xB0:  // Control Change
            case 0xE0:  // Pitch Bend
              expected_length = 3;
              break;
            case 0xC0:  // Program Change
            case 0xD0:  // Channel Pressure
              expected_length = 2;
              break;
            case 0xF0:  // System messages
              if (midi_byte == 0xF0) {  // SysEx - handle separately
                expected_length = 0;  // Variable length
              } else if (midi_byte == 0xF1 || midi_byte == 0xF3) {
                expected_length = 2;
              } else if (midi_byte == 0xF2) {
                expected_length = 3;
              } else {
                expected_length = 1;  // Real-time messages
              }
              break;
            default:
              expected_length = 1;
              break;
          }
          
          // Send immediately if single-byte message
          if (expected_length == 1) {
            midi_packet.data[0] = midi_byte;
            midi_packet.length = 1;
            
            // Turn on LED before sending
            if (xSemaphoreTake(xLedMutex, 0) == pdTRUE) {
              HAL_GPIO_WritePin(TxMIDI_GPIO_Port, TxMIDI_Pin, GPIO_PIN_SET);
              xSemaphoreGive(xLedMutex);
            }
            ledOnTime = xTaskGetTickCount();
            
            UART_TX_SendDMA(midi_packet.data, midi_packet.length);
            midi_index = 0;
          }
        } else {  // Data byte
          if (midi_index > 0 && midi_index < 3) {
            midi_buffer[midi_index++] = midi_byte;
            
            // Check if message is complete
            if (midi_index == expected_length) {
              midi_packet.length = expected_length;
              memcpy(midi_packet.data, midi_buffer, expected_length);
              
              // Turn on LED before sending
              if (xSemaphoreTake(xLedMutex, 0) == pdTRUE) {
                HAL_GPIO_WritePin(TxMIDI_GPIO_Port, TxMIDI_Pin, GPIO_PIN_SET);
                xSemaphoreGive(xLedMutex);
              }
              ledOnTime = xTaskGetTickCount();
              
              UART_TX_SendDMA(midi_packet.data, midi_packet.length);
              midi_index = 0;
            }
          }
        }
      }
    }
    
    // Process Active Sensing for MIDI 2.0 mode
#if MIDI_AUTO_ACTIVE_SENSING
    TickType_t currentTime = xTaskGetTickCount();
    if ((currentTime - lastActiveSensingTime) > ACTIVE_SENSING_INTERVAL) {
      // Send Active Sensing as MIDI 1.0 message
      midi_packet.data[0] = MIDI_ACTIVE_SENSING;
      midi_packet.length = 1;
      
      // Turn on LED before sending
      if (xSemaphoreTake(xLedMutex, 0) == pdTRUE) {
        HAL_GPIO_WritePin(TxMIDI_GPIO_Port, TxMIDI_Pin, GPIO_PIN_SET);
        xSemaphoreGive(xLedMutex);
      }
      ledOnTime = xTaskGetTickCount();
      
      UART_TX_SendDMA(midi_packet.data, midi_packet.length);
      lastActiveSensingTime = currentTime;
    }
#endif
  }
}

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initialize MIDI 2.0 converter instances
  * @retval pdPASS if successful, pdFAIL otherwise
  */
static BaseType_t InitMIDI2Converters(void)
{
  g_bs_to_ump_converter = midi2_bs_to_ump_create();
  if (g_bs_to_ump_converter == NULL) {
    return pdFAIL;
  }
  
  g_ump_to_midi2_converter = midi2_ump_to_midi2_create();
  if (g_ump_to_midi2_converter == NULL) {
    midi2_bs_to_ump_destroy(g_bs_to_ump_converter);
    g_bs_to_ump_converter = NULL;
    return pdFAIL;
  }
  
  g_ump_to_midi1_converter = midi2_ump_to_midi1_create();
  if (g_ump_to_midi1_converter == NULL) {
    midi2_bs_to_ump_destroy(g_bs_to_ump_converter);
    midi2_ump_to_midi2_destroy(g_ump_to_midi2_converter);
    g_bs_to_ump_converter = NULL;
    g_ump_to_midi2_converter = NULL;
    return pdFAIL;
  }
  
  return pdPASS;
}
