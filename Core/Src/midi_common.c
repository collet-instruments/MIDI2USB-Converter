/**
  * @file           : midi_common.c
  * @brief          : MIDI common implementation
  */

/* Includes ------------------------------------------------------------------*/
#include "midi_common.h"

/* Private variables ---------------------------------------------------------*/
// Queues for MIDI packet communication
QueueHandle_t xUartToUsbQueue;  // UART RX -> USB TX
QueueHandle_t xUsbToUartQueue;  // USB RX -> UART TX

// MIDI statistics
MIDIStats_t midi_stats = {0};

// Mutex for LED control
SemaphoreHandle_t xLedMutex = NULL;

// DMA circular buffer for UART2 RX (RX MIDI)
uint8_t dma_rx_buffer[DMA_RX_BUFFER_SIZE];
volatile uint32_t dma_rx_head = 0;  // DMA write position
volatile uint32_t dma_rx_tail = 0;  // Processing read position

// MIDI parsing state
volatile uint8_t midi_msg_buffer[3];
volatile uint8_t midi_msg_index = 0;
volatile uint8_t midi_running_status = 0;

/* Exported functions --------------------------------------------------------*/
/**
  * @brief  Initialize MIDI queues
  * @retval pdPASS if successful, pdFAIL otherwise
  */
BaseType_t MIDI_InitQueues(void)
{
  /* Create MIDI packet queues */
  // Increased queue size to handle larger SysEx messages
  // A 1024-byte SysEx requires ~342 packets of 3 bytes each
  // Using 64 to balance memory usage and capacity
  xUartToUsbQueue = xQueueCreate(64, sizeof(MIDIPacket_t));
  xUsbToUartQueue = xQueueCreate(64, sizeof(MIDIPacket_t));
  
  /* Create LED control mutex */
  xLedMutex = xSemaphoreCreateMutex();
  
  /* Check if all resources were created successfully */
  if (xUartToUsbQueue == NULL || xUsbToUartQueue == NULL || xLedMutex == NULL)
  {
    return pdFAIL;
  }
  
  return pdPASS;
}

/**
  * @brief  Get Code Index Number (CIN) for USB MIDI packet
  * @param  status: MIDI status byte
  * @param  length: MIDI message length
  * @retval CIN value for USB MIDI packet
  */
uint8_t MIDI_GetCIN(uint8_t status, uint8_t length)
{
  if (length == 1) {
    // Single byte message (system real-time)
    return USB_MIDI_CIN_1BYTE;
  } else if (length == 2) {
    uint8_t status_type = status & MIDI_MESSAGE_TYPE_MASK;
    if (status_type == MIDI_PROGRAM_CHANGE) return USB_MIDI_CIN_PROG_CHANGE;
    else if (status_type == MIDI_CHANNEL_PRESSURE) return USB_MIDI_CIN_CHAN_PRESSURE;
    else return USB_MIDI_CIN_2BYTE_SYSCOM; // Two-byte System Common
  } else if (length == 3) {
    uint8_t status_type = status & MIDI_MESSAGE_TYPE_MASK;
    switch (status_type) {
      case MIDI_NOTE_OFF:        return USB_MIDI_CIN_NOTE_OFF;
      case MIDI_NOTE_ON:         return USB_MIDI_CIN_NOTE_ON;
      case MIDI_POLY_KEY_PRESSURE: return USB_MIDI_CIN_POLY_KEYPRESS;
      case MIDI_CONTROL_CHANGE:  return USB_MIDI_CIN_CTRL_CHANGE;
      case MIDI_PITCH_BEND:      return USB_MIDI_CIN_PITCH_BEND;
      default:                   return USB_MIDI_CIN_3BYTE_SYSCOM; // Three-byte System Common
    }
  }
  return USB_MIDI_CIN_MISC; // Miscellaneous function codes
}

/**
  * @brief  Reset MIDI running status (e.g., on System messages)
  * @retval None
  */
void MIDI_ResetRunningStatus(void)
{
  midi_running_status = 0;
  midi_msg_index = 0;
}

/**
  * @brief  Get copy of MIDI statistics
  * @param  stats: Pointer to statistics structure to fill
  * @retval None
  */
void MIDI_GetStatistics(MIDIStats_t* stats)
{
  if (stats != NULL)
  {
    taskENTER_CRITICAL();
    *stats = midi_stats;
    taskEXIT_CRITICAL();
  
  }
}

/**
  * @brief  Calculate MIDI message length from USB MIDI CIN
  * @param  cin: Code Index Number from USB MIDI packet
  * @retval MIDI message length (1-3 bytes)
  */
uint8_t MIDI_GetLengthFromCIN(uint8_t cin)
{
  switch (cin) {
    case USB_MIDI_CIN_2BYTE_SYSCOM: // Two-byte System Common messages
    case USB_MIDI_CIN_PROG_CHANGE:  // Program Change
    case USB_MIDI_CIN_CHAN_PRESSURE: // Channel Pressure
      return 2;
    case USB_MIDI_CIN_3BYTE_SYSCOM: // Three-byte System Common messages
    case USB_MIDI_CIN_NOTE_OFF:     // Note-off
    case USB_MIDI_CIN_NOTE_ON:      // Note-on
    case USB_MIDI_CIN_POLY_KEYPRESS: // Poly-KeyPress
    case USB_MIDI_CIN_CTRL_CHANGE:  // Control Change
    case USB_MIDI_CIN_PITCH_BEND:   // PitchBend Change
      return 3;
    case USB_MIDI_CIN_1BYTE: // Single-byte System Common Message or SysEx ends
      return 1;
    default:
      return 3; // Default to 3 bytes
  }
}

/**
  * @brief  Calculate expected MIDI message length from UART status byte
  * @param  status: MIDI status byte
  * @retval Expected MIDI message length (1-3 bytes)
  */
uint8_t MIDI_GetExpectedLength(uint8_t status)
{
  // System Real-Time messages (0xF8-0xFF) are single byte
  if (status >= 0xF8) {
    return 1;
  }
  
  // System Common messages (0xF0-0xF7)
  if (status >= 0xF0) {
    switch (status) {
      case 0xF1: // MTC Quarter Frame
      case 0xF3: // Song Select
        return 2;
      case 0xF2: // Song Position Pointer
        return 3;
      case 0xF0: // SysEx Start (variable length, but we handle differently)
      case 0xF4: // Undefined
      case 0xF5: // Undefined  
      case 0xF6: // Tune Request
      case 0xF7: // SysEx End
      default:
        return 1;
    }
  }
  
  // Channel Voice messages (0x80-0xEF)
  uint8_t status_type = status & MIDI_MESSAGE_TYPE_MASK;
  if (status_type == MIDI_PROGRAM_CHANGE || status_type == MIDI_CHANNEL_PRESSURE) {
    return 2; // Program Change or Channel Pressure (2-byte messages)
  } else {
    return 3; // Note On/Off, Control Change, Pitch Bend, etc. (3-byte messages)
  }
}
