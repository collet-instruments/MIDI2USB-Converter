/**
  * @file           : uart_midi_task.c
  * @brief          : UART MIDI tasks implementation
  */

/* Includes ------------------------------------------------------------------*/
#include "uart_midi_task.h"
#include "tusb.h"
#include <string.h>
#include <stdbool.h>

/* External variables --------------------------------------------------------*/
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart1;

/* Private variables ---------------------------------------------------------*/
static TickType_t rxLedOnTime = 0;  // Shared LED on time

// SysEx buffer for accumulating SysEx messages from UART
#define UART_SYSEX_BUFFER_SIZE 1024  // Increased to handle larger SysEx messages
static struct {
  uint8_t data[UART_SYSEX_BUFFER_SIZE];
  uint16_t length;
  bool in_sysex;
  bool overflow;  // Track if buffer overflowed
} uart_sysex_buffer = {.length = 0, .in_sysex = false, .overflow = false};

/* Private function prototypes -----------------------------------------------*/
static void CheckDmaBufferOverrun(void);
static void ProcessMidiByte(uint8_t rx_byte);
static void UpdateRxLedState(void);
static void SendCompleteMessage(void);
static void TurnOnRxLed(void);

/* Public functions ----------------------------------------------------------*/
/**
  * @brief UART RX MIDI Task - receives MIDI data from UART2 DMA circular buffer and parses it
  * @param pvParameters: Task parameters
  * @retval None
  */
/**
  * @brief Check for DMA buffer overrun
  * @retval None
  */
static void CheckDmaBufferOverrun(void) {
  uint32_t bytes_available = (dma_rx_head >= dma_rx_tail) ? 
                            (dma_rx_head - dma_rx_tail) : 
                            (DMA_RX_BUFFER_SIZE - dma_rx_tail + dma_rx_head);
  
  if (bytes_available > (DMA_RX_BUFFER_SIZE - 4)) {
    // Buffer overrun detected
    midi_stats.dma_overruns++;
    dma_rx_tail = dma_rx_head;  // Reset to catch up
    MIDI_ResetRunningStatus();   // Reset MIDI state
  }
}

/**
  * @brief Turn on RX LED
  * @retval None
  */
static void TurnOnRxLed(void) {
  if (xSemaphoreTake(xLedMutex, 0) == pdTRUE) {
    HAL_GPIO_WritePin(RxMIDI_GPIO_Port, RxMIDI_Pin, GPIO_PIN_SET);
    xSemaphoreGive(xLedMutex);
  }
  rxLedOnTime = xTaskGetTickCount();
}

/**
  * @brief Send complete MIDI message to USB queue
  * @retval None
  */
static void SendCompleteMessage(void) {
  
  uint8_t expected_length = 3;
  uint8_t status_type = midi_running_status & MIDI_MESSAGE_TYPE_MASK;
  
  if (status_type == MIDI_PROGRAM_CHANGE || status_type == MIDI_CHANNEL_PRESSURE) {
    expected_length = 2;
  }
  
  if (midi_msg_index >= expected_length) {
    MIDIPacket_t midi_packet;
    for (int i = 0; i < expected_length; i++) {
      midi_packet.data[i] = midi_msg_buffer[i];
    }
    midi_packet.length = expected_length;
    
    // Send to USB output queue
    if (xQueueSend(xUartToUsbQueue, &midi_packet, 0) != pdTRUE) {
      midi_stats.queue_full_errors++;
    }
    
    // Turn on LED when message is sent
    TurnOnRxLed();
    
    // Reset for next message (keep running status)
    midi_msg_buffer[0] = midi_running_status;
    midi_msg_index = 1;
  }
}

/**
  * @brief Process a single MIDI byte
  * @param rx_byte: Received MIDI byte
  * @retval None
  */
static void ProcessMidiByte(uint8_t rx_byte) {
  
  midi_stats.uart_rx_count++;
  
  if (rx_byte & 0x80) {
    // Status byte
    if (rx_byte == MIDI_SYSEX_START) {
      // Start SysEx message
      uart_sysex_buffer.in_sysex = true;
      uart_sysex_buffer.length = 0;
      uart_sysex_buffer.overflow = false;
      if (uart_sysex_buffer.length < UART_SYSEX_BUFFER_SIZE) {
        uart_sysex_buffer.data[uart_sysex_buffer.length++] = rx_byte;
      } else {
        uart_sysex_buffer.overflow = true;
      }
      midi_msg_index = 0;
      return;
    } else if (rx_byte == MIDI_SYSEX_END) {
      // End SysEx message
      if (uart_sysex_buffer.in_sysex) {
        if (uart_sysex_buffer.length < UART_SYSEX_BUFFER_SIZE) {
          uart_sysex_buffer.data[uart_sysex_buffer.length++] = rx_byte;
        } else {
          uart_sysex_buffer.overflow = true;
        }
        
        // Send complete SysEx message to USB queue if no overflow
        if (!uart_sysex_buffer.overflow) {
          for (uint16_t offset = 0; offset < uart_sysex_buffer.length; ) {
            MIDIPacket_t sysex_packet;
            uint16_t remaining = uart_sysex_buffer.length - offset;
            uint8_t chunk_size = (remaining > 3) ? 3 : remaining;
            
            memcpy(sysex_packet.data, &uart_sysex_buffer.data[offset], chunk_size);
            sysex_packet.length = chunk_size;
            
            // Use blocking send with timeout to ensure delivery
            if (xQueueSend(xUartToUsbQueue, &sysex_packet, pdMS_TO_TICKS(10)) == pdTRUE) {
              offset += chunk_size;
            } else {
              midi_stats.queue_full_errors++;
              break;  // Stop if queue is persistently full
            }
          }
        } else {
          // Log overflow error
          midi_stats.queue_full_errors++;
        }
        
        // Turn on LED when message is sent
        TurnOnRxLed();
      }
      uart_sysex_buffer.in_sysex = false;
      uart_sysex_buffer.length = 0;
      uart_sysex_buffer.overflow = false;
      midi_msg_index = 0;
      return;
    } else if (uart_sysex_buffer.in_sysex) {
      // In SysEx but received non-SysEx status byte - abort SysEx
      uart_sysex_buffer.in_sysex = false;
      uart_sysex_buffer.length = 0;
      uart_sysex_buffer.overflow = false;
    }
    
    // Normal status byte processing
    midi_running_status = rx_byte;
    midi_msg_buffer[0] = rx_byte;
    midi_msg_index = 1;
    
    // Check if this is a single-byte message
    if (rx_byte >= 0xF8) {
      // Real-time message (single byte)
#if MIDI_FILTER_ACTIVE_SENSING
      if (rx_byte == MIDI_ACTIVE_SENSING) {
        midi_msg_index = 0;
        return;
      }
#endif

#if MIDI_FILTER_TIMING_CLOCK
      if (rx_byte == MIDI_TIMING_CLOCK) {
        midi_msg_index = 0;
        return;
      }     
#endif
      MIDIPacket_t midi_packet;
      midi_packet.data[0] = rx_byte;
      midi_packet.length = 1;
      if (xQueueSend(xUartToUsbQueue, &midi_packet, 0) != pdTRUE) {
        midi_stats.queue_full_errors++;
      }
      midi_msg_index = 0;
      
      // Turn on LED when message is sent
      TurnOnRxLed();
    } else if (rx_byte >= 0xF0) {
      // System Common message - reset running status
      MIDI_ResetRunningStatus();
      midi_msg_buffer[0] = rx_byte;
      midi_msg_index = 1;
    }
  } else {
    // Data byte
    if (uart_sysex_buffer.in_sysex) {
      // In SysEx - accumulate data
      if (uart_sysex_buffer.length < UART_SYSEX_BUFFER_SIZE) {
        uart_sysex_buffer.data[uart_sysex_buffer.length++] = rx_byte;
      } else {
        uart_sysex_buffer.overflow = true;
      }
    } else if (midi_running_status != 0 && midi_msg_index < 3) {
      // Normal MIDI data byte
      midi_msg_buffer[midi_msg_index] = rx_byte;
      midi_msg_index++;
      
      // Check if we have a complete message
      SendCompleteMessage();
    }
  }
}

/**
  * @brief Update RX LED state based on minimum on time
  * @retval None
  */
static void UpdateRxLedState(void) {
  const TickType_t MIN_LED_ON_TIME = pdMS_TO_TICKS(MIDI_RX_LED_MIN_ON_TIME_MS);
  
  if (rxLedOnTime != 0) {
    TickType_t currentTime = xTaskGetTickCount();
    if ((currentTime - rxLedOnTime) >= MIN_LED_ON_TIME) {
      if (xSemaphoreTake(xLedMutex, 0) == pdTRUE) {
        HAL_GPIO_WritePin(RxMIDI_GPIO_Port, RxMIDI_Pin, GPIO_PIN_RESET);
        xSemaphoreGive(xLedMutex);
      }
      rxLedOnTime = 0;
    }
  }
}

/**
  * @brief UART RX MIDI Task - receives MIDI data from UART2 DMA circular buffer and parses it
  * @param pvParameters: Task parameters
  * @retval None
  */
void vUartRxMidiTask(void *pvParameters) {
  (void) pvParameters;
  
  // Start DMA reception in circular mode for UART2
  if (HAL_UART_Receive_DMA(&huart2, dma_rx_buffer, DMA_RX_BUFFER_SIZE) != HAL_OK) {
    // DMA initialization failed
    vTaskDelete(NULL);
    return;
  }
  
  while (1) {
    // Update DMA head position with critical section
    taskENTER_CRITICAL();
    dma_rx_head = DMA_RX_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(huart2.hdmarx);
    taskEXIT_CRITICAL();
    
    // Check for buffer overrun
    CheckDmaBufferOverrun();
    
    // Process all available bytes in circular buffer
    while (dma_rx_tail != dma_rx_head) {
      uint8_t rx_byte = dma_rx_buffer[dma_rx_tail];
      dma_rx_tail = (dma_rx_tail + 1) % DMA_RX_BUFFER_SIZE;
      
      ProcessMidiByte(rx_byte);
    }
    
    // Update LED state
    UpdateRxLedState();
    
    // Small delay to prevent overwhelming the CPU
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

/**
  * @brief UART to USB MIDI Task - forwards MIDI packets from UART to USB
  * @param pvParameters: Task parameters
  * @retval None
  */
void vUartToUsbTask(void *pvParameters) {
  (void) pvParameters;
  MIDIPacket_t midi_packet;
  
  while (1) {
    // Wait for MIDI packet from UART RX
    if (xQueueReceive(xUartToUsbQueue, &midi_packet, portMAX_DELAY) == pdTRUE) {
      // Convert MIDI message to USB MIDI packet
      uint8_t usb_packet[4] = {0};
      uint8_t cin;
      
      // Check if this is a SysEx message
      if (midi_packet.data[0] == MIDI_SYSEX_START) {
        // SysEx start or continue
        cin = USB_MIDI_CIN_SYSEX_START;
      } else if (midi_packet.data[midi_packet.length - 1] == MIDI_SYSEX_END) {
        // SysEx end - determine CIN based on length
        if (midi_packet.length == 1) {
          cin = USB_MIDI_CIN_1BYTE;
        } else if (midi_packet.length == 2) {
          cin = USB_MIDI_CIN_SYSEX_END_2;
        } else {
          cin = USB_MIDI_CIN_SYSEX_END_3;
        }
      } else if (midi_packet.length == 3 && (midi_packet.data[0] < 0x80 || 
                (midi_packet.data[1] < 0x80 && midi_packet.data[2] < 0x80))) {
        // SysEx continue (all data bytes)
        cin = USB_MIDI_CIN_SYSEX_START;
      } else {
        // Normal MIDI message
        cin = MIDI_GetCIN(midi_packet.data[0], midi_packet.length);
      }
      
      // Create USB MIDI packet
      usb_packet[0] = (0x00 << 4) | cin; // Cable 0 in upper nibble, CIN in lower nibble
      for (int i = 0; i < midi_packet.length && i < 3; i++) {
        usb_packet[i + 1] = midi_packet.data[i];
      }
      
      // Send USB MIDI packet
      if (tud_mounted() && tud_midi_mounted()) {
        if (tud_midi_packet_write(usb_packet)) {
          midi_stats.usb_tx_count++;
        } else {
          midi_stats.usb_errors++;
          // Add delay when buffer is full to prevent overwhelming
          vTaskDelay(pdMS_TO_TICKS(1));
        }
      } else {
        midi_stats.usb_errors++;
      }
    }
  }
}
