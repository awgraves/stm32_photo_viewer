# STM32 Photo Viewer
A picture-frame-like embedded display that renders photos from a micro SD card.

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
- ✅ Homemade, 3D printed case (on a Bambu Labs A1 mini)

## Hardware
- STM32F446RE (32-bit Arm Cortex M4 processor) MCU
- 3.5" TFT LCD display w/ ST7796 controller
- EC11 rotary encoder
- SD card slot

## Schematics
![EasyEDA Board Schematics](./docs/schematics/stm32_photo_viewer_schematics.png)
## Requirements
1. [GNU Arm Embedded Toolchain](https://developer.arm.com/downloads/-/gnu-rm)
2. [GNU Make](https://www.gnu.org/software/make/)
3. [Bear](https://github.com/rizsotto/Bear) (for clangd compile_commands generation)

## Local Dev Setup
1. Run `make setup` to generate the clangd compile commands for your IDE.
