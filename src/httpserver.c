#include <FreeRTOS.h>
#include <stdio.h>
#include <task.h>

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "portmacro.h"

void WaitForeverInCriticalSection() {
  portDISABLE_INTERRUPTS();
  taskENTER_CRITICAL();
  int setToZeroToContinue = 1;
  while (setToZeroToContinue != 0) {
    portNOP();
  }
  taskEXIT_CRITICAL();
  portENABLE_INTERRUPTS();
}

#if configUSE_MALLOC_FAILED_HOOK
void vApplicationMallocFailedHook(void) { WaitForeverInCriticalSection(); }
#endif

#if (configCHECK_FOR_STACK_OVERFLOW != 0)
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName) {
  WaitForeverInCriticalSection();
}
#endif

void led_task() {
  while (true) {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(250));
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(250));
  }
}

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
  stdio_init_all();
  if (cyw43_arch_init()) {
    printf("Wi-Fi init failed");
    return -1;
  }

  xTaskCreate(main_task, "main_task", 1024, (void *)1, 1, &g_main_handle);
  vTaskStartScheduler();

  while (1) {
  };
}
