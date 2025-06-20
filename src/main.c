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

int main() {
  stdio_init_all();
  i2c_init(i2c_default, SSD1306_I2C_CLK * 1000);
  init_i2c_pin();
  init_pull_up_i2c();
  return 1;
}
