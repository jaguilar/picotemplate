
#include <FreeRTOS.h>
#include <task.h>

#include "FreeRTOSConfig.h"
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
  if (cyw43_arch_init()) {
    printf("Wi-Fi init failed");
    WaitForeverInCriticalSection();
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
