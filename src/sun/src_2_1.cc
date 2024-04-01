
#include "FreeRTOS.h"
#include "hardware/gpio.h"
#include "shared_init.h"
#include "task.h"

extern "C" void main_task(void*) {
  gpio_init(15);
  gpio_set_dir(15, true);
  while (true) {
    gpio_put(15, false);
    vTaskDelay(pdMS_TO_TICKS(500));
    gpio_put(15, true);
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}