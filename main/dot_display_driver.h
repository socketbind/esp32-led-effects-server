#ifndef __DOT_DISPLAY_SPI_H
#define __DOT_DISPLAY_SPI_H

#include "spi.h"
#include "dot_buffer.h"

typedef struct dot_display {
    spi_t *spi_device;
    int n_devices;
} dot_display_t;

void dot_display_init(dot_display_t *display, int n_devices, spi_t *spi_device);
void dot_display_sync(dot_display_t *display, dot_buffer_t *buffer);
void dot_display_clear(dot_display_t *display, int addr);
void dot_display_shutdown(dot_display_t *display, int addr, bool b);

#endif
