/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef IGNITION_H
#define IGNITION_H

#include <pico/stdlib.h>
#include <pico/time.h>
#include <hardware/gpio.h>
#include "state.h"
#include "timing.h"
#include "scheduler.h"

typedef struct ignition* Ignition_t;

/**
 * Construct a new ignition instance
 */
Ignition_t ignition_init(
  uint8_t coil_pin,
  uint8_t indicator_pin,
  uint64_t dwell_us,
  uint64_t timing_light_pulse_us,
  timing_func_t get_timing
  // alarm_pool_t* alarm_pool
);

/**
 * Set up GPIO for the ignition output(s)
 */
void ignition_init_io(Ignition_t ign);

/**
 * Callback for the scheduler. Ignition logic lives here.
 * If the enging is running, this will begin a pulse to the ignition coil, initiating the dwell period.
 * The dwell period is then ended by a second callback, which is scheduled based provided timing function.
 * The second callback then reschedules `ignition_event_callback` for the following cycle.
 */
void ignition_event_callback(event_t* event);

/**
 * Setter for the timing function. Feel free to call in flight.
 */
void ignition_set_timing_func(Ignition_t ign, timing_func_t get_timing);

#endif
