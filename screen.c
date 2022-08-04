#include "screen.h"

#include <stdlib.h>
#include <pico/stdlib.h>

struct screen {
  pixel_block_t* buffer;
  screen_size_t h_size, v_size;
};

Screen_t screen_init(screen_size_t h_size, screen_size_t v_size) {
  Screen_t scr = malloc(sizeof(struct screen));
  scr->h_size = h_size;
  scr->v_size = v_size;
  scr->buffer = screen_buffer_init(h_size * v_size);
}

pixel_block_t* screen_buffer_init(screen_size_t num_pixels) {
  pixel_block_t* buffer = malloc(num_pixels / 8);
  return buffer;
}

bool screen_set_pixel(Screen_t scr, screen_size_t x, screen_size_t y, bool value) {
  if (!screen_in_bounds(scr, x, y)) {
    return false;
  }
  screen_size_t pos = x + scr->h_size * y;
  screen_size_t buffer_pos = pos / sizeof(pixel_block_t);
  if (value) {
    scr->buffer[buffer_pos] |= (pixel_block_t) 1 << pos % sizeof(pixel_block_t);
  } else {
    scr->buffer[buffer_pos] &= (pixel_block_t) 0 << pos % sizeof(pixel_block_t);
  }

  return true;
}

bool screen_draw_line(Screen_t scr, screen_size_t x1, screen_size_t y1, screen_size_t x2, screen_size_t y2) {
  if (y1 == y2) {

  }
}

bool screen_in_bounds(Screen_t scr, screen_size_t x, screen_size_t y) {
  return (x < scr->h_size && y < scr->v_size);
}