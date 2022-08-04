#ifndef SCREEN_H
#define SCREEN_H

#include <pico/stdlib.h>

typedef struct screen* Screen_t;
typedef uint8_t pixel_block_t;
typedef uint8_t screen_size_t;

Screen_t screen_init(screen_size_t h_size, screen_size_t v_size);
static  pixel_block_t* screen_buffer_init(screen_size_t num_pixels);
bool screen_set_pixel(Screen_t scr, screen_size_t x, screen_size_t y, bool value);
bool screen_draw_line(Screen_t scr, screen_size_t x1, screen_size_t y1, screen_size_t x2, screen_size_t y2);
bool screen_in_bounds(Screen_t scr, screen_size_t x, screen_size_t y);

#endif