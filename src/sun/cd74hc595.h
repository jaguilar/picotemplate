#ifndef SUN_CD74HC595_H
#define SUN_CD74HC595_H

#include <cstdint>
#include <optional>

#include "hardware/pio.h"

namespace jpico {

class Cd74Hc595DriverPio {
 public:
  struct Config {
    PIO pio;
    uint32_t pin_srclk = 999;  // Note: pin_rclk must be pin_srclk + 1.
    uint32_t pin_ser = 999;
    uint32_t target_frequency = 1'000'000;
  };

  Cd74Hc595DriverPio(const Cd74Hc595DriverPio&) = delete;
  Cd74Hc595DriverPio(Cd74Hc595DriverPio&& o);
  Cd74Hc595DriverPio& operator=(const Cd74Hc595DriverPio&) = delete;

  ~Cd74Hc595DriverPio();

  // Constructs the driver from the Config.
  static std::optional<Cd74Hc595DriverPio> Create(const Config& config);

  inline void Send(uint8_t x) {
    pio_sm_put_blocking(pio_, state_machine_, static_cast<uint32_t>(x) << 24);
  }

  uint32_t state_machine() const { return state_machine_; }

 private:
  Cd74Hc595DriverPio(PIO pio, uint32_t state_machine)
      : pio_{pio}, state_machine_{state_machine} {}

  PIO pio_;
  uint32_t state_machine_;
};

}  // namespace jpico

#endif  // SUN_CD74HC595_H