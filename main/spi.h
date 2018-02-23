#ifndef __SPI_H
#define __SPI_H

#include <driver/spi_master.h>
#include <driver/gpio.h>

#define DEFAULT_HOST        HSPI_HOST
#define DEFAULT_MOSI_PIN    GPIO_NUM_13
#define DEFAULT_MISO_PIN    GPIO_NUM_12
#define DEFAULT_CLK_PIN     GPIO_NUM_14
#define DEFAULT_CS_PIN      GPIO_NUM_15
#define PIN_NOT_SET         -1;

typedef struct spi_t {
    spi_device_handle_t handle;
    spi_host_device_t   host;
} spi_t;

void spi_init(spi_t *interface, spi_host_device_t host, int mosiPin, int misoPin, int clkPin, int csPin);
void spi_init_default(spi_t *interface);
void spi_transfer(spi_t *interface, uint8_t* data, size_t dataLen);
void spi_destroy(spi_t *interface);

#endif
