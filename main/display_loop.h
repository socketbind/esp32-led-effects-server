#ifndef __DISPLAY_LOOP_H
#define __DISPLAY_LOOP_H

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "spi.h"
#include "dot_buffer.h"
#include "dot_display_driver.h"
#include "effects.h"

typedef struct display_loop {
    int n_devices;
    spi_t *spi_device;
    dot_display_t *dot_display;
    dot_buffer_t *buffer;
    effect_fns_t const *current_effect;
    void** current_effect_data;

    SemaphoreHandle_t *mux;
} display_loop_t;

void display_loop_init(display_loop_t *loop, int n_devices);
void display_loop_destroy(display_loop_t *loop);
void display_loop_run(display_loop_t *loop);
void display_loop_change_text_safe(display_loop_t *loop, char *text);
void display_loop_replace_buffer_safe(display_loop_t *loop, int n_horiz, uint8_t *dots);
void display_loop_change_effect_safe(display_loop_t *loop, int effect_num);

#endif
