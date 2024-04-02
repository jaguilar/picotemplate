
#include <cstdint>
#include <initializer_list>

#include "FreeRTOS.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/structs/clocks.h"
#include "projdefs.h"
#include "shared_init.h"
#include "task.h"

struct PwmInfo {
  uint pin;
  uint slice;
  uint chan;
};
PwmInfo GetPwm(uint32_t pin) {
  return {pin, pwm_gpio_to_slice_num(pin), pwm_gpio_to_channel(pin)};
}
void SetPwm(const PwmInfo& pwm, uint32_t level) {
  pwm_set_chan_level(pwm.slice, pwm.chan, level);
}

extern "C" void main_task(void*) {
  const auto kR = GetPwm(13);
  const auto kG = GetPwm(14);
  const auto kB = GetPwm(15);

  for (const auto pwm : {kR, kG, kB}) {
    gpio_init(pwm.pin);
    gpio_set_function(pwm.pin, GPIO_FUNC_PWM);
    pwm_set_wrap(pwm.slice, 256);
    pwm_set_chan_level(pwm.slice, pwm.chan, 0);
    pwm_set_enabled(pwm.slice, true);
  }

  const auto colors = {0xd6fff6, 0x231651, 0x4dccbd, 0x2374ab, 0xff8484};

  while (true) {
    for (const auto color : colors) {
      const auto r = (color >> 16) & 0xff;
      const auto g = (color >> 8) & 0xff;
      const auto b = color & 0xff;

      SetPwm(kR, r);
      SetPwm(kG, g);
      SetPwm(kB, b);
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }
}