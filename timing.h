#ifndef TIMING_H
#define TIMING_H

#include <pico/stdlib.h>

#define TIMING_STATIC_VALUE 16.0f

typedef float (*timing_func_t)(uint64_t);

float timing_static(uint64_t rpm);
float timing_curved(uint64_t rpm);

#endif
