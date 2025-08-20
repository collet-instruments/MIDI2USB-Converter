/**
  * @file           : midi2_wrapper.cpp
  * @brief          : C wrapper for AM MIDI 2.0 Library
  */

/* Includes ------------------------------------------------------------------*/
#include "midi2_wrapper.h"
#include "bytestreamToUMP.h"
#include "umpToBytestream.h"
#include "umpToMIDI2Protocol.h"

extern "C" {

/* Bytestream to UMP converter implementation */
midi2_converter_handle_t midi2_bs_to_ump_create(void) {
    return new bytestreamToUMP();
}

void midi2_bs_to_ump_destroy(midi2_converter_handle_t handle) {
    if (handle) {
        delete static_cast<bytestreamToUMP*>(handle);
    }
}

bool midi2_bs_to_ump_process_byte(midi2_converter_handle_t handle, uint8_t byte) {
    if (!handle) return false;
    bytestreamToUMP* converter = static_cast<bytestreamToUMP*>(handle);
    converter->bytestreamParse(byte);
    return true;
}

bool midi2_bs_to_ump_available(midi2_converter_handle_t handle) {
    if (!handle) return false;
    bytestreamToUMP* converter = static_cast<bytestreamToUMP*>(handle);
    return converter->availableUMP();
}

uint32_t midi2_bs_to_ump_read(midi2_converter_handle_t handle) {
    if (!handle) return 0;
    bytestreamToUMP* converter = static_cast<bytestreamToUMP*>(handle);
    return converter->readUMP();
}

/* UMP to MIDI1 Protocol converter implementation */
midi2_converter_handle_t midi2_ump_to_midi1_create(void) {
    return new umpToBytestream();
}

void midi2_ump_to_midi1_destroy(midi2_converter_handle_t handle) {
    if (handle) {
        delete static_cast<umpToBytestream*>(handle);
    }
}

void midi2_ump_to_midi1_process(midi2_converter_handle_t handle, uint32_t ump_word) {
    if (!handle) return;
    umpToBytestream* converter = static_cast<umpToBytestream*>(handle);
    converter->UMPStreamParse(ump_word);
}

bool midi2_ump_to_midi1_available(midi2_converter_handle_t handle) {
    if (!handle) return false;
    umpToBytestream* converter = static_cast<umpToBytestream*>(handle);
    return converter->availableBS();
}

uint8_t midi2_ump_to_midi1_read(midi2_converter_handle_t handle) {
    if (!handle) return 0;
    umpToBytestream* converter = static_cast<umpToBytestream*>(handle);
    return converter->readBS();
}

/* UMP to MIDI2 Protocol converter implementation */
midi2_converter_handle_t midi2_ump_to_midi2_create(void) {
    return new umpToMIDI2Protocol();
}

void midi2_ump_to_midi2_destroy(midi2_converter_handle_t handle) {
    if (handle) {
        delete static_cast<umpToMIDI2Protocol*>(handle);
    }
}

void midi2_ump_to_midi2_process(midi2_converter_handle_t handle, uint32_t ump_word) {
    if (!handle) return;
    umpToMIDI2Protocol* converter = static_cast<umpToMIDI2Protocol*>(handle);
    converter->UMPStreamParse(ump_word);
}

bool midi2_ump_to_midi2_available(midi2_converter_handle_t handle) {
    if (!handle) return false;
    umpToMIDI2Protocol* converter = static_cast<umpToMIDI2Protocol*>(handle);
    return converter->availableUMP();
}

uint32_t midi2_ump_to_midi2_read(midi2_converter_handle_t handle) {
    if (!handle) return 0;
    umpToMIDI2Protocol* converter = static_cast<umpToMIDI2Protocol*>(handle);
    return converter->readUMP();
}

} // extern "C"
