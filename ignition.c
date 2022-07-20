/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ignition.h"

#include "pico/stdlib.h"
#include "hardware/gpio.h"

Ignition_t ignition_init(
  uint coil_pin,
  uint indicator_pin,
  float dwell_ms,
  uint timing_light_pulse_us
) {
  Ignition_t ign = malloc(sizeof(struct ignition));
  ign->coil_pin = coil_pin;
  ign->indicator_pin = indicator_pin;
  ign->dwell_ms = dwell_ms;
  ign->timing_light_pulse_us = timing_light_pulse_us;
  ignition_init_io(ign);

  return ign;
}

void ignition_init_io(Ignition_t ign) {
  gpio_init(ign->indicator_pin);
  gpio_init(ign->coil_pin);
  gpio_set_dir(ign->indicator_pin, GPIO_OUT);
  gpio_set_dir(ign->coil_pin, GPIO_OUT);
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