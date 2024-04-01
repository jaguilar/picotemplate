
#include "FreeRTOS.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/structs/clocks.h"
#include "projdefs.h"
#include "shared_init.h"
#include "task.h"


extern "C" void main_task(void*) {
  gpio_init(15);
  gpio_set_function(15, GPIO_FUNC_PWM);
  const auto slice = pwm_gpio_to_slice_num(15);
  const auto chan = pwm_gpio_to_channel(15);

  pwm_set_wrap(slice, 100);
  pwm_set_chan_level(slice, chan, 0);
  pwm_set_enabled(slice, true);

  while (true) {
    for (int i = 0; i < 100; ++i) {
      // Wait approximately 10ms before increasing by 1%.
      pwm_set_chan_level(slice, chan, i);
      vTaskDelay(pdMS_TO_TICKS(10));
    }
    for (int i = 0; i < 100; ++i) {
      // Wait approximately 10ms before decreasing by 1%.
      pwm_set_chan_level(slice, chan, 99 - i);
      vTaskDelay(pdMS_TO_TICKS(10));
    }
  }
}