#include "display.h"
#include "mcu/sysclock.h"
#include "time.h"
#include <stdbool.h>

/* Commands begin on display datasheet pg. 83 */
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

#define PARAMS_MADCTL ((MADCTL_BGR | MADCTL_MY))

#define CMD_MEM_WRITE 0x2C

#define FALLBACK_BRIGHTNESS 50

static display_config_t io;

void begin_tx(void) { gpio_clear_pin(io.cs); }
void end_tx(void) { gpio_set_pin(io.cs); }

static void write_cmd(uint8_t cmd);
static void write_param(uint8_t data);

static void hard_reset(void);
static inline bool valid_brightness(uint8_t val);

/*
  Public API
*/

void display_init(display_config_t *c) {
  io = *c;

  if (!valid_brightness(io.brightness_val)) {
    io.brightness_val = FALLBACK_BRIGHTNESS;
  }

  gpio_set_mode(io.cs, GPIO_MODE_OUTPUT);
  gpio_set_mode(io.dc, GPIO_MODE_OUTPUT);
  gpio_set_mode(io.rst, GPIO_MODE_OUTPUT);

  // gpio_set_mode(io.bl, GPIO_MODE_OUTPUT);
  gpio_set_mode(io.bl, GPIO_MODE_AF);
  gpio_set_AF(io.bl, c->bl_af);

  // ensure off before hard reset
  end_tx();
  hard_reset();

  begin_tx();

  write_cmd(CMD_SOFTWARE_RESET);
  delay_ms(120); // pg. 89
  write_cmd(CMD_SLEEP_OUT);
  delay_ms(5); // pg. 101

  write_cmd(CMD_PIXEL_FORMAT_SET);
  write_param(PARAM_16_BIT_COLOR);

  write_cmd(CMD_MEM_ACCESS_CTL);
  write_param(PARAMS_MADCTL);

  write_cmd(CMD_DISPLAY_ON);
  end_tx();

  // turn on backlight
  // targeting about 10khz PWM freq, just arbitrarily not too high/low
  // freq = timer_clock / ((PSC+1) * (ARR + 1))
  uint16_t psc_divisor = sysclock_get_cpu_hz() / (10000 * 100);
  timer_pwm_config_t pwm_conf = {.psc_val = psc_divisor - 1,
                                 .arr_val = 100 - 1,
                                 .ccr_val = io.brightness_val};
  timer_init_in_pwm_mode(io.timer, &pwm_conf);
  // gpio_digital_write(io.bl, HIGH);
}

void display_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
  begin_tx();

  write_cmd(CMD_COLUMN_ADDR_SET);
  write_param(x0 >> 8);
  write_param(x0 & 0xFF);
  write_param(x1 >> 8);
  write_param(x1 & 0xFF);

  write_cmd(CMD_PAGE_ADDR_SET);
  write_param(y0 >> 8);
  write_param(y0 & 0xFF);
  write_param(y1 >> 8);
  write_param(y1 & 0xFF);

  end_tx();
}

void display_pixel_stream_begin(void) {
  begin_tx();
  write_cmd(CMD_MEM_WRITE);
  gpio_set_pin(io.dc);
}

void display_pixel_stream_end(void) {
  write_cmd(CMD_NOOP);
  end_tx();
}

void display_pixel_stream_write(const uint16_t *pixels, uint32_t count) {
  // SPI must operate in 8-bit mode although pixels must be 16bits.
  spi_tx(io.spi, (uint8_t *)pixels, count * 2);
}

uint8_t display_get_brightness(void) { return io.brightness_val; }

void display_set_brightness(uint8_t val) {
  if (!valid_brightness(val)) {
    return;
  }

  // TODO: update the val should update the timer

  io.brightness_val = val;
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

static void write_param(uint8_t data) {
  gpio_set_pin(io.dc);
  spi_tx(io.spi, &data, 1);
}

static inline bool valid_brightness(uint8_t val) { return val <= 100; }
