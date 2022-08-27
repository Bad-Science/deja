/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <pico/stdlib.h>
#include <hardware/gpio.h>
#include "ignition.h"
#include "state.h"
#include "timing.h"
#include "scheduler.h"

#define SCHEDULE_DWELL 0

struct ignition {
  uint8_t coil_pin;
  uint8_t indicator_pin;
  uint64_t dwell_us;
  uint64_t timing_light_pulse_us;
  alarm_pool_t* alarm_pool;
  timing_func_t get_timing;
};

Ignition_t ignition_init(
  uint8_t coil_pin,
  uint8_t indicator_pin,
  uint64_t dwell_us,
  uint64_t timing_light_pulse_us,
  timing_func_t get_timing
) {
  Ignition_t ign = malloc(sizeof(struct ignition));
  ign->coil_pin = coil_pin;
  ign->indicator_pin = indicator_pin;
  ign->dwell_us = dwell_us;
  ign->timing_light_pulse_us = timing_light_pulse_us;
  ign->get_timing = get_timing;
  ignition_init_io(ign);

  return ign;
}

void ignition_init_io(Ignition_t ign) {
  gpio_init(ign->indicator_pin);
  gpio_init(ign->coil_pin);
  gpio_set_dir(ign->indicator_pin, GPIO_OUT);
  gpio_set_dir(ign->coil_pin, GPIO_OUT);
}

static void ignition_dwell_event_callback(event_t* event) {
  Ignition_t ign = event->param;
  State_t state = state_get();

  // ~~ Zap! ~~
  gpio_put(ign->coil_pin, 0);

  // Schedule start of next dwell at `dwell_us` prior to the desired timing in the following engine cycle
  event->mode = NEXT_CYCLE;
  event->what = ignition_event_callback;
  event->when.degrees = ign->get_timing(&state);
  event->when.us = -ign->dwell_us;
}

/**
 * TODO:
 *  A. Increase dwell maximum to ~2ms. After dwell period (and possibly half way thru)
 *  re-check state.next_tdc and update schedule. Will need to add ticker.
 *  B. 
 * 
 * */
void ignition_event_callback(event_t* event) {
  Ignition_t ign = event->param;
  State_t state = state_get();
  if (state.running) {
    // Begin dwell period
    gpio_put(ign->coil_pin, 1);

    // Schedule spark (end of dwell period) for the desired timing in the present engine cycle
    event->mode = SAME_CYCLE;
    event->what = ignition_dwell_event_callback;
    event->when.degrees = ign->get_timing(&state);
    event->when.us = 0;

  } else {
    // If the engine is stopped, go back to starting mode
    event->mode = NEXT_CYCLE;
    event->what = ignition_event_callback;
    event->when.degrees = TIMING_STATIC_VALUE;
    event->when.us = -ign->dwell_us;
  }
}

void ignition_set_timing_func(Ignition_t ign, timing_func_t get_timing) {
  ign->get_timing = get_timing;
}
