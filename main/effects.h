#ifndef __EFFECTS_H
#define __EFFECTS_H

#include <stdbool.h>
#include "dot_buffer.h"

typedef void (*effect_init_fn)(void**, dot_buffer_t*);
typedef void (*effect_tick_fn)(void*, long); // elapsed time
typedef dot_buffer_t* (*effect_buffer_fn)(void*);
typedef void (*effect_destroy_fn)(void*);

typedef struct effect_fns {
    effect_init_fn init_fn;
    effect_tick_fn tick_fn;
    effect_buffer_fn buffer_fn;
    effect_destroy_fn destroy_fn;
} effect_fns_t;

void scroll_left_init(void** scroll_data, dot_buffer_t *buffer);
dot_buffer_t* scroll_left_buffer(void* scroll_data);
void scroll_left_tick(void* scroll_data, long elapsed);
void scroll_left_destroy(void* scroll_data);

void inverse_init(void** inverse_data, dot_buffer_t *buffer);
dot_buffer_t* inverse_buffer(void* inverse_data);
void inverse_tick(void* scroll_data, long elapsed);
void inverse_destroy(void* inverse_data);

void blinky_frame_init(void** blinky_data, dot_buffer_t *buffer);
dot_buffer_t* blinky_frame_buffer(void* blinky_data);
void blinky_frame_tick(void* blinky_data, long elapsed);
void blinky_frame_destroy(void* blinky_data);

extern const effect_fns_t available_effects[];
extern const int available_effects_num;

#endif
