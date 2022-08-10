/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "trigger.h"
#include "helpers.h"

#include <pico/stdlib.h>
#include "hardware/gpio.h"
#include "hardware/adc.h"

struct trigger {
  uint pin;
  bool debounce;
  uint local_frequency;
  void (*poll)(); // use callback or return info struct from poll()?
  void (*callback)(uint period);
};

Trigger_t trigger_init(
  enum trigger_type type,
  uint_8t pin,
  uint_8t local_frequency,
  void (*callback)(uint period)
) {
  Trigger_t trig = malloc(sizeof(struct trigger));
  trig->pin = pin;
  trig->local_frequency = local_frequency;
  if (callback) {
    trig->callback = callback;
  }

  if (type == TRIGGER_COIL_ANALOG) {
    adc_gpio_init(pin);
    trig->poll = trigger_read_analog(trig);
  } else if (type == TRIGGER_COIL_DIGITAL) {
    trig->poll = trigger_read_digital(trig);
  }

  return trig;
}

bool trigger_read_analog(Trigger_t trig) {
  uint64_t millivolts = read_adc_channel(trig->pin - ADC_CHANNEL_OFFSET);
  if (millivolts  > 100 && !trig->debounce) {

  }
}

bool trigger_read_digital() {

}