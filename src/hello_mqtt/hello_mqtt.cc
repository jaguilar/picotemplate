#include <FreeRTOS.h>

#include <memory>
#include <string>
#include <variant>

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

class PingPongClient {
 public:
  PingPongClient() { topic_.reserve(32); }

  ~PingPongClient() {
    if (client_) mqtt_client_free(client_);
    if (queue_) vQueueDelete(queue_);
  }

  static err_t Create(std::unique_ptr<PingPongClient>& client) {
    ip_addr_t mqtt_server_address;
    err_t err;
    do {
      err = netconn_gethostbyname(MQTT_HOST, &mqtt_server_address);
      if (err != ERR_OK) {
        printf("error resolving " MQTT_HOST ": %s\n", lwip_strerr(err));
        vTaskDelay(pdMS_TO_TICKS(1000));
      }

    } while (err != ERR_OK);

    const struct mqtt_connect_client_info_t client_info = {
        .client_id = "pingpong",
        .client_user = MQTT_USER,
        .client_pass = MQTT_PASSWORD,
    };

    client = std::make_unique<PingPongClient>();
    client->client_ = mqtt_client_new();
    client->queue_ = xQueueCreate(10, sizeof(std::string*));

    {
      QueueHandle_t result_queue =
          xQueueCreate(1, sizeof(mqtt_connection_status_t));
      mqtt_connection_cb_t cb = +[](mqtt_client_s* client, void* queue_arg,
                                    mqtt_connection_status_t status) {
        xQueueSend(static_cast<QueueHandle_t>(queue_arg), &status,
                   portMAX_DELAY);
      };
      err = mqtt_client_connect(client->client_, &mqtt_server_address, 1883, cb,
                                result_queue, &client_info);
      if (err != ERR_OK) {
        panic("error from mqtt_client_connect: %s\n", lwip_strerr(err));
      }
      mqtt_connection_status_t status;
      if (xQueueReceive(result_queue, &status, portMAX_DELAY) != pdTRUE) {
        panic("failed to receive error from queue\n");
      }
      if (status != MQTT_CONNECT_ACCEPTED) {
        panic("error connecting to mqtt: %d\n", status);
      }
    }

    xTaskCreate(
        +[](void* arg) { static_cast<PingPongClient*>(arg)->WriterTask(); },
        "writer", 512, client.get(), 1, &client->writer_task_);

    mqtt_incoming_publish_cb_t pub_cb =
        +[](void* self, const char* topic, u32_t num_data) {
          static_cast<PingPongClient*>(self)->SetTopic(std::string_view(topic),
                                                       num_data);
        };
    mqtt_incoming_data_cb_t data_cb =
        +[](void* self, const u8_t* data, u16_t len, u8_t flags) {
          static_cast<PingPongClient*>(self)->RecvData(
              std::string_view(reinterpret_cast<const char*>(data), len));
        };
    mqtt_set_inpub_callback(client->client_, pub_cb, data_cb, client.get());
    err = mqtt_subscribe(client->client_, "/ping", 1, NULL, NULL);
    if (err != ERR_OK) {
      panic("error subscribing to mqtt: %s\n", lwip_strerr(err));
    }
    return 0;
  }

 private:
  void SetTopic(std::string_view topic, int num_data) {
    topic_.assign(topic);
    num_expected_items_ = num_data;
  }

  void RecvData(std::string_view payload) {
    --num_expected_items_;
    if (num_expected_items_ == 0) {
      topic_ = "";
    }
    if (num_expected_items_ < 0) {
      panic("more than the expected number of items received\n");
    }
    if (topic_ != "/ping") return;
    std::string* buffer = new std::string(payload);
    xQueueSend(queue_, &buffer, portMAX_DELAY);
  }

  static void WriterTask(void* arg) {
    reinterpret_cast<PingPongClient*>(arg)->WriterTask();
  }

  void WriterTask() {
    while (true) {
      std::string* buffer_ptr;
      if (xQueueReceive(queue_, &buffer_ptr, pdMS_TO_TICKS(1000)) != pdTRUE) {
        continue;
      }
      std::unique_ptr<std::string> buffer(buffer_ptr);

      err_t err;
      if ((err = mqtt_publish(client_, "/pong", buffer->data(), buffer->size(),
                              1, false, NULL, NULL)) != ERR_OK) {
        printf("error publishing to mqtt: %s\n", lwip_strerr(err));
      }
    }
  }

  mqtt_client_t* client_ = nullptr;
  TaskHandle_t writer_task_ = nullptr;
  QueueHandle_t queue_ = nullptr;

  std::string topic_;
  int num_expected_items_ = 0;
};

extern "C" void start_initial_tasks() {
  PingPongClient::Create(*(new std::unique_ptr<PingPongClient>));
}
