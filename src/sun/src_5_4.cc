

#include <array>
#include <cstdint>

#include "FreeRTOS.h"
#include "hardware/pio.h"
#include "hardware/timer.h"
#include "pico/platform.h"
#include "projdefs.h"
#include "sun/cd74hc595.h"
#include "task.h"

using Frame = std::array<uint8_t, 8>;

constexpr int kFrameMs = 500;
constexpr int kLineMs = 1;
constexpr int kSlowdown = 1;  // Slow down the whole program by a factor of
                              // 500 so we can see what's going on.

static uint32_t TicksDiff(uint32_t start, uint32_t end) {
  if (end >= start) {
    [[likely]] return end - start;
  }
  return UINT32_MAX - start + end;
}

static void ShowFrame(const Frame& f, jpico::Cd74Hc595DriverPio& driver) {
  uint32_t start_time = time_us_32();
  do {
    for (int i = 0; i < 8; i++) {
      // The first byte describes what columns to drive. A 1 here means
      // *don't* show this column, because we've driven it high! We want
      // to *sink* the selected columns. So invert the frame byte.
      const uint32_t column_select = ~f[i] & 0xff;
      // The second byte written describes what values to drive to row sources.
      // A 1 here means, "Light up diodes on this row."
      const uint32_t row_select = 1 << i;
      const uint32_t message = (column_select << 8) | row_select;
      driver.Send(message);
      vTaskDelay(pdMS_TO_TICKS(kLineMs * kSlowdown));
    }
  } while (TicksDiff(start_time, time_us_32()) < (kFrameMs * 1000 * kSlowdown));
}

extern "C" void main_task(void*) {
  auto driver = jpico::Cd74Hc595DriverPio::Create(
      {.pio = pio0, .pin_srclk = 16, .pin_ser = 18, .output_bits = 16});
  if (!driver) {
    panic("unable to create driver\n");
  }
  auto frames = std::to_array<Frame>({Frame{
                                          0b11100111,
                                          0b11100111,
                                          0b11100111,
                                          0b00000000,
                                          0b00101000,
                                          0b01111100,
                                          0b00111000,
                                          0b00010000,
                                      },
                                      {
                                          0b11100111,
                                          0b11111111,
                                          0b11100111,
                                          0b00000000,
                                          0b00101000,
                                          0b01111100,
                                          0b00111000,
                                          0b00010000,
                                      },
                                      {
                                          0b11100111,
                                          0b11111111,
                                          0b11100111,
                                          0b00000000,
                                          0b00111000,
                                          0b00111000,
                                          0b00111000,
                                          0b00000000,
                                      },
                                      {
                                          0b00000000,
                                          0b00000000,
                                          0b00000000,
                                          0b01111100,
                                          0b01111100,
                                          0b01111100,
                                          0b01111100,
                                          0b01111100,
                                      },
                                      {
                                          0b00000000,
                                          0b00000000,
                                          0b11111110,
                                          0b11111110,
                                          0b11111110,
                                          0b11111110,
                                          0b11111110,
                                          0b11111110,
                                      },
                                      {
                                          0b00000000,
                                          0b11111111,
                                          0b11111111,
                                          0b11111111,
                                          0b11111111,
                                          0b11111111,
                                          0b11111111,
                                          0b11111111,
                                      },
                                      {
                                          0b11111111,
                                          0b11111111,
                                          0b11111111,
                                          0b11111111,
                                          0b11111111,
                                          0b11111111,
                                          0b11111111,
                                          0b11111111,
                                      }});
  while (true) {
    // vTaskDelay(pdMS_TO_TICKS(500));
    for (const auto& frame : frames) {
      ShowFrame(frame, *driver);
    }
  }
}