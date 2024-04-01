
#include <array>

#include "FreeRTOS.h"
#include "hardware/gpio.h"
#include "projdefs.h"
#include "shared_init.h"
#include "task.h"

extern "C" void main_task(void*) {
  constexpr auto kGpios = std::to_array({6, 7, 8, 9, 10, 11, 12, 13, 14, 15});
  for (auto gpio : kGpios) {
    gpio_init(gpio);
    gpio_set_dir(gpio, true);
  }

  while (true) {
    for (int i = 0; i < 10; ++i) {
      gpio_put(kGpios[i], true);
      vTaskDelay(pdMS_TO_TICKS(100));
    }
    for (int i = 0; i < 10; ++i) {
      gpio_put(kGpios[kGpios.size() - i - 1], false);
      vTaskDelay(pdMS_TO_TICKS(100));
    }
  }
}