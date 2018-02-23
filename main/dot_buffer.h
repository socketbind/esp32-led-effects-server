#ifndef __DOT_BUFFER_H
#define __DOT_BUFFER_H

#include <stdint.h>

typedef struct dot_buffer {
    int n_horiz;
    uint8_t *dots;
    int dots_len;
    int scanline_width;
} dot_buffer_t;

void dot_buffer_init(dot_buffer_t *buffer, int n_horiz);
dot_buffer_t* dot_buffer_clone(dot_buffer_t *other_buffer);
void dot_buffer_destroy(dot_buffer_t *buffer);
void dot_buffer_print(dot_buffer_t *buffer);
void dot_buffer_put_char(dot_buffer_t *buffer, uint8_t column, char c, uint8_t font[128][8]);
void dot_buffer_put_str(dot_buffer_t *buffer, uint8_t column, char *str, uint8_t font[128][8]);
void dot_buffer_scroll_left(dot_buffer_t *buffer, uint8_t by);
void dot_buffer_scroll_right(dot_buffer_t *buffer, uint8_t by);
void dot_buffer_clear(dot_buffer_t *buffer);

#endif
