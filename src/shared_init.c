
#include <FreeRTOS.h>
#include <task.h>

#include "FreeRTOSConfig.h"
#include "cyw43_country.h"
#include "cyw43_ll.h"
#include "lwip/apps/mdns.h"
#include "lwip/debug.h"
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
  printf("stack overflow in %s\n", pcTaskName);
  WaitForeverInCriticalSection();
}
#endif

extern void start_initial_tasks();

static TaskHandle_t g_init_task;

static void srv_txt(struct mdns_service *service, void *txt_userdata) {
  err_t res;
  LWIP_UNUSED_ARG(txt_userdata);

  res = mdns_resp_add_service_txtitem(service, "path=/", 6);
  LWIP_ERROR("mdns add service txt failed\n", (res == ERR_OK), return );
}

static void mdns_example_report(struct netif *netif, u8_t result,
                                s8_t service) {
  LWIP_PLATFORM_DIAG(
      ("mdns status[netif %d][service %d]: %d\n", netif->num, service, result));
}

static void tcpip_hwm_watcher(void *) {
  TaskHandle_t task = NULL;
  while (task == NULL) {
    task = xTaskGetHandle("tcpip_thread");
    if (task == NULL) vTaskDelay(pdMS_TO_TICKS(1000));
  }
  while (1) {
    vTaskDelay(pdMS_TO_TICKS(1000));
    printf("TCP HWM: %d\n", uxTaskGetStackHighWaterMark(task));
  }
}

static void init_task(void *unused) {
  printf("will initialize wifi\n");
#ifdef RASPBERRYPI_PICO_W
  if (cyw43_arch_init_with_country(CYW43_COUNTRY_USA)) {
    printf("Wi-Fi init failed");
    WaitForeverInCriticalSection();
  }
  printf("wifi init done\n");
  cyw43_arch_enable_sta_mode();

  xTaskCreate(tcpip_hwm_watcher, "tcpip_hwm_watcher", 256, NULL, 1, NULL);

  printf("will connect wifi\n");
  {
    int error;
    if ((error = cyw43_arch_wifi_connect_blocking(
             WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK)) != 0) {
      printf("Failed to connect to %s (pwd %s): %d\n", WIFI_SSID, WIFI_PASSWORD,
             error);
    }
  }
  printf("wifi connected\n");
  for (int i = 0; i < 3; ++i) {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
  }

#if LWIP_MDNS_RESPONDER
  mdns_resp_register_name_result_cb(mdns_example_report);
  mdns_resp_init();
  mdns_resp_add_netif(netif_default, CYW43_HOST_NAME);
  mdns_resp_add_service(netif_default, "telnet", "_telnet", DNSSD_PROTO_TCP, 23,
                        srv_txt, NULL);
  mdns_resp_announce(netif_default);
#endif  // LWIP_MDNS_RESPONDER
#endif  // RASPBERRYPI_PICO_W
  start_initial_tasks();
  vTaskDelete(g_init_task);
}

int main(void) {
  stdio_init_all();
  BaseType_t err =
      xTaskCreate(init_task, "__init_task", 1024, NULL, 1, &g_init_task);
  configASSERT(err == pdPASS);
  vTaskStartScheduler();
}
