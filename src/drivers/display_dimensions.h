#pragma once

#if !defined(DISPLAY_DRIVER_ILI9341) && !defined(DISPLAY_DRIVER_ST7796)
#error "Please set DISPLAY_DRIVER var in the makefile"
#endif

/*
  ILI9341 and ST7796 LCD drivers both share essentially the same commands.
  Just need to swap between pixel height/width values and MADCTL values.
*/

#if defined(DISPLAY_DRIVER_ILI9341)
#define DISPLAY_HEIGHT_PIXELS (uint16_t)240
#define DISPLAY_WIDTH_PIXELS (uint16_t)320
#elif defined(DISPLAY_DRIVER_ST7796)
#define DISPLAY_HEIGHT_PIXELS (uint16_t)480
#define DISPLAY_WIDTH_PIXELS (uint16_t)320
#endif
