/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef TRIGGER_H
#define TRIGGER_H

#include "pico/stdlib.h"

enum trigger_type{TRIGGER_COIL_ANALOG, TRIGGER_COIL_DIGITAL};
typedef enum trigger_type trigger_type_t;

typedef struct trigger* Trigger_t;
typedef void (*trigger_callback_t)(void);

Trigger_t trigger_init(
  trigger_type_t type,
  uint8_t pin,
  uint8_t local_freq,
  float timing_offset_degrees,
  void (*callback)(uint period)
);

#endif
