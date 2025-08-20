/**
  * @file           : midi2_wrapper.h
  * @brief          : C wrapper for AM MIDI 2.0 Library
  */

#ifndef __MIDI2_WRAPPER_H__
#define __MIDI2_WRAPPER_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/
typedef void* midi2_converter_handle_t;

/* Exported functions prototypes ---------------------------------------------*/

// Bytestream to UMP converter
midi2_converter_handle_t midi2_bs_to_ump_create(void);
void midi2_bs_to_ump_destroy(midi2_converter_handle_t handle);
bool midi2_bs_to_ump_process_byte(midi2_converter_handle_t handle, uint8_t byte);
bool midi2_bs_to_ump_available(midi2_converter_handle_t handle);
uint32_t midi2_bs_to_ump_read(midi2_converter_handle_t handle);

// UMP to MIDI1 Protocol converter  
midi2_converter_handle_t midi2_ump_to_midi1_create(void);
void midi2_ump_to_midi1_destroy(midi2_converter_handle_t handle);
void midi2_ump_to_midi1_process(midi2_converter_handle_t handle, uint32_t ump_word);
bool midi2_ump_to_midi1_available(midi2_converter_handle_t handle);
uint8_t midi2_ump_to_midi1_read(midi2_converter_handle_t handle);

// UMP to MIDI2 Protocol converter
midi2_converter_handle_t midi2_ump_to_midi2_create(void);
void midi2_ump_to_midi2_destroy(midi2_converter_handle_t handle);
void midi2_ump_to_midi2_process(midi2_converter_handle_t handle, uint32_t ump_word);
bool midi2_ump_to_midi2_available(midi2_converter_handle_t handle);
uint32_t midi2_ump_to_midi2_read(midi2_converter_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif /* __MIDI2_WRAPPER_H__ */
