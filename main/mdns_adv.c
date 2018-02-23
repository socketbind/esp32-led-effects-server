#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "mdns.h"

#include "mdns_adv.h"

#define MDNS_HOSTNAME "gebdisplay"
#define MDNS_INSTANCE "Dot Display"

static mdns_server_t * mdns = NULL;
static const char *TAG = "mdns";

void mdns_adv_setup() {
  esp_err_t err = mdns_init(TCPIP_ADAPTER_IF_STA, &mdns);
  if (err) {
      ESP_LOGE(TAG, "MDNS failed to start: %u", err);
      return;
  }

  ESP_ERROR_CHECK( mdns_set_hostname(mdns, MDNS_HOSTNAME) );
  ESP_ERROR_CHECK( mdns_set_instance(mdns, MDNS_INSTANCE) );

  ESP_ERROR_CHECK( mdns_service_add(mdns, "_http", "_tcp", 80) );
  ESP_ERROR_CHECK( mdns_service_instance_set(mdns, "_http", "_tcp", "ESP32 Dot Display WebServer") );
}

void mdns_adv_teardown() {
  if (mdns) {
    mdns_free(mdns);
  }
}
