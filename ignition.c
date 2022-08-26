/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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
  // alarm_pool_t* alarm_pool
) {
  Ignition_t ign = malloc(sizeof(struct ignition));
  ign->coil_pin = coil_pin;
  ign->indicator_pin = indicator_pin;
  ign->dwell_us = dwell_us;
  ign->timing_light_pulse_us = timing_light_pulse_us;
  ign->get_timing = get_timing;
  // ign->alarm_pool = alarm_pool;
  ignition_init_io(ign);

  return ign;
}

void ignition_init_io(Ignition_t ign) {
  gpio_init(ign->indicator_pin);
  gpio_init(ign->coil_pin);
  gpio_set_dir(ign->indicator_pin, GPIO_OUT);
  gpio_set_dir(ign->coil_pin, GPIO_OUT);
}

int64_t ignition_alarm_callback(alarm_id_t id, void* data) {
  Ignition_t ign = (Ignition_t) data;
  State_t state = state_get();
  if (state.running) {
    ignition_start_dwell(id, data);
    float timing_dbtc = ign->get_timing(&state);
    return state_offset_next_tdc_by_degrees(&state, timing_dbtc);
  } else {
    return 10000;
  }
}

void ignition_go(Ignition_t ign) {
  alarm_pool_add_alarm_in_us(ign->alarm_pool, 0, ignition_alarm_callback, ign, true);
}

static ignition_dwell_event_callback(event_t* event) {
  Ignition_t ign = event->param;
  State_t state = state_get();

  // ~~ Zap! ~~
  gpio_put(ign->coil_pin, 0);

  // Schedule start of next dwell
  event->what = ignition_event_callback;
  event->when.degrees = ign->get_timing(&state);
  event->when.us = -ign->dwell_us;

  return EVENT_NEXT_CYCLE;
}

/**
 * TODO:
 *  A. Increase dwell maximum to ~2ms. After dwell period (and possibly half way thru)
 *  re-check state.next_tdc and update schedule. Will need to add ticker.
 *  B. 
 * 
 * */
bool ignition_event_callback(event_t* event) {
  Ignition_t ign = event->param;
  State_t state = state_get();
  if (state.running) {
    // Charge...
    gpio_put(ign->coil_pin, 1);

    // And again :)
    event->when.degrees = ign->get_timing(&state);
    event->when.us = 0;
  } else {
    event->when.degrees = TIMING_STATIC_VALUE;
    event->when.us = -ign->dwell_us;
  }

  return EVENT_THIS_CYCLE;
}

void ignition_set_timing_func(Ignition_t ign, timing_func_t get_timing) {
  ign->get_timing = get_timing;
}

void ignition_schedule_spark_in_degrees(Ignition_t ign, float degrees, uint64_t period) {
  uint64_t ignite_in_us = (uint64_t) ((degrees / 360.0f) * period - ign->dwell_us);
  ignition_schedule_dwell_in_us(ign, ignite_in_us);
}

void ignition_schedule_dwell_in_us(Ignition_t ign, uint64_t delay) {
  add_alarm_in_us(delay, ignition_start_dwell, ign, true);
}

int64_t ignition_start_dwell(alarm_id_t id, void* ign_param) {
  Ignition_t ign = (Ignition_t) ign_param;
  gpio_put(ign->coil_pin, 1);
  add_alarm_in_us(ign->dwell_us, ignition_end_dwell, ign, true);

  return 0;
}

int64_t ignition_end_dwell(alarm_id_t id, void* ign_param) {
  Ignition_t ign = (Ignition_t) ign_param;
  gpio_put(ign->coil_pin, 0);

  if (ign->timing_light_pulse_us > 0) {
    gpio_put(ign->indicator_pin, 1);
    add_alarm_in_us(ign->timing_light_pulse_us, ignition_end_timing_light, ign, true);
  }

  return 0;
}

int64_t ignition_end_timing_light(alarm_id_t id, void* ign_param) {
  Ignition_t ign = (Ignition_t) ign_param;
  gpio_put(ign->indicator_pin, 0);

  return 0;
}