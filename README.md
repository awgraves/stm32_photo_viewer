# STM32 Photo Viewer
A picture-frame-like embedded display that renders photos from a micro SD card.

Powered by an STM32F446RE (32-bit Arm Cortex M4 processor).

Built 100% by hand from scratch.

- Register-level bare metal C programming with no OS or framework.
- NO 3rd party code of any kind, not even CMSIS header files.
  - Linker script, vector table, reset handler, MMIO, all by hand.
  - GPIO, SPI, SDIO, DMA drivers from the ground up.
  - Custom lightweight graphics library for the UI.
  - Hand-rolled SD card driver with FAT32 filesystem implementation.
- Custom PCB design.
- Homemade, 3D printed case.

## Schematics
![EasyEDA Board Schematics](./docs/schematics/stm32_photo_viewer_schematics.png)

