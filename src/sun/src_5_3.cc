#include <cstdint>

#include "cd74hc595.h"
#include "disp_4digit.h"
#include "projdefs.h"

extern "C" void main_task(void*) {
  jpico::Cd74Hc595DriverPio::Config cd74_config;
  cd74_config.pin_srclk = 17;
  cd74_config.pin_ser = 16;
  cd74_config.pio = pio0;
  auto digit_driver = jpico::Cd74Hc595DriverPio::Create(cd74_config);

  jpico::Disp4Digit::Config disp_config{
      .digit_driver = *std::move(digit_driver),
      .pin_select = 12,
  };
  auto disp = jpico::Disp4Digit(std::move(disp_config));

  while (true) {
    for (int i = 0; i <= 9999; ++i) {
      disp.Set(
          {
              static_cast<uint8_t>((i / 1000) % 10),
              static_cast<uint8_t>((i / 100) % 10),
              static_cast<uint8_t>((i / 10) % 10),
              static_cast<uint8_t>(i % 10),
          },
          i % 6);
      vTaskDelay(pdMS_TO_TICKS(250));
    }
  }
}