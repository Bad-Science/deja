#ifndef SCREEN_H
#define SCREEN_H

#include <pico/stdlib.h>

typedef struct screen* Screen_t;
typedef uint64_t pixel_block_t;

static  pixel_block_t* screen_buffer_init(uint8_t num_pixels);

#endif