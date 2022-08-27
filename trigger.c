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
#include "scheduler.h"

struct trigger {
  uint8_t pin;
  bool debounce;
  uint8_t local_frequency; // TODO: Move to config (not state)
  uint64_t last_trigger;
  float timing_offset_degrees;
  uint64_t clock;
  bool (*read)(Trigger_t);
};

static inline void trigger_update_state(Trigger_t trig) {
  uint64_t current_time = to_us_since_boot(get_absolute_time());

  uint64_t trigger_period = current_time - trig->last_trigger;
  uint16_t physical_period = trigger_period * trig->local_frequency;

  State_t state = state_get();
  float timing_offset_degrees = state.trigger_timing_offset + trig->timing_offset_degrees;
  uint64_t timing_offset_us = physical_period * (timing_offset_degrees / 360.f);

  State_t update = state_begin_write();
  update.physical_period = physical_period;
  update.ignition_period = trigger_period;
  update.next_tdc = current_time + trigger_period + timing_offset_us;
  ++update.clock;
  state_commit_write(&update);

  trig->last_trigger = current_time;
}

static bool trigger_read_analog(Trigger_t trig) {
  uint64_t millivolts = read_adc_channel(trig->pin - ADC_CHANNEL_OFFSET);
  if (millivolts  > 100 && !trig->debounce) {
    trig->debounce = true;
    return true;
  } else if (millivolts <= 75) {
    trig->debounce = false;
  }
  return false;
}

static bool trigger_read_digital() {
  // TODO
}

Trigger_t trigger_init(
  enum trigger_type type,
  uint8_t pin,
  uint8_t local_frequency,
  float timing_offset_degrees
) {
  Trigger_t trig = malloc(sizeof(struct trigger));
  trig->pin = pin;
  trig->local_frequency = local_frequency;
  trig->last_trigger = 0;
  trig->clock = 0;

  if (type == TRIGGER_COIL_ANALOG) {
    adc_gpio_init(pin);
    trig->read = trigger_read_analog;
  } else if (type == TRIGGER_COIL_DIGITAL) {
    trig->read = trigger_read_digital;
  }

  return trig;
}

void trigger_event_callback(event_t* event) {
  Trigger_t trig = event->param;
  bool triggered = trig->read(trig);
  State_t state = state_get();
  if (triggered) {
    if (!state.running) {
      // Engine is running again!
      State_t update = state_begin_write();
      state_set_running(&update, true);
      state_commit_write(&update);
    }
    trigger_update_state(trig);
  } else if (state.running && time_us_64() - trig->last_trigger > TRIGGER_TIMEOUT_PERIOD) {
    // Engine has stopped
    State_t update = state_begin_write();
    state_set_running(&update, false);
    state_commit_write(&update);
  }
}
