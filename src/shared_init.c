
#include <FreeRTOS.h>
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

void prvSetupHardware(void) {
  stdio_init_all();
#ifdef RASPBERRYPI_PICO_W
  if (cyw43_arch_init()) {
    printf("Wi-Fi init failed");
    WaitForeverInCriticalSection();
  }
#endif
}
