#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "string.h"

#include "display_loop.h"

#include "dot_display_driver.h"
#include "font8x8_basic_swapped.h"

#define TEXT_MAX_LENGTH 24
#define BUFFER_MAX_HORIZ 8

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

static int display_loop_mux_take(display_loop_t *loop) {
  return xSemaphoreTake(loop->mux, 1000 / portTICK_PERIOD_MS);
}

static int display_loop_mux_give(display_loop_t *loop) {
  return xSemaphoreGive(loop->mux);
}

static void display_loop_setup_effect(display_loop_t *loop, effect_fns_t const *fns) {
  if (loop->current_effect) {
    loop->current_effect->destroy_fn(*(loop->current_effect_data));
  }

  loop->current_effect = fns;
  loop->current_effect->init_fn(loop->current_effect_data, loop->buffer);
}

void display_loop_init(display_loop_t *loop, int n_devices) {
  loop->current_effect = NULL;
  loop->mux = xSemaphoreCreateMutex();

  loop->n_devices = n_devices;

  loop->buffer = (dot_buffer_t*) malloc(sizeof(dot_buffer_t));
  dot_buffer_init(loop->buffer, n_devices);
  dot_buffer_put_str(loop->buffer, 0, "NOPE", font8x8_basic_swapped);

  display_loop_setup_effect(loop, &available_effects[2]);

  loop->spi_device = (spi_t*) malloc(sizeof(spi_t));
  loop->dot_display = (dot_display_t*) malloc(sizeof(dot_display_t));

  spi_init_default(loop->spi_device);
  dot_display_init(loop->dot_display, n_devices, loop->spi_device);
}

void display_loop_destroy(display_loop_t *loop) {
  if (loop->current_effect) {
    loop->current_effect->destroy_fn(loop->current_effect_data);
  }

  spi_destroy(loop->spi_device);
  dot_buffer_destroy(loop->buffer);

  free(loop->spi_device);
  free(loop->dot_display);
  free(loop->buffer);

  vSemaphoreDelete(loop->mux);
}

void display_loop_run(display_loop_t *loop) {
  long elapsed = 0;

  while (1) {
    if (display_loop_mux_take(loop) == pdTRUE) {
      if (loop->current_effect) {
        dot_buffer_t *display_buffer = loop->current_effect->buffer_fn(*(loop->current_effect_data));
        dot_display_sync(loop->dot_display, display_buffer);
        loop->current_effect->tick_fn(*(loop->current_effect_data), elapsed);
      } else {
        dot_display_sync(loop->dot_display, loop->buffer);
      }

      display_loop_mux_give(loop);
    }

    vTaskDelay(100 / portTICK_PERIOD_MS);
    elapsed += 100;
  }
}

void display_loop_change_text_safe(display_loop_t *loop, char *text) {
  if (display_loop_mux_take(loop) == pdTRUE) {
    int text_length = strlen(text);
    int new_buf_horiz;
    if (text_length < loop->n_devices) {
      new_buf_horiz = loop->n_devices;
    } else if (text_length > TEXT_MAX_LENGTH) {
      new_buf_horiz = TEXT_MAX_LENGTH;
    } else {
      new_buf_horiz = text_length;
    }

    dot_buffer_destroy(loop->buffer);
    dot_buffer_init(loop->buffer, new_buf_horiz);

    dot_buffer_clear(loop->buffer);
    dot_buffer_put_str(loop->buffer, 0, text, font8x8_basic_swapped);

    display_loop_setup_effect(loop, loop->current_effect);

    dot_buffer_print(loop->buffer);
    fflush(stdout);

    display_loop_mux_give(loop);
  }
}

void display_loop_replace_buffer_safe(display_loop_t *loop, int n_horiz, uint8_t *dots) {
  if (display_loop_mux_take(loop) == pdTRUE) {
    int new_buf_horiz;
    if (n_horiz < loop->n_devices) {
      new_buf_horiz = loop->n_devices;
    } else if (n_horiz > BUFFER_MAX_HORIZ) {
      new_buf_horiz = BUFFER_MAX_HORIZ;
    } else {
      new_buf_horiz = n_horiz;
    }

    dot_buffer_destroy(loop->buffer);
    dot_buffer_init(loop->buffer, new_buf_horiz);

    if (new_buf_horiz == n_horiz) {
      memcpy(loop->buffer->dots, dots, n_horiz * 8);
    } {
      dot_buffer_clear(loop->buffer);

      int permissible_horiz = MIN(n_horiz, new_buf_horiz);

      int i, j;
      for (i = 0; i < permissible_horiz; i++) {
        for (j = 0; j < 8; j++) {
          loop->buffer->dots[i + j * new_buf_horiz] = dots[i + j * n_horiz];
        }
      }
    }

    display_loop_setup_effect(loop, loop->current_effect);

    dot_buffer_print(loop->buffer);
    fflush(stdout);

    display_loop_mux_give(loop);
  }
}

void display_loop_change_effect_safe(display_loop_t *loop, int effect_num) {
  if (effect_num >= 0 && effect_num < available_effects_num) {
    if (display_loop_mux_take(loop) == pdTRUE) {
      display_loop_setup_effect(loop, &available_effects[effect_num]);
      display_loop_mux_give(loop);
    }
  }
}
