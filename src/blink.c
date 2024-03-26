#include <FreeRTOS.h>
#include <stdio.h>
#include <task.h>

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "portmacro.h"

void main_task(void *arg) {
  while (true) {
    printf("loop\n");
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(250));
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(250));
  }
}

static TaskHandle_t g_main_handle;

int main() {
  xTaskCreate(main_task, "main_task", 1024, (void *)1, 1, &g_main_handle);
  vTaskStartScheduler();
  while (1) {
  };
}
