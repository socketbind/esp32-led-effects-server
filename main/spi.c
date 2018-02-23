#include <driver/spi_master.h>
#include <esp_log.h>
#include "sdkconfig.h"

#include "spi.h"

static const char* LOG_TAG = "SPI";

void spi_init(spi_t *interface, spi_host_device_t host, int mosiPin, int misoPin, int clkPin, int csPin) {
    interface->handle = 0;
    interface->host = host;

    ESP_LOGD(LOG_TAG, "spi_init: mosi=%d, miso=%d, clk=%d, cs=%d", mosiPin, misoPin, clkPin, csPin);

    spi_bus_config_t bus_config;
    bus_config.sclk_io_num     = clkPin;  // CLK
    bus_config.mosi_io_num     = mosiPin; // MOSI
    bus_config.miso_io_num     = misoPin; // MISO
    bus_config.quadwp_io_num   = -1;      // Not used
    bus_config.quadhd_io_num   = -1;      // Not used
    bus_config.max_transfer_sz = 0;       // 0 means use default.

    ESP_LOGI(LOG_TAG, "... Initializing bus; host=%d", interface->host);

    esp_err_t errRc = spi_bus_initialize(
        interface->host,
        &bus_config,
        1 // DMA Channel
    );

    if (errRc != ESP_OK) {
      ESP_LOGE(LOG_TAG, "spi_bus_initialize(): rc=%d", errRc);
      abort();
    }

    spi_device_interface_config_t dev_config;
    dev_config.address_bits     = 0;
    dev_config.command_bits     = 0;
    dev_config.dummy_bits       = 0;
    dev_config.mode             = 0;
    dev_config.duty_cycle_pos   = 0;
    dev_config.cs_ena_posttrans = 0;
    dev_config.cs_ena_pretrans  = 0;
    dev_config.clock_speed_hz   = 100000;
    dev_config.spics_io_num     = csPin;
    dev_config.flags            = 0;
    dev_config.queue_size       = 1;
    dev_config.pre_cb           = NULL;
    dev_config.post_cb          = NULL;
    ESP_LOGI(LOG_TAG, "... Adding device bus.");
    errRc = spi_bus_add_device(interface->host, &dev_config, &interface->handle);
    if (errRc != ESP_OK) {
      ESP_LOGE(LOG_TAG, "spi_bus_add_device(): rc=%d", errRc);
      abort();
    }
}

void spi_init_default(spi_t *interface) {
  spi_init(interface, DEFAULT_HOST, DEFAULT_MOSI_PIN, DEFAULT_MISO_PIN, DEFAULT_CLK_PIN, DEFAULT_CS_PIN);
}

void spi_transfer(spi_t *interface, uint8_t* data, size_t dataLen) {
	spi_transaction_t trans_desc;
	//trans_desc.address   = 0;
	//trans_desc.command   = 0;
	trans_desc.flags     = 0;
	trans_desc.length    = dataLen * 8;
	trans_desc.rxlength  = 0;
	trans_desc.tx_buffer = data;
	trans_desc.rx_buffer = data;

	esp_err_t rc = spi_device_transmit(interface->handle, &trans_desc);
	if (rc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "transfer:spi_device_transmit: %d", rc);
	}
}

void spi_destroy(spi_t *interface) {
    if (interface->handle) {
      ESP_LOGI(LOG_TAG, "... Removing device.");
      ESP_ERROR_CHECK(spi_bus_remove_device(interface->handle));
    }

    ESP_LOGI(LOG_TAG, "... Freeing bus.");
    ESP_ERROR_CHECK(spi_bus_free(interface->host));
}
