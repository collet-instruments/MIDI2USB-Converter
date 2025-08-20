#ifndef __MIDI_COMMON_H__
#define __MIDI_COMMON_H__

#include <stdint.h>
#include "mock_freertos.h"

// DMA buffer size
#define DMA_RX_BUFFER_SIZE 64

// MIDI message structure for parsing and conversion
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

// MIDI statistics structure
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

// MIDI Status Bytes - Channel Voice Messages
#define MIDI_NOTE_OFF              0x80
#define MIDI_NOTE_ON               0x90
#define MIDI_POLY_KEY_PRESSURE     0xA0
#define MIDI_CONTROL_CHANGE        0xB0
#define MIDI_PROGRAM_CHANGE        0xC0
#define MIDI_CHANNEL_PRESSURE      0xD0
#define MIDI_PITCH_BEND            0xE0

// MIDI System Common Messages
#define MIDI_SYSEX_START           0xF0
#define MIDI_MTC_QUARTER_FRAME     0xF1
#define MIDI_SONG_POSITION         0xF2
#define MIDI_SONG_SELECT           0xF3
#define MIDI_TUNE_REQUEST          0xF6
#define MIDI_SYSEX_END             0xF7

// MIDI System Real-Time Messages
#define MIDI_TIMING_CLOCK          0xF8
#define MIDI_START                 0xFA
#define MIDI_CONTINUE              0xFB
#define MIDI_STOP                  0xFC
#define MIDI_ACTIVE_SENSING        0xFE
#define MIDI_RESET                 0xFF

// MIDI Parsing constants
#define MIDI_STATUS_MASK           0x80
#define MIDI_CHANNEL_MASK          0x0F
#define MIDI_MESSAGE_TYPE_MASK     0xF0

// USB MIDI Class-Specific Codes (Cable Number / Code Index Number)
#define USB_MIDI_CIN_MISC          0x00
#define USB_MIDI_CIN_CABLE_EVENT   0x01
#define USB_MIDI_CIN_2BYTE         0x02
#define USB_MIDI_CIN_3BYTE         0x03
#define USB_MIDI_CIN_SYSEX_START   0x04
#define USB_MIDI_CIN_1BYTE         0x05
#define USB_MIDI_CIN_SYSEX_END_1   0x05
#define USB_MIDI_CIN_SYSEX_END_2   0x06
#define USB_MIDI_CIN_SYSEX_END_3   0x07
#define USB_MIDI_CIN_NOTE_OFF      0x08
#define USB_MIDI_CIN_NOTE_ON       0x09
#define USB_MIDI_CIN_POLY_KEY_PRESS 0x0A
#define USB_MIDI_CIN_POLY_KEYPRESS  0x0A
#define USB_MIDI_CIN_CONTROL_CHANGE 0x0B
#define USB_MIDI_CIN_CTRL_CHANGE    0x0B
#define USB_MIDI_CIN_PROG_CHANGE   0x0C
#define USB_MIDI_CIN_CHAN_PRESSURE 0x0D
#define USB_MIDI_CIN_PITCH_BEND    0x0E
#define USB_MIDI_CIN_SINGLE_BYTE   0x0F
#define USB_MIDI_CIN_2BYTE_SYSCOM  0x02
#define USB_MIDI_CIN_3BYTE_SYSCOM  0x03

extern MIDIStats_t midi_stats;
extern QueueHandle_t xUartToUsbQueue;
extern QueueHandle_t xUsbToUartQueue;
extern SemaphoreHandle_t xLedMutex;
extern uint8_t dma_rx_buffer[DMA_RX_BUFFER_SIZE];

// Function prototypes
BaseType_t MIDI_InitQueues(void);
uint8_t MIDI_GetCIN(uint8_t status, uint8_t length);
uint8_t MIDI_GetLengthFromCIN(uint8_t cin);
uint8_t MIDI_GetExpectedLength(uint8_t status);

#endif /* __MIDI_COMMON_H__ */