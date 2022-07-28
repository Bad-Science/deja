#ifndef TRIGGER_H
#define TRIGGER_H

#include "pico/stdlib.h"
#include "hardware/gpio.h"

typedef struct trigger Trigger_t;
typedef void (*trigger_callback_t)(void);

Trigger_t trigger_init();

static bool trigger_read_analog();

#endif
