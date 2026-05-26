#include "ili9341.h"
#include "mcu/time.h"

/* Commands begin on ili9341 datasheet pg. 83 */
#define CMD_NOOP 0x00
#define CMD_SOFTWARE_RESET 0x01
#define CMD_SLEEP_OUT 0x11
#define CMD_DISPLAY_OFF 0x28
#define CMD_DISPLAY_ON 0x29

#define CMD_PIXEL_FORMAT_SET 0x3A
#define PARAM_16_BIT_COLOR 0x55

#define CMD_COLUMN_ADDR_SET 0x2A
#define CMD_PAGE_ADDR_SET 0x2B

/* Screen orientation & physical display RGB vs BGR */
#define CMD_MEM_ACCESS_CTL 0x36
#define MADCTL_MY (1U << 7)
#define MADCTL_MX (1U << 6)
#define MADCTL_MV (1U << 5)
#define MADCTL_BGR (1U << 3)
#define PARAMS_MADCTL ((MADCTL_MV | MADCTL_BGR) & 0xFF)

#define CMD_MEM_WRITE 0x2C

static ili9341_config_t io;

static inline void begin_tx(void) { gpio_clear_pin(io.cs); }
static inline void end_tx(void) { gpio_set_pin(io.cs); }

static void write_cmd(uint8_t cmd);
static void write_data(uint8_t data);
static void write_data_stream(const uint8_t stream[], uint16_t len);

static void set_addr_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
static void hard_reset(void);

/*
  public functions
*/

void ili9341_init(ili9341_config_t *c) {
  io = *c;

  gpio_set_mode(io.cs, GPIO_MODE_OUTPUT);
  gpio_set_mode(io.dc, GPIO_MODE_OUTPUT);
  gpio_set_mode(io.rst, GPIO_MODE_OUTPUT);

  // ensure off before hard reset
  end_tx();
  hard_reset();

  begin_tx();

  write_cmd(CMD_SOFTWARE_RESET);
  delay_ms(120); // pg. 89
  write_cmd(CMD_SLEEP_OUT);
  delay_ms(5); // pg. 101

  write_cmd(CMD_PIXEL_FORMAT_SET);
  write_data(PARAM_16_BIT_COLOR);

  write_cmd(CMD_MEM_ACCESS_CTL);
  write_data(PARAMS_MADCTL);

  write_cmd(CMD_DISPLAY_ON);
  end_tx();
}

void ili9341_fill(uint16_t color) {
  begin_tx();

  set_addr_window(0, 0, ILI9341_WIDTH - 1, ILI9341_HEIGHT - 1);

  uint8_t linebuf[ILI9341_WIDTH * 2];
  for (int x = 0; x < ILI9341_WIDTH; x++) {
    linebuf[x * 2] = color >> 8;
    linebuf[x * 2 + 1] = color & 0xFF;
  }

  write_cmd(CMD_MEM_WRITE);
  for (int y = 0; y < ILI9341_HEIGHT; y++) {
    write_data_stream(linebuf, sizeof(linebuf));
  }

  write_cmd(CMD_NOOP);
  end_tx();
}

/*
  helpers
*/

static void hard_reset(void) {
  // pg.213-214
  gpio_digital_write(io.rst, LOW);
  delay_ms(10);
  gpio_digital_write(io.rst, HIGH);
  delay_ms(120);
}

static void write_cmd(uint8_t cmd) {
  gpio_clear_pin(io.dc);
  spi_tx(io.spi, &cmd, 1);
}

static void write_data(uint8_t data) {
  gpio_set_pin(io.dc);
  spi_tx(io.spi, &data, 1);
}

static void write_data_stream(const uint8_t stream[], uint16_t len) {
  gpio_set_pin(io.dc);
  spi_tx(io.spi, stream, len);
}

static void set_addr_window(uint16_t x0, uint16_t y0, uint16_t x1,
                            uint16_t y1) {

  write_cmd(CMD_COLUMN_ADDR_SET);
  write_data(x0 >> 8);
  write_data(x0 & 0xFF);
  write_data(x1 >> 8);
  write_data(x1 & 0xFF);

  write_cmd(CMD_PAGE_ADDR_SET);
  write_data(y0 >> 8);
  write_data(y0 & 0xFF);
  write_data(y1 >> 8);
  write_data(y1 & 0xFF);
}
