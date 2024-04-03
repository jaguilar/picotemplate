#include <cassert>
#include <memory>
#include <optional>
#include <type_traits>

#include "FreeRTOS.h"
#include "hardware/gpio.h"
#include "portmacro.h"
#include "projdefs.h"
#include "queue.h"

namespace freertos {

template <typename T>
class Queue {
 public:
  static_assert(std::is_standard_layout_v<T> && std::is_trivial_v<T>,
                "Queue cannot contain non-POD types");

  Queue() = delete;
  Queue(int size) : queue_(xQueueCreate(size, sizeof(T))) {
    assert(size >= 1);
  }
  Queue(const Queue&) = delete;
  Queue& operator=(const Queue&) = delete;
  Queue& operator=(Queue&& q) = delete;
  Queue(Queue&& q) {
    queue_ = q.queue_;
    q.queue_ = nullptr;
  }
  ~Queue() { vQueueDelete(queue_); }

  // Pushes an item into the queue. Blocks for ticks_to_wait. Returns false
  bool Push(const T& t, TickType_t ticks_to_wait = portMAX_DELAY) {
    return xQueueSend(queue_, &t, ticks_to_wait) == pdTRUE;
  }
  struct ISRResult {
    bool pushed;
    BaseType_t should_context_switch;
  };
  ISRResult PushFromISR(const T& t, TickType_t ticks_to_wait = 0) {
    BaseType_t should_context_switch = pdFALSE;
    bool pushed =
        xQueueSendFromISR(queue_, &t, &should_context_switch) == pdTRUE;
    return {pushed, should_context_switch};
  }

  std::optional<T> Pop(TickType_t ticks_to_wait = portMAX_DELAY) {
    T t;
    if (xQueueReceive(queue_, &t, ticks_to_wait) == pdTRUE) {
      return t;
    };
    return std::nullopt;
  }

 private:
  QueueHandle_t queue_ = nullptr;
};
}  // namespace freertos

extern "C" void main_task(void*) {
  static freertos::Queue<bool>* const g_queue = new freertos::Queue<bool>(1);
  static constexpr auto kPin = 15;

  gpio_init(kPin);
  gpio_set_dir(kPin, false);
  gpio_set_pulls(kPin, false, true);
  gpio_set_irq_callback(+[](uint gpio, uint32_t events) {
    bool switch_task = false;
    if (gpio == kPin && (events & (GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL))) {
      const bool rise = events & GPIO_IRQ_EDGE_RISE;
      switch_task = g_queue->PushFromISR(rise).should_context_switch;
    }
    portYIELD_FROM_ISR(switch_task);
  });
  gpio_set_irq_enabled(kPin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);

  printf("enabled irqs\n");

  while (true) {
    const std::optional<bool> rising = g_queue->Pop(portMAX_DELAY);
    if (!rising) continue;

    printf("received interrupt %s\n", *rising ? "up" : "down");

    // We've seen an edge. Now we will de-bounce. Wait for 5ms for the signal to
    // stabilize.
    vTaskDelay(pdMS_TO_TICKS(10));
    // Consume anything in the queue, in case more interrupts were generated.
    (void)g_queue->Pop(0);

    const bool is_up = gpio_get(kPin) > 0;
    if (is_up != *rising) {
      printf("bounced\n");
      continue;  // A bounce occurred.
    }
    if (is_up) {
      printf("button pressed\n");
    } else {
      printf("button released\n");
    }
  }
}