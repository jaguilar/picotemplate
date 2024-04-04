#include <stdint.h>
#include <sys/types.h>

#include <cstdio>
#include <expected>
#include <optional>

#include "FreeRTOS.h"
#include "cd74hc595.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hardware/structs/clocks.h"
#include "pico/platform.h"
#include "task.h"

using jpico::Cd74Hc595DriverPio;

extern "C" void main_task(void*) {
  Cd74Hc595DriverPio::Config config;
  config.pin_srclk = 17;
  config.pin_ser = 16;
  config.pio = pio0;
  auto driver = Cd74Hc595DriverPio::Create(config);

  while (true) {
    for (uint16_t x = 0; x < 256; ++x) {
      driver->Send(x);
      vTaskDelay(pdMS_TO_TICKS(50));
    }
  }
}
