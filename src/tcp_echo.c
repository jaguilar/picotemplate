#include <FreeRTOS.h>

#include "FreeRTOSConfig.h"
#include "lwip/api.h"
#include "lwip/err.h"
#include "lwip/ip_addr.h"
#include "lwip/netbuf.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "pico/cyw43_arch.h"
#include "pico/printf.h"
#include "pico/stdlib.h"
#include "portmacro.h"
#include "semphr.h"
#include "task.h"

static SemaphoreHandle_t g_connection_sema;

typedef struct _server {
  struct netconn *conn;
} server_t;

static const char *get_address_string(struct netconn *conn) {
  ip_addr_t addr;
  u16_t port;
  err_t err = netconn_getaddr(conn, &addr, &port, 23);
  if (err != ERR_OK) {
    printf("error getting address: %s\n", lwip_strerr(err));
    return "";
  }
  return ipaddr_ntoa(&addr);
}

void echo_server_task(void *arg) {
  struct netconn *net = arg;

  err_t error;
  char addr[32];
  snprintf(addr, sizeof(addr), "%s", get_address_string(net));
  printf("accepted connection from %s\n", addr);
  struct netbuf *buf = 0;
  while ((error = netconn_recv(net, &buf)) == ERR_OK) {
    void *data;
    u16_t len;
    configASSERT(netbuf_data(buf, &data, &len) == ERR_OK);
    if ((error = netconn_write(net, data, len, NETCONN_COPY)) != ERR_OK) {
      printf("error sending data: %s\n", lwip_strerr(error));
    }
    netbuf_delete(buf);
    buf = 0;
  }
  printf("closing connection from %s with err %s\n", addr, lwip_strerr(error));
  if ((error = netconn_close(net)) != ERR_OK) {
    printf("error closing connection: %s\n", addr, lwip_strerr(error));
  }
  netconn_delete(net);
  vTaskDelete(NULL);
}

void listen_task(void *arg) {
  g_connection_sema = xSemaphoreCreateCounting(4, 0);

  server_t server;
  server.conn = netconn_new(NETCONN_TCP);
  netconn_bind(server.conn, IP_ADDR_ANY, 23);
  netconn_listen(server.conn);
  struct netconn *accepted;
  int error;

  int num_tasks = 0;
  while ((error = netconn_accept(server.conn, &accepted)) == ERR_OK) {
    char task_name[16];
    snprintf(task_name, 16, "echo_%d", num_tasks++);
    xTaskCreate(echo_server_task, task_name, 256, accepted, 1, NULL);
  }
}

void start_initial_tasks() {
  xTaskCreate(listen_task, "listen_task", 512, (void *)1, 1, NULL);
}
