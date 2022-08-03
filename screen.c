#include "screen.h"

#include <stdlib.h>
#include <pico/stdlib.h>

struct screen {
  pixel_block_t* buffer;
  uint8_t h_size, v_size;
};

struct screen* screen_init(uint8_t h_size, uint8_t v_size) {
  Screen_t scr = malloc(sizeof(struct screen));
  scr->h_size = h_size;
  scr->v_size = v_size;
  scr->buffer = screen_buffer_init(h_size * v_size);
}

pixel_block_t* screen_buffer_init(uint8_t num_pixels) {
  pixel_block_t* buffer = malloc((num_pixels / 8) / sizeof(pixel_block_t));
  return buffer;
}

