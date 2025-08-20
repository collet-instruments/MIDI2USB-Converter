/**
  * @file           : midi_common.h
  * @brief          : MIDI common definitions and structures
  */

#ifndef __MIDI_COMMON_H__
#define __MIDI_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#ifndef TESTING
#include "main.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#else
#include <main.h>
#include "mock_freertos.h"
#endif

/* Exported types ------------------------------------------------------------*/
// Simple MIDI message structure
// MIDI message structure for parsing and conversion (unified)
typedef struct {
    uint8_t data[3];      // MIDI message data (status + data bytes)
    uint8_t length;       // Message length (1-3 bytes)
    uint32_t timestamp;   // Timestamp (for future use)
} MidiMessage_t;

// Legacy alias for compatibility
typedef MidiMessage_t MIDIMessage_t;

// MIDI packet structure for queue communication
typedef struct {
    uint8_t data[4];  // MIDI packet data (cable number + 3 bytes MIDI)
    uint8_t length;   // Actual length of MIDI data (1-3 bytes)
} MIDIPacket_t;

// MIDI statistics structure for debugging
typedef struct {
    uint32_t uart_rx_count;
    uint32_t uart_tx_count;
    uint32_t usb_rx_count;
    uint32_t usb_tx_count;
    uint32_t uart_rx_errors;
    uint32_t uart_tx_errors;
    uint32_t usb_errors;
    uint32_t dma_overruns;
    uint32_t queue_full_errors;
} MIDIStats_t;

/* Exported constants --------------------------------------------------------*/
#define DMA_RX_BUFFER_SIZE 64

// MIDI filter settings
#define MIDI_FILTER_ACTIVE_SENSING 1  // Set to 0 to pass through Active Sensing (0xFE)
#define MIDI_FILTER_TIMING_CLOCK 0    // Set to 0 to pass through Timing Clock (0xF8)
#define MIDI_AUTO_ACTIVE_SENSING 1    // Set to 0 to disable automatic Active Sensing transmission

// LED control settings
#define MIDI_RX_LED_MIN_ON_TIME_MS 1  // Minimum LED on time in milliseconds for RX visibility
#define MIDI_TX_LED_MIN_ON_TIME_MS 1  // Minimum LED on time in milliseconds for TX visibility

// MIDI Status Bytes - Channel Voice Messages (0x80-0xEF)
#define MIDI_NOTE_OFF              0x80  // Note Off
#define MIDI_NOTE_ON               0x90  // Note On
#define MIDI_POLY_KEY_PRESSURE     0xA0  // Polyphonic Key Pressure (Aftertouch)
#define MIDI_CONTROL_CHANGE        0xB0  // Control Change
#define MIDI_PROGRAM_CHANGE        0xC0  // Program Change
#define MIDI_CHANNEL_PRESSURE      0xD0  // Channel Pressure (Aftertouch)
#define MIDI_PITCH_BEND            0xE0  // Pitch Bend Change

// MIDI System Common Messages (0xF0-0xF7)
#define MIDI_SYSEX_START           0xF0  // System Exclusive Start
#define MIDI_MTC_QUARTER_FRAME     0xF1  // MIDI Time Code Quarter Frame
#define MIDI_SONG_POSITION         0xF2  // Song Position Pointer
#define MIDI_SONG_SELECT           0xF3  // Song Select
#define MIDI_TUNE_REQUEST          0xF6  // Tune Request
#define MIDI_SYSEX_END             0xF7  // System Exclusive End

// MIDI System Real-Time Messages (0xF8-0xFF)
#define MIDI_TIMING_CLOCK          0xF8  // Timing Clock
#define MIDI_START                 0xFA  // Start
#define MIDI_CONTINUE              0xFB  // Continue
#define MIDI_STOP                  0xFC  // Stop
#define MIDI_ACTIVE_SENSING        0xFE  // Active Sensing
#define MIDI_SYSTEM_RESET          0xFF  // System Reset

// MIDI Message Type Masks
#define MIDI_STATUS_MASK           0x80  // Status byte identifier
#define MIDI_CHANNEL_MASK          0x0F  // Channel number mask
#define MIDI_MESSAGE_TYPE_MASK     0xF0  // Message type mask

// USB MIDI Code Index Number (CIN) definitions
#define USB_MIDI_CIN_MISC          0x0   // Miscellaneous function codes
#define USB_MIDI_CIN_CABLE_EVENT   0x1   // Cable events
#define USB_MIDI_CIN_2BYTE_SYSCOM  0x2   // Two-byte System Common messages
#define USB_MIDI_CIN_3BYTE_SYSCOM  0x3   // Three-byte System Common messages
#define USB_MIDI_CIN_SYSEX_START   0x4   // SysEx starts or continues
#define USB_MIDI_CIN_1BYTE         0x5   // Single-byte System Common Message or SysEx ends
#define USB_MIDI_CIN_SYSEX_END_2   0x6   // SysEx ends with following two bytes
#define USB_MIDI_CIN_SYSEX_END_3   0x7   // SysEx ends with following three bytes
#define USB_MIDI_CIN_NOTE_OFF      0x8   // Note-off
#define USB_MIDI_CIN_NOTE_ON       0x9   // Note-on
#define USB_MIDI_CIN_POLY_KEYPRESS 0xA   // Poly-KeyPress
#define USB_MIDI_CIN_CTRL_CHANGE   0xB   // Control Change
#define USB_MIDI_CIN_PROG_CHANGE   0xC   // Program Change
#define USB_MIDI_CIN_CHAN_PRESSURE 0xD   // Channel Pressure
#define USB_MIDI_CIN_PITCH_BEND    0xE   // PitchBend Change
#define USB_MIDI_CIN_1BYTE_DATA    0xF   // Single Byte

/* Exported variables --------------------------------------------------------*/
extern QueueHandle_t xUartToUsbQueue;  // UART RX -> USB TX
extern QueueHandle_t xUsbToUartQueue;  // USB RX -> UART TX
#ifndef TESTING
extern UART_HandleTypeDef huart2;
#endif

// DMA circular buffer for UART2 RX (RX MIDI)
extern uint8_t dma_rx_buffer[DMA_RX_BUFFER_SIZE];
extern volatile uint32_t dma_rx_head;  // DMA write position
extern volatile uint32_t dma_rx_tail;  // Processing read position

// MIDI parsing state
extern volatile uint8_t midi_msg_buffer[3];
extern volatile uint8_t midi_msg_index;
extern volatile uint8_t midi_running_status;

// MIDI statistics
extern MIDIStats_t midi_stats;

// Mutex for LED control
extern SemaphoreHandle_t xLedMutex;

/* Exported functions prototypes ---------------------------------------------*/
BaseType_t MIDI_InitQueues(void);
uint8_t MIDI_GetCIN(uint8_t status, uint8_t length);
void MIDI_ResetRunningStatus(void);
void MIDI_GetStatistics(MIDIStats_t* stats);
uint8_t MIDI_GetLengthFromCIN(uint8_t cin);
uint8_t MIDI_ToUsbPacket(const MIDIMessage_t* midi_msg, uint8_t* usb_packet, uint8_t cable);
uint8_t MIDI_FromUsbPacket(const uint8_t* usb_packet, MIDIMessage_t* midi_msg);
void MIDI_InitStats(void);
const MIDIStats_t* MIDI_GetStats(void);
uint8_t MIDI_GetExpectedLength(uint8_t status);

#ifdef __cplusplus
}
#endif

#endif /* __MIDI_COMMON_H__ */
