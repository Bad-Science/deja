/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef TRIGGER_H
#define TRIGGER_H

#include <pico/stdlib.h>
#include "scheduler.h"

#define TRIGGER_POLL_PERIOD 20
#define TRIGGER_TIMEOUT_PERIOD 1000000

enum trigger_type{TRIGGER_COIL_ANALOG, TRIGGER_COIL_DIGITAL};
typedef enum trigger_type trigger_type_t;

typedef struct trigger* Trigger_t;
typedef void (*trigger_callback_t)(void);

Trigger_t trigger_init(
  trigger_type_t type,
  uint8_t pin,
  uint8_t local_frequency,
  float timing_offset_degrees
);

void trigger_event_callback(event_t* event);

#endif
