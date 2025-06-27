#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "lib.h"


static void init_i2c_pin() {
  gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
  gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
}

static void init_pull_up_i2c() {
  gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
  gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
}

static inline int GetFontIndex(uint8_t ch) {
  if (ch >= 'A' && ch <='Z') {
    return  ch - 'A' + 1;
  }
  else if (ch >= '0' && ch <='9') {
    return  ch - '0' + 27;
  }
  else return  0; // Not got that char so space.
}

// Function to send a command to the display
static void send_command(uint8_t cmd) {
  // I2C write process expects a control byte followed by data
  // this "data" can be a command or data to follow up a command
  uint8_t buf[2] = {0x80, cmd};
  i2c_write_blocking(i2c_default, SSD1306_I2C_ADDR, buf, 2, false);
}

// Function to send a list of commands to the display
static void send_command_list(uint8_t *buf, int num) {
  for (int i = 0; i < num; i++) {
    send_command(buf[i]);
  }
}

// SSD1306 display initialization code
static void init_display(void) {
  stdio_init_all();
  i2c_init(i2c_default, SSD1306_I2C_CLK * 1000);
  init_i2c_pin();
  init_pull_up_i2c();
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
   send_command_list(cmds, count_of(cmds));
}

// Function to set up a render area for the display
static void setup_render_area(struct render_area *area, uint8_t start_col, uint8_t end_col, uint8_t start_page, uint8_t end_page) {
  area->start_col = start_col;
  area->end_col = end_col;
  area->start_page = start_page;
  area->end_page = end_page;
}

// Function to calculate the buffer length for a render area
static void calc_render_area_buflen(struct render_area *area) {
  // calculate how long the flattened buffer will be for a render area
  area->buflen = (area->end_col - area->start_col + 1) * (area->end_page - area->start_page + 1);
}

// Function to write a character to the display buffer
static void write_char(uint8_t *buf, int16_t x, int16_t y, uint8_t ch) {
  if (x > SSD1306_WIDTH - 8 || y > SSD1306_HEIGHT - 8)
    return;

  // For the moment, only write on Y row boundaries (every 8 vertical pixels)
  y = y/8;

  ch = toupper(ch);
  int idx = GetFontIndex(ch);
  int fb_idx = y * 128 + x;

  for (int i=0;i<8;i++) {
    buf[fb_idx++] = font[idx * 8 + i];
  }
}

// Function to write a string of characters to the display buffer
static void write_string(uint8_t *buf, int16_t x, int16_t y, char *str) {
  // Cull out any string off the screen
  if (x > SSD1306_WIDTH - 8 || y > SSD1306_HEIGHT - 8)
    return;

  while (*str) {
    write_char(buf, x, y, *str++);
    x+=8;
  }
}

// Function to render a render area with the given display buffer
static void render(uint8_t *buf, struct render_area *area) {
  // update a portion of the display with a render area
  uint8_t cmds[] = {
    SSD1306_SET_COL_ADDR,
    area->start_col,
    area->end_col,
    SSD1306_SET_PAGE_ADDR,
    area->start_page,
    area->end_page
  };

  uint8_t *temp_buf = malloc(area->buflen +1);
  send_command_list(cmds, count_of(cmds));
  temp_buf[0] = 0x40;
  memcpy(temp_buf+1, buf, area->buflen);
  i2c_write_blocking(i2c_default, SSD1306_I2C_ADDR, temp_buf, area->buflen + 1, false);
  free(temp_buf);
}

// Function to display text on the display
void display_text(int num_args, ...) {
  struct render_area frame_area = {
    start_col: 0,
    end_col : SSD1306_WIDTH - 1,
    start_page : 0,
    end_page : SSD1306_NUM_PAGES - 1
  };
  calc_render_area_buflen(&frame_area);

  // zero the entire display
  uint8_t buf[SSD1306_BUF_LEN];
  memset(buf, 0, SSD1306_BUF_LEN);
  render(buf, &frame_area);

  va_list args;
  va_start(args, num_args);
  int y = 0;
  for (int i = 0; i < num_args; ++i) {
    char *text = va_arg(args, char *);
    write_string(buf, 5, y, text);
    y+=8;
  }
  va_end(args);
  render(buf, &frame_area);
}

int main() {
  init_display(); // Initialize the display

  uint8_t *buffer = (uint8_t *)malloc(SSD1306_BUF_LEN);
  memset(buffer, 0, SSD1306_BUF_LEN);

  struct render_area frame_area;
  setup_render_area(&frame_area, 0, SSD1306_WIDTH - 1, 0, SSD1306_NUM_PAGES - 1);

  display_text(2, " HELLO ", " WORLD ");

  free(buffer);
  return 1;
}
