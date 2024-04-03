#include <stdint.h>
#include <sys/types.h>

#include <cstdio>
#include <expected>
#include <optional>

#include "FreeRTOS.h"
#include "cd74hc595.pio.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hardware/structs/clocks.h"
#include "pico/platform.h"
#include "task.h"

class Cd74Hc595DriverPio {
 public:
  struct Config {
    uint32_t pin_srclk = 999;  // Note: pin_rclk must be pin_srclk + 1.
    uint32_t pin_ser = 999;
    PIO pio;
    uint32_t state_machine;
  };

  Cd74Hc595DriverPio(const Cd74Hc595DriverPio&) = delete;
  Cd74Hc595DriverPio(Cd74Hc595DriverPio&& o)
      : pio_(o.pio_), state_machine_(o.state_machine_) {
    o.pio_ = nullptr;
    o.state_machine_ = 0;
  }
  Cd74Hc595DriverPio& operator=(const Cd74Hc595DriverPio&) = delete;

  ~Cd74Hc595DriverPio() {
    if (!pio_) return;
    pio_sm_set_enabled(pio_, state_machine_, false);
    pio_sm_unclaim(pio_, state_machine_);
  }

  static std::optional<Cd74Hc595DriverPio> Create(const Config& config) {
    for (uint32_t pin :
         {config.pin_srclk, config.pin_srclk + 1, config.pin_ser}) {
      pio_gpio_init(config.pio, pin);
    }
    pio_sm_claim(config.pio, config.state_machine);
    if (!pio_can_add_program(config.pio, &cd74hc595_program)) {
      panic("can't add cd74hc595 to PIO\n");
    }
    uint32_t offset = pio_add_program(config.pio, &cd74hc595_program);
    pio_sm_set_consecutive_pindirs(config.pio, config.state_machine,
                                   config.pin_srclk, 2, true);
    pio_sm_set_consecutive_pindirs(config.pio, config.state_machine,
                                   config.pin_ser, 1, true);
    pio_sm_config sm_config = cd74hc595_program_get_default_config(offset);

    // Set the divider to achieve a target frequency. For testing, this is low.
    // At 2 volts, the maximum required pulse durations are 120ns. At 4.5 volts,
    // it's 25ns. It seems like there is a rapid decay in max pulse duration
    // required as we go up in voltage, so at 3.3 volts, it should be about
    // 30-50ns. That gives us 10-20MHz as a workable target frequency.
    constexpr uint32_t kTargetFrequency = 1000000;
    sm_config_set_clkdiv(
        &sm_config,
        static_cast<double>(clock_get_hz(clk_sys)) / kTargetFrequency);

    sm_config_set_out_pins(&sm_config, config.pin_ser, 1);
    sm_config_set_sideset_pins(&sm_config, config.pin_srclk);

    // MSB should be output first, so we're shifting left. Note that this means
    // that when we push, we need to shift left by 24.
    sm_config_set_out_shift(&sm_config, false, true, 8);
    pio_sm_init(config.pio, config.state_machine, offset, &sm_config);
    pio_sm_set_enabled(config.pio, config.state_machine, true);
    return Cd74Hc595DriverPio(config.pio, config.state_machine);
  }

  void Send(uint8_t x) {
    pio_sm_put_blocking(pio_, state_machine_, static_cast<uint32_t>(x) << 24);
  }

 private:
  Cd74Hc595DriverPio(PIO pio, uint32_t state_machine)
      : pio_{pio}, state_machine_{state_machine} {}

  PIO pio_;
  uint32_t state_machine_;
};

extern "C" {
void SysprogsDebugger_SavePIOOutputs(PIO pio);
void SysprogsDebugger_RestorePIOOutputs(PIO pio);
int SysprogsDebugger_StepPIOs(PIO pio, int mask, int stepCount,
                              int overrideMask);
int SysprogsDebugger_SavePIOStateToBuffer(PIO pio, int mask, int flags,
                                          uint32_t inputValues);
}

static uint ReverseBits(uint value) {
  uint result = 0;
  for (int i = 0, j = 31; i < 32; i++, j--) result |= ((value >> i) & 1) << j;

  return result;
}

static unsigned SysprogsDebugger_ReadPIORegisterDestroyingPinState(
    PIO pio, unsigned sm, pio_src_dest reg) {
  pio_sm_exec_wait_blocking(pio, sm, pio_encode_mov(pio_pins, reg));
  unsigned normalValues = pio->dbg_padout;
  pio_sm_exec_wait_blocking(pio, sm, pio_encode_mov_reverse(pio_pins, reg));
  unsigned reversedValues = pio->dbg_padout;

  uint reconstructedFromReversing = ReverseBits(reversedValues);

  uint intersectionMask =
      0x3FFFFFFC;  // These bits should be accessible by both normal and reverse
                   // read operations
  if (((normalValues ^ reconstructedFromReversing) & intersectionMask) == 0)
    return normalValues |
           reconstructedFromReversing;  // Normal value has 2 MSBs at 0,
                                        // reversed one has 2 LSBs at 0. ORing
                                        // both values gets the actual register
                                        // value.

  return 0;
}

static unsigned SysprogsDebugger_ReadPIORegister(PIO pio, unsigned sm,
                                                 pio_src_dest reg) {
  uint32_t outputValues = pio->dbg_padout;
  SysprogsDebugger_SavePIOOutputs(pio);
  unsigned result =
      SysprogsDebugger_ReadPIORegisterDestroyingPinState(pio, sm, reg);
  pio_sm_set_pins(pio, sm, outputValues);
  SysprogsDebugger_RestorePIOOutputs(pio);
  return result;
}

static void StepPios(unsigned int u) {
  (void)u;
  SysprogsDebugger_StepPIOs(pio0, 0x1, 1, 0);
  SysprogsDebugger_ReadPIORegister(pio0, 0, pio_pins);
}

extern "C" void main_task(void*) {
  Cd74Hc595DriverPio::Config config;
  config.pin_srclk = 13;
  config.pin_ser = 15;
  config.pio = pio0;
  config.state_machine = 0;
  auto driver = Cd74Hc595DriverPio::Create(config);

  while (true) {
    for (uint16_t x = 0; x < 256; ++x) {
      driver->Send(x);
      vTaskDelay(pdMS_TO_TICKS(50));
    }
  }
}
