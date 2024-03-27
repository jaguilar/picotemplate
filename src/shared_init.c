
#include <FreeRTOS.h>
#include <task.h>

#include "FreeRTOSConfig.h"
#include "cyw43_country.h"
#include "cyw43_ll.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "portmacro.h"
#include "projdefs.h"

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

extern void start_initial_tasks();

static TaskHandle_t g_init_task;

static void init_task(void *unused) {
#ifdef RASPBERRYPI_PICO_W
  if (cyw43_arch_init_with_country(CYW43_COUNTRY_USA)) {
    printf("Wi-Fi init failed");
    WaitForeverInCriticalSection();
  }
  cyw43_arch_enable_sta_mode();

  {
    int error;
    if ((error = cyw43_arch_wifi_connect_blocking(
             WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK)) != 0) {
      printf("Failed to connect to %s (pwd %s): %d\n", WIFI_SSID, WIFI_PASSWORD,
             error);
    }
  }
  for (int i = 0; i < 3; ++i) {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
  }
#endif
  start_initial_tasks();
  vTaskDelete(g_init_task);
}

int main(void) {
  stdio_init_all();
  configASSERT(xTaskCreate(init_task, "__init_task", 1024, NULL, 1,
                           &g_init_task) == pdPASS);
  vTaskStartScheduler();
}
