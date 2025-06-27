#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "lib.h"

// To extract beceause the init should be called only 1 times
void display_init() {
  uint8_t cmds[] = {
    SSD1306_SET_DISP,               // set display off
    /* memory mapping */
    SSD1306_SET_MEM_MODE,           // set memory address mode 0 = horizontal, 1 = vertical, 2 = page
    0x00,                           // horizontal addressing mode
    /* resolution and layout */
    SSD1306_SET_DISP_START_LINE,    // set display start line to 0
    SSD1306_SET_SEG_REMAP | 0x01,   // set segment re-map, column address 127 is mapped   SSD1306_SET_MUX_RATIO,          // set multiplex ratio
    SSD1306_HEIGHT - 1,             // Display height - 1
    SSD1306_SET_COM_OUT_DIR | 0x08, // set COM (common) output scan direction. Scan from bottom up, COM[N-1] to COM0 to SEG0
    SSD1306_SET_MUX_RATIO,          // set multiplex ratio
    SSD1306_HEIGHT - 1,             // Display height - 1
    SSD1306_SET_COM_OUT_DIR | 0x08, // set COM (common) output scan direction. Scan from bottom up, COM[N-1] to COM0
    SSD1306_SET_DISP_OFFSET,        // set display offset
    0x00,                           // no offset
    SSD1306_SET_COM_PIN_CFG,        // set COM (common) pins hardware configuration. Board specific magic number.
    // 0x02 Works for 128x32, 0x12 Possibly works for 128x64. Other options 0x22, 0x32
#if ((SSD1306_WIDTH == 128) && (SSD1306_HEIGHT == 32))
    0x02,
#elif ((SSD1306_WIDTH == 128) && (SSD1306_HEIGHT == 64))
    0x12,
#else
    0x02,
#endif
    /* timing and driving scheme */
    SSD1306_SET_DISP_CLK_DIV,       // set display clock divide ratio
    0x80,                           // div ratio of 1, standard freq
    SSD1306_SET_PRECHARGE,          // set pre-charge period
    0xF1,                           // Vcc internally generated on our board
    SSD1306_SET_VCOM_DESEL,         // set VCOMH deselect level
    0x30,                           // 0.83xVcc
    /* display */
    SSD1306_SET_CONTRAST,           // set contrast control
    0xFF,
    SSD1306_SET_ENTIRE_ON,          // set entire display on to follow RAM content
    SSD1306_SET_NORM_DISP,           // set normal (not inverted) display
    SSD1306_SET_CHARGE_PUMP,        // set charge pump
    0x14,                           // Vcc internally generated on our board
    SSD1306_SET_SCROLL | 0x00,      // deactivate horizontal scrolling if set. This is necessary as memory writes will corrupt if scrolling was enabled
    SSD1306_SET_DISP | 0x01, // turn display on
  };
  SSD1306_send_cmd_list(cmds, count_of(cmds));
}

static void init_i2c_pin() {
  gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
  gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
}

static void init_pull_up_i2c() {
  gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
  gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
}

int main() {
  stdio_init_all();
[O
  i2c_init(i2c_default, SSD1306_I2C_CLK * 1000);
  init_i2c_pin();
  init_pull_up_i2c();
  return 1;
}
