/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <hardware/gpio.h>
#include "ignition.h"
#include "state.h"
#include "timing.h"

struct ignition {
  uint coil_pin;
  uint indicator_pin;
  float dwell_ms;
  uint timing_light_pulse_us;
  alarm_pool_t* alarm_pool;
  timing_func_t get_timing;
};

Ignition_t ignition_init(
  uint coil_pin,
  uint indicator_pin,
  float dwell_ms,
  uint timing_light_pulse_us,
  timing_func_t get_timing,
  alarm_pool_t* alarm_pool
) {
  Ignition_t ign = malloc(sizeof(struct ignition));
  ign->coil_pin = coil_pin;
  ign->indicator_pin = indicator_pin;
  ign->dwell_ms = dwell_ms;
  ign->timing_light_pulse_us = timing_light_pulse_us;
  ign->get_timing = get_timing;
  ign->alarm_pool = alarm_pool;
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
  if (state.run) {
    ignition_start_dwell(id, data);
    float timing_dbtc = ign->get_timing(state.rpm);
    return state_offset_next_tdc_by_degrees(&state, timing_dbtc);
  } else {
    return 10000;
  }
}

void ignition_go(Ignition_t ign) {
  alarm_pool_add_alarm_in_us(ign->alarm_pool, 0, ignition_alarm_callback, ign, true);
  ignition_go_alarm_callback(0, ign);
}

void ignition_set_timing_func(Ignition_t ign, timing_func_t get_timing) {
  ign->get_timing = get_timing;
}

void ignition_schedule_spark_in_degrees(Ignition_t ign, float degrees, uint64_t period) {
  uint64_t ignite_in_us = (uint64_t) ((degrees / 360.0f) * period - (ign->dwell_ms * 1000.0f));
  ignition_schedule_dwell_in_us(ign, ignite_in_us);
}

void ignition_schedule_dwell_in_us(Ignition_t ign, uint64_t delay) {
  add_alarm_in_us(delay, ignition_start_dwell, ign, true);
}

int64_t ignition_start_dwell(alarm_id_t id, void* ign_param) {
  Ignition_t ign = (Ignition_t) ign_param;
  gpio_put(ign->coil_pin, 1);
  uint64_t duration = (uint64_t) (ign->dwell_ms * 1000.0f);

  add_alarm_in_us(duration, ignition_end_dwell, ign, true);

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