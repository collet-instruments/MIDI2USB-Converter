#!/bin/bash

# Flash script for STM32F401 using OpenOCD and ST-Link

echo "Building firmware..."
/opt/homebrew/bin/ninja -C build/Debug

if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo "Flashing firmware to STM32F401..."
/opt/homebrew/bin/openocd \
    -f interface/stlink.cfg \
    -f target/stm32f4x.cfg \
    -c "program build/Debug/MIDI2USB_Converter.elf verify reset exit"

if [ $? -eq 0 ]; then
    echo "Flash successful!"
else
    echo "Flash failed! Check connections:"
    echo "  - ST-Link connected to USB"
    echo "  - SWDIO connected to PA13"
    echo "  - SWCLK connected to PA14"
    echo "  - GND connected"
    echo "  - Target powered (3.3V)"
fi