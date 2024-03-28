#include <FreeRTOS.h>

#include "FreeRTOSConfig.h"
#include "lwip/api.h"
#include "lwip/apps/mqtt.h"
#include "lwip/err.h"
#include "lwip/ip_addr.h"
#include "lwip/netbuf.h"
#include "lwip/opt.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "pico/cyw43_arch.h"
#include "pico/platform.h"
#include "pico/printf.h"
#include "pico/stdlib.h"
#include "portmacro.h"
#include "projdefs.h"
#include "semphr.h"
#include "task.h"

typedef struct _state {
  mqtt_client_t* client;
  QueueHandle_t queue;
} mqtt_state;

typedef struct _buf {
  void* data;
  u16_t len;
} buffer;

static void writer_task(void* arg) {
  // Allocate the client.
  mqtt_state* state = (mqtt_state*)arg;
  while (true) {
    buffer buffer;
    if (xQueueReceive(state->queue, &buffer, pdMS_TO_TICKS(1000)) != pdPASS) {
      continue;
    }
    err_t err;
    if ((err = mqtt_publish(state->client, "/pong", buffer.data, buffer.len, 1,
                            false, NULL, NULL)) != ERR_OK) {
      printf("error publishing to mqtt: %s\n", lwip_strerr(err));
    }
    free(buffer.data);
  }
}

void mqtt_connection_cb(mqtt_client_t* client, void* arg,
                        mqtt_connection_status_t status) {
  QueueHandle_t queue = arg;
  BaseType_t err = xQueueSend(queue, &status, portMAX_DELAY);
  if (err != ERR_OK) {
    panic("Error sending to queue %d\n", err);
  }
}

static void mqtt_incoming_data_cb(void* arg, const u8_t* data, u16_t len,
                                  u8_t flags) {
  mqtt_state* state = (mqtt_state*)arg;
  buffer buffer = {
      .data = malloc(len),
      .len = len,
  };
  MEMCPY(buffer.data, data, len);
  xQueueSend(state->queue, &buffer, portMAX_DELAY);
}

void start_initial_tasks() {
  mqtt_state* state = malloc(sizeof(mqtt_state));
  state->client = mqtt_client_new();
  state->queue = xQueueCreate(1, sizeof(buffer));

  ip_addr_t mqtt_server_address;
  err_t err = netconn_gethostbyname(MQTT_HOST, &mqtt_server_address);
  if (err != ERR_OK) {
    panic("error resolving" MQTT_HOST ": %s\n", lwip_strerr(err));
  }

  const struct mqtt_connect_client_info_t client_info = {
      .client_id = "pingpong",
      .client_user = MQTT_USER,
      .client_pass = MQTT_PASSWORD,
  };

  {
    QueueHandle_t result_queue =
        xQueueCreate(1, sizeof(mqtt_connection_status_t));
    err = mqtt_client_connect(state->client, &mqtt_server_address, 1883,
                              &mqtt_connection_cb, result_queue, &client_info);
    if (err != ERR_OK) {
      if (xQueueReceive(result_queue, &err, portMAX_DELAY) != ERR_OK) {
        panic("failed to receive error from queue\n");
      }
    }
    if (err != ERR_OK) {
      panic("error connecting to mqtt: %s\n", lwip_strerr(err));
    }
    vQueueDelete(result_queue);
  }

  xTaskCreate(writer_task, "writer", 512, (void*)1, 1, NULL);

  mqtt_set_inpub_callback(state->client, NULL, mqtt_incoming_data_cb, state);
  err = mqtt_subscribe(state->client, "/ping", 1, NULL, NULL);
  if (err != ERR_OK) {
    panic("error subscribing to mqtt: %s\n", lwip_strerr(err));
  }
}
