# Notes from the docs

## Waveshare documentation
https://www.waveshare.com/wiki/2.4inch_LCD_Module#Introduction

### Hardware Description
Supports 12, 16, 18 bit input colors
aka RGB444, RGB565, RGB666
uses four-wire SPI

### Communication Protocol
Pins:
- No MISO, because display is read only
- RESX: reset, low when powering up, high at other times
- CS: chip select, low = enabled
- DC: 0 = command 1 = data

Clock polarity = 0 (idle low)
Clock phase = 0 (first clock edge)

## ILI9341
https://files.waveshare.com/upload/e/e3/ILI9341_DS.pdf

### Serial Interface
pg. 33, 4-line/8-bit setup

1. D/CX: data/command selection
2. CSX: chip enable input
3. SCL: serial clock
4. SDA/SDI/SDO: serial data i/o (DIN in this case)

SCL can be stopped when no comm necessary

MSB is transmitted first

pg. 63 shows RGB565 on 4 wire setup
RGB565 gives 65k colors with 16-bits
RGB666 gives 262K colors with 18-bits

### Command list
begins pg. 83
- nop: (0x00) can be used to terminate frame memory write / read pg. 89
- softare reset: (0x01) pg. 90
- sleep OUT: (0x11) pg. 101
  - on initial power on, will be in sleep IN mode
  - requires 120ms wait time
- display off: (0x28) pg. 108
- display on: (0x29) pg. 109
- memory write: (0x2C) pg. 114
- color set: (0x2D), not sure if needed? pg. 115
- write display brightness: (0x51) pg. 141
- Pixel Format Set: (0x3A) pg. 134
  - follow with a paramater (data mode!) 01010101 ie 0x55 for 16 bit pixels
- RGB interface control: (0x0H) pg. 154
  - pg. 45 describes more

### Memory read/write
Diagram data stream is from left to right, top left is 0,0 pg. 207

### Reset Sequence
pg. 213, should plan for case 1 - RST pin high/unstable on power on
sleep out (aka come out of sleep mode) takes 120ms

pg. 217, power on starts in sleep IN mode

### Timing diagram
pg. 237
Clock cycle min duration 40ns (aka max clock speed is 25MHz)

Initialization:
1. hardware reset (with RST pin held low, then high)
2. software reset with 0x01 command
3. set SLEEP to out
4. set pixel format
5. display on
