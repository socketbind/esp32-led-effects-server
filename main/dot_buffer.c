#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "dot_buffer.h"

void dot_buffer_init(dot_buffer_t *display, int n_horiz) {
  display->n_horiz = n_horiz;
  display->dots_len = n_horiz * 8;
  display->scanline_width = n_horiz * 8;
  display->dots = (uint8_t*) malloc(display->dots_len);
  memset(display->dots, 0, display->dots_len);
}

dot_buffer_t* dot_buffer_clone(dot_buffer_t *other_buffer) {
  dot_buffer_t* fresh_buffer = (dot_buffer_t*) malloc(sizeof(dot_buffer_t));
  fresh_buffer->n_horiz = other_buffer->n_horiz;
  fresh_buffer->scanline_width = other_buffer->scanline_width;
  fresh_buffer->dots_len = other_buffer->dots_len;
  fresh_buffer->dots = (uint8_t*) malloc(other_buffer->dots_len);
  memcpy(fresh_buffer->dots, other_buffer->dots, other_buffer->dots_len);
  return fresh_buffer;
}

void dot_buffer_destroy(dot_buffer_t *display) {
  free(display->dots);
}

static void display_bits(uint8_t value) {
  uint8_t mask = 0x80;
  for (int i = 0; i < 8; i++) {
    putchar((value & mask) != 0 ? '*' : ' ');
    mask >>= 1;
  }
}

void dot_buffer_print(dot_buffer_t *display) {
    int i;
    for (i = 0; i < display->dots_len; i++) {
      if (i > 0 && i % display->n_horiz == 0) {
        printf("\n");
      }
      display_bits(display->dots[i]);
    }
    printf("\n");
}

void dot_buffer_put_char(dot_buffer_t *display, uint8_t column, char c, uint8_t font[128][8]) {
  if (column > display->scanline_width) {
    return;
  }

  if (c > 127) {
    c = '?';
  }

  uint8_t arr_idx = column / 8;
  uint8_t bit_idx = column % 8;

  int i = 0;

  if (bit_idx == 0) {
    for (i = 0; i < 8; i++) {
      display->dots[arr_idx + i * display->n_horiz] = font[(uint8_t) c][i];
    }
  } else {
    for (i = 0; i < 8; i++) {
      uint8_t mask = 0xFF >> bit_idx;
      int first_idx = arr_idx + i * display->n_horiz;

      display->dots[first_idx] &= ~mask;
      display->dots[first_idx] |= font[(uint8_t) c][i] >> bit_idx;

      if (arr_idx + 1 < display->n_horiz) {


        int second_idx = (arr_idx + 1) + i * display->n_horiz;

        display->dots[second_idx] &= mask;
        display->dots[second_idx] |= font[(uint8_t) c][i] << (8 - bit_idx);
      }
    }
  }
}

void dot_buffer_put_str(dot_buffer_t *display, uint8_t column, char *str, uint8_t font[128][8]) {
  uint8_t current_column = column;
  int i, len = strlen(str);

  for (i = 0; i < len; i++) {
    dot_buffer_put_char(display, current_column, str[i], font);

    current_column += 8;
  }
}

void dot_buffer_scroll_left(dot_buffer_t *display, uint8_t by) {
  if (by == 0 || by > 8) return;

  int i, j;

  uint8_t mask = ~(0xFF >> by);

  for (j = 0; j < 8; j++) {
    uint8_t first_idx = j * display->n_horiz;
    uint8_t first_shifted = (display->dots[first_idx] & mask) >> (8 - by);
    display->dots[first_idx] <<= by;

    for (i = 1; i < display->n_horiz; i++) {
        uint8_t idx = i + j * display->n_horiz;

        uint8_t current_shifted = (display->dots[idx] & mask) >> (8 - by);
        display->dots[idx] <<= by;
        display->dots[idx-1] |= current_shifted;
    }

    uint8_t last_idx = (display->n_horiz - 1) + j * display->n_horiz;
    display->dots[last_idx] |= first_shifted;
  }
}

void dot_buffer_scroll_right(dot_buffer_t *display, uint8_t by) {
  if (by == 0 || by > 8) return;

  int i, j;

  uint8_t mask = ~(0xFF << by);

  for (j = 0; j < 8; j++) {
    uint8_t last_idx = (display->n_horiz-1) + j * display->n_horiz;
    uint8_t last_shifted = (display->dots[last_idx] & mask) << (8 - by);
    display->dots[last_idx] >>= by;

    for (i = display->n_horiz - 2; i >= 0; i--) {
        uint8_t idx = i + j * display->n_horiz;

        uint8_t current_shifted = (display->dots[idx] & mask) << (8 - by);
        display->dots[idx] >>= by;
        display->dots[idx+1] |= current_shifted;
    }

    uint8_t first_idx = j * display->n_horiz;
    display->dots[first_idx] |= last_shifted;
  }
}

void dot_buffer_clear(dot_buffer_t *buffer) {
  memset(buffer->dots, 0, buffer->dots_len);
}
