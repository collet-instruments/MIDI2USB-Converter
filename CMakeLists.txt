cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Optimization flags for different build types
# For STM32F401 (Cortex-M4F), we optimize for performance while maintaining reasonable code size
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    # -O2: Good balance between speed and size for embedded systems
    # -ffunction-sections -fdata-sections: Allow linker to remove unused code
    set(CMAKE_C_FLAGS_RELEASE "-O2 -ffunction-sections -fdata-sections -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "-O2 -ffunction-sections -fdata-sections -DNDEBUG")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-Wl,--gc-sections")
elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    # -Os: Optimize for size (important for flash-constrained devices)
    set(CMAKE_C_FLAGS_MINSIZEREL "-Os -flto -ffunction-sections -fdata-sections -DNDEBUG")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -flto -ffunction-sections -fdata-sections -DNDEBUG")
    set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "-Wl,--gc-sections -flto")
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    # -O2 with debug info: For debugging optimized code
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g -ffunction-sections -fdata-sections -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -ffunction-sections -fdata-sections -DNDEBUG")
    set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "-Wl,--gc-sections")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME MIDI2USB-Converter)

# Include toolchain file
include("cmake/gcc-arm-none-eabi.cmake")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Add FreeRTOS submodule
# Create FreeRTOS config target
add_library(freertos_config INTERFACE)
target_include_directories(freertos_config SYSTEM INTERFACE Core/Inc)
target_compile_definitions(freertos_config INTERFACE projCOVERAGE_TEST=0)

# Set FreeRTOS port and heap
set(FREERTOS_PORT GCC_ARM_CM4F)
set(FREERTOS_HEAP 4)
add_subdirectory(submodules/FreeRTOS-Kernel EXCLUDE_FROM_ALL)

# Add TinyUSB submodule
set(FAMILY stm32f4)
set(BOARD stm32f401_blackpill)

# Add TinyUSB sources (use original usbd.c with Application Driver support)
file(GLOB_RECURSE TINYUSB_SOURCES 
    "submodules/tinyusb/src/tusb.c"
    "submodules/tinyusb/src/common/*.c"
    "submodules/tinyusb/src/class/midi/*.c"
    "submodules/tinyusb/src/portable/synopsys/dwc2/*.c"
    "submodules/tinyusb/src/device/*.c"
)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

file(GLOB_RECURSE DRIVER_SRC_FILES "submodules/tinyusb/src/*.c")

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    Core/Src/usb_descriptors.c
    Core/Src/usb_callbacks.c
    Core/Src/led_task.c
    Core/Src/usb_device_task.c
    Core/Src/usb_midi_task.c
    Core/Src/uart_midi_task.c
    Core/Src/midi_common.c
    Core/Src/mode_manager.c
    Core/Src/midi2_task.c
    Core/Src/ump_task.c
    Core/Src/ump_discovery.c
    Core/Src/midi2_wrapper.cpp
    Core/Src/usbd_app_driver.c
    ${TINYUSB_SOURCES}
    submodules/tusb_ump/ump_device.cpp
    # AM MIDI 2.0 Library
    submodules/AM_MIDI2.0Lib/src/bytestreamToUMP.cpp
    submodules/AM_MIDI2.0Lib/src/umpToBytestream.cpp
    submodules/AM_MIDI2.0Lib/src/utils.cpp
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # FreeRTOS configuration
    Core/Inc
    # FreeRTOS port specific includes
    submodules/FreeRTOS-Kernel/portable/GCC/ARM_CM4F
    # TinyUSB includes
    submodules/tinyusb/src
    submodules/tinyusb/hw
    # MIDI 2.0 libraries
    submodules/AM_MIDI2.0Lib/include
    submodules/tusb_ump
    # Add user defined include paths
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Add linked libraries
# Note: Order matters - FreeRTOS must come after objects that use it
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx
    -Wl,--whole-archive
    freertos_kernel
    -Wl,--no-whole-archive
    # Add user defined libraries
)

# Apply strict warnings to our source files
set_source_files_properties(
    Core/Src/led_task.c
    Core/Src/mode_manager.c
    Core/Src/ump_discovery.c
    PROPERTIES COMPILE_FLAGS "${PROJECT_WARNING_FLAGS}"
)

# Apply relaxed warnings to files that include TinyUSB headers (to suppress enum warnings)
set_source_files_properties(
    Core/Src/usb_descriptors.c
    Core/Src/usb_device_task.c
    Core/Src/usb_midi_task.c
    Core/Src/uart_midi_task.c
    Core/Src/midi_common.c
    Core/Src/midi2_task.c
    Core/Src/ump_task.c
    Core/Src/ump_discovery.c
    Core/Src/midi2_wrapper.cpp
    Core/Src/midi_processor.c
    Core/Src/uart_midi_handler.c
    # AM MIDI 2.0 Library
    submodules/AM_MIDI2.0Lib/src/bytestreamToUMP.cpp
    submodules/AM_MIDI2.0Lib/src/umpToBytestream.cpp
    submodules/AM_MIDI2.0Lib/src/utils.cpp
    # TinyUSB UMP device
    submodules/tusb_ump/ump_device.cpp
    PROPERTIES COMPILE_FLAGS "${EXTERNAL_WARNING_FLAGS}"
)

# Apply relaxed warnings to external libraries
foreach(TINYUSB_FILE ${TINYUSB_SOURCES})
    set_source_files_properties(${TINYUSB_FILE} PROPERTIES COMPILE_FLAGS "${EXTERNAL_WARNING_FLAGS}")
endforeach()

# Apply relaxed warnings to STM32 HAL drivers to suppress unused parameter warnings
set_property(TARGET STM32_Drivers PROPERTY COMPILE_FLAGS "${EXTERNAL_WARNING_FLAGS} -Wno-unused-parameter")
