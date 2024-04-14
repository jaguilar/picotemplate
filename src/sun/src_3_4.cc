#include <cstdio>

#include "FreeRTOS.h"
#include "LiquidCrystal_I2C.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "portmacro.h"
#include "shared_init.h"
#include "task.h"

void main_task(void*) {
  auto* i2c = i2c1;
  gpio_set_function(26, GPIO_FUNC_I2C);
  gpio_set_function(27, GPIO_FUNC_I2C);
  printf("i2c baudrate %d\n", i2c_init(i2c, 100000));

  LiquidCrystal_I2C::Config config{
      .i2c = i2c, .addr = 0x27, .cols = 16, .rows = 2};
  LiquidCrystal_I2C lcd(config);
  lcd.init();

  lcd.backlight();
  vTaskDelay(pdMS_TO_TICKS(1000));
  lcd.noBacklight();
  vTaskDelay(pdMS_TO_TICKS(1000));
  lcd.backlight();
  lcd.setCursor(3, 0);
  lcd.print("Hello, world!");
  lcd.setCursor(2, 1);
  lcd.print("Ywrobot Pico!");
  vTaskDelay(portMAX_DELAY);
}