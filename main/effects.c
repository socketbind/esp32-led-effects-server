#include <stdlib.h>
#include "effects.h"

const effect_fns_t available_effects[] = {
  {
      .init_fn = scroll_left_init,
      .buffer_fn = scroll_left_buffer,
      .tick_fn = scroll_left_tick,
      .destroy_fn = scroll_left_destroy
  },
  {
      .init_fn = inverse_init,
      .buffer_fn = inverse_buffer,
      .tick_fn = inverse_tick,
      .destroy_fn = inverse_destroy
  },
  {
      .init_fn = blinky_frame_init,
      .buffer_fn = blinky_frame_buffer,
      .tick_fn = blinky_frame_tick,
      .destroy_fn = blinky_frame_destroy
  }
};

const int available_effects_num = sizeof(available_effects);

void scroll_left_init(void** scroll_data, dot_buffer_t *buffer) {
    *scroll_data = dot_buffer_clone(buffer);
}

dot_buffer_t* scroll_left_buffer(void* scroll_data) {
    return (dot_buffer_t*) scroll_data;
}

void scroll_left_tick(void* scroll_data, long elapsed) {
    dot_buffer_scroll_left((dot_buffer_t*) scroll_data, 1);
}

void scroll_left_destroy(void* scroll_data) {
    dot_buffer_destroy((dot_buffer_t*)scroll_data);
}

typedef struct inverse_data {
    dot_buffer_t *original;
    dot_buffer_t *inverted;
    bool flop;
} inverse_data_t;

void inverse_init(void** inverse_data, dot_buffer_t *buffer) {
    inverse_data_t *data = (inverse_data_t*) malloc(sizeof(inverse_data_t));
    data->flop = true;
    data->original = buffer;
    data->inverted = dot_buffer_clone(buffer);

    int i;
    for (i = 0; i < data->inverted->dots_len; i++) {
      data->inverted->dots[i] = ~data->inverted->dots[i];
    }

    *inverse_data = data;
}

dot_buffer_t* inverse_buffer(void* inverse_data) {
    inverse_data_t *data = (inverse_data_t*) inverse_data;

    if (data->flop) {
      return data->original;
    } else {
      return data->inverted;
    }
}

void inverse_tick(void* inverse_data, long elapsed) {
    inverse_data_t *data = (inverse_data_t*) inverse_data;
    data->flop = !data->flop;
}

void inverse_destroy(void* inverse_data) {
    inverse_data_t *data = (inverse_data_t*) inverse_data;
    dot_buffer_destroy(data->inverted);
    free(data);
}

typedef struct blinky_data {
    dot_buffer_t *original;
    dot_buffer_t *copy;
    bool flop;
} blinky_data_t;

void blinky_frame_init(void** blinky_data, dot_buffer_t *buffer) {
    blinky_data_t *data = (blinky_data_t*) malloc(sizeof(blinky_data_t));
    data->original = buffer;
    data->copy = dot_buffer_clone(buffer);
    data->flop = false;
    *blinky_data = data;
}

dot_buffer_t* blinky_frame_buffer(void* blinky_data) {
    return ((blinky_data_t*) blinky_data)->copy;
}

void blinky_frame_tick(void* blinky_data, long elapsed) {
    blinky_data_t *data = (blinky_data_t*) blinky_data;

    int i;
    for (i = 0; i < data->copy->n_horiz; i++) {
      if (data->flop) {
        data->copy->dots[i] = data->original->dots[i];

        int last_row = i + 7 * data->copy->n_horiz;
        data->copy->dots[last_row] = data->original->dots[last_row];
      } else {
        data->copy->dots[i] = 0xFF;
        data->copy->dots[i + 7 * data->copy->n_horiz] = 0xFF;
      }
    }

    data->flop = !data->flop;
}

void blinky_frame_destroy(void* blinky_data) {
    blinky_data_t *data = (blinky_data_t*) blinky_data;
    dot_buffer_destroy(data->copy);
    free(data);
}
