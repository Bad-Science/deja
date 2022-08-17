/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <pico/stdlib.h>
#include <hardware/gpio.h>
#include <hardware/adc.h>
#include "trigger.h"
#include "helpers.h"
#include "state.h"

struct trigger {
  uint8_t pin;
  bool debounce;
  uint8_t local_frequency;
  uint64_t last_trigger;
  float timing_offset_degrees;
  bool (*poll)(); // use callback or return info struct from poll()?
  void (*callback)(uint period);
};

static inline void trigger_update_state(Trigger_t trig) {
  uint64_t current_time = to_us_since_boot(get_absolute_time());
  uint64_t time_delta = current_time - trig->last_trigger;
  uint16_t rpm = 6E7 / (time_delta * trig->local_frequency);

  uint64_t timing_offset_us = time_delta * (trig->timing_offset_degrees / 360.f);

  State_t update = state_begin_write();
  update.last_tdc = current_time;
  update.next_tdc = timing_offset_us + current_time;
  state_commit_write(&update);
}

static bool trigger_read_analog(Trigger_t trig) {
  uint64_t millivolts = read_adc_channel(trig->pin - ADC_CHANNEL_OFFSET);
  if (millivolts  > 100 && !trig->debounce) {
    trigger_update_state(trig);
  }
}

static bool trigger_read_digital() {

}

Trigger_t trigger_init(
  enum trigger_type type,
  uint8_t pin,
  uint8_t local_frequency,
  float timing_offset_degrees,
  void (*callback)(uint period)
) {
  Trigger_t trig = malloc(sizeof(struct trigger));
  trig->pin = pin;
  trig->local_frequency = local_frequency;
  trig->last_trigger = 0;
  if (callback) {
    trig->callback = callback;
  }

  if (type == TRIGGER_COIL_ANALOG) {
    adc_gpio_init(pin);
    trig->poll = trigger_read_analog;
  } else if (type == TRIGGER_COIL_DIGITAL) {
    trig->poll = trigger_read_digital;
  }

  return trig;
}

static inline void trigger_recalculate_rpm(Trigger_t trig) {

}
