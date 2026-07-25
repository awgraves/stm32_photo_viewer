# STM32 Photo Viewer
A picture-frame-like embedded device that renders photos from a micro SD card.

![STM32 Photo Viewer Demo](./docs/photo_viewer_demo.gif)

Firmware written 100% by hand from scratch. ✍️
- ❌ no generated code
- ❌ no 3rd party frameworks or libraries
- ❌ no vendor hardware abstraction layer (HAL), not even CMSIS headers
- ✅ Linker script, vector table, reset handler, clock trees, all manual
- ✅ GPIO, SPI, SDIO, TIMER, DMA drivers from the ground up
- ✅ Custom lightweight graphics rendering for the GUI
- ✅ Hand-rolled SD card driver with a minimal FAT32 implementation

And for the physical device:
- ✅ Custom PCB design (with EasyEDA + JLCPCB as manufacturer)
- ✅ Homemade, 3D printed case (on a Bambu Lab A1 mini)

Watch the project build video on YouTube 👇

[![STM32 Photo Viewer - From Start to Finish](https://img.youtube.com/vi/W2JZO2xCuKY/maxresdefault.jpg)](https://www.youtube.com/watch?v=W2JZO2xCuKY)

## Features
- Handles up to ***5,000 photos*** at a time
- Manual photo browse mode (turning the knob switches between photos)
- Slideshow modes with 2, 5, or 10 second intervals
- Adjustable display brightness

## Hardware
- STM32F446RE (32-bit Arm Cortex M4 processor) MCU
- [3.5" TFT LCD display w/ ST7796 controller](https://www.amazon.com/dp/B0DFWL9M9B)
- [EC11 rotary encoder](https://www.amazon.com/dp/B0DZTX96NH)
- SD card slot (during initial prototyping, used the [Adafruit breakout module](https://www.adafruit.com/product/4682))

## Schematics
![EasyEDA Board Schematics](./docs/schematics/stm32_photo_viewer_schematics.png)
## Requirements
1. [GNU Arm Embedded Toolchain](https://developer.arm.com/downloads/-/gnu-rm)
2. [GNU Make](https://www.gnu.org/software/make/)
3. [Bear](https://github.com/rizsotto/Bear) (for clangd compile_commands generation)
4. [OpenOCD](https://openocd.org/pages/getting-openocd.html) (for ST-link connection)
5. [UV Python package & project manager](https://docs.astral.sh/uv/) (for photo conversion script)

## Loading Photos
A high capacity micro SD card is required for the firmware.
The card must be pre-formatted to contain a FAT32 filesystem located at the 1st partition.

The firmware expects a custom `.pic` binary file format. 

A photo conversion script is included under `tools/convert.py`.
See the tool [README](tools/README.md) for more details.


## Local Dev Setup
1. Run `make setup` to generate the clangd compile commands for your IDE.

## RAM Budgeting
Total RAM available on STM32F446RE: **128KB**

### Noteworthy Buffer Allocations
- Photo rendering: 64KB (50% of total RAM)
- File list: ~39KB (~30.5%), translates to max 5000 photos
- Graphics rendering: 10KB (~8%)

Leaves 15KB for stack with plenty of head room (~11%)
