/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "nvs_flash.h"
#include "display_loop.h"
#include "wifi_connection.h"
#include "http_server_mng.h"
#include "mdns_adv.h"

#define DEVICES 4

void app_main() {
    ESP_ERROR_CHECK( nvs_flash_init() );

    wifi_conn_init();

    mdns_adv_setup();

    display_loop_t loop;
    display_loop_init(&loop, DEVICES);

    http_server_mng_init(&loop);

    display_loop_run(&loop);
    display_loop_destroy(&loop);

    mdns_adv_teardown();

    printf("Restarting now.\n");
    fflush(stdout);

    esp_restart();
}
