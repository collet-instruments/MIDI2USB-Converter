/**
  * @file           : usb_midi_task.c
  * @brief          : USB MIDI tasks implementation
  */

/* Includes ------------------------------------------------------------------*/
#include "usb_midi_task.h"
#include "uart_midi_task.h"  // For UartTxBuffer_t and UART TX functions
#include "tusb.h"
#include "semphr.h"
#include <string.h>

/* External variables --------------------------------------------------------*/
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

/* Private variables ---------------------------------------------------------*/
// DMA TX buffer and control
static UartTxBuffer_t uart_tx_buffers[2];  // Double buffering for continuous transmission
static uint8_t current_tx_buffer = 0;
SemaphoreHandle_t xUartTxCompleteSemaphore = NULL;
volatile uint8_t uart_tx_dma_busy = 0;

// SysEx buffer for accumulating multi-packet SysEx messages
#define SYSEX_BUFFER_SIZE 1024  // Increased to handle larger SysEx messages
static struct {
  uint8_t data[SYSEX_BUFFER_SIZE];
  uint16_t length;
  bool in_sysex;
  bool overflow;  // Track if buffer overflowed
} sysex_buffer = {.length = 0, .in_sysex = false, .overflow = false};

/* Private function prototypes -----------------------------------------------*/
static void ProcessUsbMidiPacket(MIDIPacket_t *midi_packet, TickType_t *ledOnTime);
static void ProcessActiveSensing(TickType_t *lastActiveSensingTime, TickType_t *ledOnTime);
static void UpdateTxLedState(TickType_t *ledOnTime);

/* Public functions ----------------------------------------------------------*/
/**
  * @brief USB RX MIDI Task - receives MIDI data from USB and forwards to UART
  * @param pvParameters: Task parameters
  * @retval None
  */
void vUsbRxMidiTask(void* pvParameters) {
  (void) pvParameters;
  
  while (1) {
    // Handle incoming USB MIDI data and forward to UART
    while (tud_midi_available()) {
      uint8_t packet[4];
      if (tud_midi_packet_read(packet)) {
        // Extract MIDI data from USB MIDI packet first
        uint8_t cable = (packet[0] & 0xF0) >> 4;  // Cable Number in upper nibble (bits 7-4)
        uint8_t cin = packet[0] & 0x0F;           // CIN in lower nibble (bits 3-0)
        
        MIDIPacket_t midi_packet;
        
        (void)cable;  // Currently only handling single cable, suppress unused warning
        
        // Determine MIDI message length based on Code Index Number (CIN)
        uint8_t midi_length = 0;
        bool is_sysex = false;
        
        switch (cin) {
          case USB_MIDI_CIN_2BYTE_SYSCOM: // Two-byte System Common messages
          case USB_MIDI_CIN_PROG_CHANGE:  // Program Change
          case USB_MIDI_CIN_CHAN_PRESSURE: // Channel Pressure
            midi_length = 2;
            break;
          case USB_MIDI_CIN_3BYTE_SYSCOM: // Three-byte System Common messages
          case USB_MIDI_CIN_NOTE_OFF:     // Note-off
          case USB_MIDI_CIN_NOTE_ON:      // Note-on
          case USB_MIDI_CIN_POLY_KEYPRESS: // Poly-KeyPress
          case USB_MIDI_CIN_CTRL_CHANGE:  // Control Change
          case USB_MIDI_CIN_PITCH_BEND:   // PitchBend Change
            midi_length = 3;
            break;
          case USB_MIDI_CIN_1BYTE: // Single-byte System Common Message or SysEx ends with 1 byte
            // Check if it's a real-time message or SysEx end
            if (packet[1] == 0xF7) { // SysEx End
              is_sysex = true;
              midi_length = 1;
            } else {
              midi_length = 1;
            }
            break;
          case USB_MIDI_CIN_SYSEX_START: // SysEx starts or continues (3 bytes)
            is_sysex = true;
            midi_length = 3;
            break;
          case USB_MIDI_CIN_SYSEX_END_2: // SysEx ends with 2 bytes
            is_sysex = true;
            midi_length = 2;
            break;
          case USB_MIDI_CIN_SYSEX_END_3: // SysEx ends with 3 bytes
            is_sysex = true;
            midi_length = 3;
            break;
          default:
            midi_length = 3; // Default to 3 bytes
            break;
        }
        
        // Handle SysEx messages differently
        if (is_sysex) {
          // Process SysEx data
          if (cin == USB_MIDI_CIN_SYSEX_START) {
            // Check if first byte is 0xF0 (SysEx start)
            if (packet[1] == 0xF0) {
              // Start new SysEx message
              sysex_buffer.length = 0;
              sysex_buffer.in_sysex = true;
              sysex_buffer.overflow = false;
              // Add all 3 bytes
              for (int i = 0; i < 3; i++) {
                if (sysex_buffer.length < SYSEX_BUFFER_SIZE) {
                  sysex_buffer.data[sysex_buffer.length++] = packet[i + 1];
                } else {
                  sysex_buffer.overflow = true;
                }
              }
            } else if (sysex_buffer.in_sysex) {
              // Continue SysEx message (no 0xF0)
              for (int i = 0; i < 3; i++) {
                if (packet[i + 1] != 0x00) { // Skip padding bytes
                  if (sysex_buffer.length < SYSEX_BUFFER_SIZE) {
                    sysex_buffer.data[sysex_buffer.length++] = packet[i + 1];
                  } else {
                    sysex_buffer.overflow = true;
                  }
                }
              }
            }
          } else if ((cin == USB_MIDI_CIN_SYSEX_END_2 || cin == USB_MIDI_CIN_SYSEX_END_3 || 
                     (cin == USB_MIDI_CIN_1BYTE && packet[1] == 0xF7)) && sysex_buffer.in_sysex) {
            // End of SysEx message
            // Add remaining bytes
            for (int i = 0; i < midi_length; i++) {
              if (packet[i + 1] != 0x00) { // Skip padding bytes
                if (sysex_buffer.length < SYSEX_BUFFER_SIZE) {
                  sysex_buffer.data[sysex_buffer.length++] = packet[i + 1];
                } else {
                  sysex_buffer.overflow = true;
                }
              }
            }
            
            // Send complete SysEx message to UART queue
            if (sysex_buffer.length > 0 && !sysex_buffer.overflow) {
              // Send directly to UART instead of chunking through queue
              // This avoids queue overflow for large SysEx
              MIDIPacket_t sysex_packet;
              
              // Send the entire SysEx message at once
              // The UART task will handle the actual transmission
              for (uint16_t offset = 0; offset < sysex_buffer.length; ) {
                uint16_t remaining = sysex_buffer.length - offset;
                uint8_t chunk_size = (remaining > 3) ? 3 : remaining;
                
                memcpy(sysex_packet.data, &sysex_buffer.data[offset], chunk_size);
                sysex_packet.length = chunk_size;
                
                // Use blocking send with timeout to ensure delivery
                if (xQueueSend(xUsbToUartQueue, &sysex_packet, pdMS_TO_TICKS(10)) == pdTRUE) {
                  midi_stats.usb_rx_count++;
                  offset += chunk_size;
                } else {
                  midi_stats.queue_full_errors++;
                  break;  // Stop if queue is persistently full
                }
              }
            } else if (sysex_buffer.overflow) {
              // Log overflow error
              midi_stats.queue_full_errors++;
            }
            
            // Reset SysEx buffer
            sysex_buffer.in_sysex = false;
            sysex_buffer.length = 0;
            sysex_buffer.overflow = false;
          }
        } else {
          // Normal MIDI message
          for (int i = 0; i < midi_length && i < 3; i++) {
            midi_packet.data[i] = packet[i + 1];
          }
          midi_packet.length = midi_length;
          
          // Optional: Filter out Active Sensing to reduce UART traffic
#if MIDI_FILTER_ACTIVE_SENSING
          if (!(midi_length == 1 && midi_packet.data[0] == MIDI_ACTIVE_SENSING)) {
#endif
            // Send to UART output queue
            if (xQueueSend(xUsbToUartQueue, &midi_packet, 0) == pdTRUE) {
              midi_stats.usb_rx_count++;
            } else {
              midi_stats.queue_full_errors++;
            }
#if MIDI_FILTER_ACTIVE_SENSING
          }
#endif
        }
      }
    }
    
    // Small delay to prevent overwhelming the CPU
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

/**
  * @brief Initialize UART TX DMA
  * @retval None
  */
void UART_TX_DMA_Init(void) {
  // Create semaphore for TX completion
  xUartTxCompleteSemaphore = xSemaphoreCreateBinary();
  if (xUartTxCompleteSemaphore != NULL) {
    // Give semaphore initially (TX is ready)
    xSemaphoreGive(xUartTxCompleteSemaphore);
  }
  
  // Initialize buffers
  for (int i = 0; i < 2; i++) {
    uart_tx_buffers[i].in_use = 0;
    uart_tx_buffers[i].length = 0;
  }
}

/**
  * @brief Send data via UART DMA
  * @param data: Data to send
  * @param length: Length of data
  * @retval pdTRUE if successful, pdFALSE if busy
  */
BaseType_t UART_TX_SendDMA(const uint8_t *data, uint16_t length) {
  if (length == 0 || length > UART_TX_BUFFER_SIZE) {
    return pdFALSE;
  }
  
  // Wait for DMA to be ready (with timeout)
  if (xSemaphoreTake(xUartTxCompleteSemaphore, pdMS_TO_TICKS(10)) != pdTRUE) {
    return pdFALSE;  // DMA still busy
  }
  
  // Select buffer and copy data
  UartTxBuffer_t *tx_buffer = &uart_tx_buffers[current_tx_buffer];
  memcpy(tx_buffer->data, data, length);
  tx_buffer->length = length;
  tx_buffer->in_use = 1;
  
  // Start DMA transmission
  uart_tx_dma_busy = 1;
  HAL_StatusTypeDef status = HAL_UART_Transmit_DMA(&huart2, tx_buffer->data, length);
  if (status != HAL_OK) {
    // DMA start failed
    tx_buffer->in_use = 0;
    uart_tx_dma_busy = 0;
    xSemaphoreGive(xUartTxCompleteSemaphore);
    return pdFALSE;
  }
  
  // Note: Semaphore will be given back in HAL_UART_TxCpltCallback
  
  // Switch to other buffer for next transmission
  current_tx_buffer = 1 - current_tx_buffer;
  
  return pdTRUE;
}

/**
  * @brief Process USB MIDI packet and send to UART
  * @param midi_packet: MIDI packet to process
  * @param ledOnTime: Pointer to LED on time
  * @retval None
  */
static void ProcessUsbMidiPacket(MIDIPacket_t *midi_packet, TickType_t *ledOnTime) {
  // Turn on TxMIDI LED before transmission
  if (xSemaphoreTake(xLedMutex, 0) == pdTRUE) {
    HAL_GPIO_WritePin(TxMIDI_GPIO_Port, TxMIDI_Pin, GPIO_PIN_SET);
    xSemaphoreGive(xLedMutex);
  }
  *ledOnTime = xTaskGetTickCount();
  
  // Send MIDI data to UART2 (TX MIDI) via DMA
  if (UART_TX_SendDMA(midi_packet->data, midi_packet->length) == pdTRUE) {
    midi_stats.uart_tx_count++;
  } else {
    midi_stats.uart_tx_errors++;
    // If DMA is busy, wait a bit
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

/**
  * @brief Process Active Sensing transmission
  * @param lastActiveSensingTime: Pointer to last Active Sensing time
  * @param ledOnTime: Pointer to LED on time
  * @retval None
  */
static void ProcessActiveSensing(TickType_t *lastActiveSensingTime, TickType_t *ledOnTime) {
#if MIDI_AUTO_ACTIVE_SENSING
  TickType_t currentTime = xTaskGetTickCount();
  if ((currentTime - *lastActiveSensingTime) > pdMS_TO_TICKS(300)) {
    uint8_t activeSensing = MIDI_ACTIVE_SENSING;
    if (UART_TX_SendDMA(&activeSensing, 1) == pdTRUE) {
      *lastActiveSensingTime = currentTime;
      
      // Turn on LED for Active Sensing
      if (xSemaphoreTake(xLedMutex, 0) == pdTRUE) {
        HAL_GPIO_WritePin(TxMIDI_GPIO_Port, TxMIDI_Pin, GPIO_PIN_SET);
        xSemaphoreGive(xLedMutex);
      }
      *ledOnTime = xTaskGetTickCount();
    }
  }
#endif
}

/**
  * @brief Update TX LED state based on minimum on time
  * @param ledOnTime: Pointer to LED on time
  * @retval None
  */
static void UpdateTxLedState(TickType_t *ledOnTime) {
  const TickType_t MIN_LED_ON_TIME = pdMS_TO_TICKS(MIDI_TX_LED_MIN_ON_TIME_MS);
  
  if (*ledOnTime != 0) {
    TickType_t currentTime = xTaskGetTickCount();
    if ((currentTime - *ledOnTime) >= MIN_LED_ON_TIME) {
      if (xSemaphoreTake(xLedMutex, 0) == pdTRUE) {
        HAL_GPIO_WritePin(TxMIDI_GPIO_Port, TxMIDI_Pin, GPIO_PIN_RESET);
        xSemaphoreGive(xLedMutex);
      }
      *ledOnTime = 0;
    }
  }
}

/**
  * @brief USB to UART Task - receives MIDI packets from queue and sends to UART
  * @param pvParameters: Task parameters
  * @retval None
  */
void vUsbToUartTask(void *pvParameters) {
  (void) pvParameters;
  MIDIPacket_t midi_packet;
  TickType_t lastActiveSensingTime = xTaskGetTickCount();  // Initialize to current time for immediate Active Sensing
  TickType_t ledOnTime = 0;  // LED turn on time
  
  while (1) {
    // Wait for MIDI packet from USB RX (with timeout for Active Sensing)
    if (xQueueReceive(xUsbToUartQueue, &midi_packet, pdMS_TO_TICKS(10)) == pdTRUE) {
      ProcessUsbMidiPacket(&midi_packet, &ledOnTime);
      
      // Reset Active Sensing timer only on non-Active Sensing messages
      if (!(midi_packet.length == 1 && midi_packet.data[0] == MIDI_ACTIVE_SENSING)) {
        lastActiveSensingTime = xTaskGetTickCount();
      }
    }
    
    // Update LED state
    UpdateTxLedState(&ledOnTime);
    
    // Process Active Sensing
    ProcessActiveSensing(&lastActiveSensingTime, &ledOnTime);
  }
}
