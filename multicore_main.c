/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <pico/stdlib.h>
#include <pico/multicore.h>
#include "timing.h"
#include "ignition.h"
#include "trigger.h"
#include "state.h"
#include "helpers.h"

const uint IGN_MANUAL_TRIGGER_PIN = 16;
const uint TRIGGER_PIN = 26;
const uint IGN_COIL_PIN = 22;
const uint TIMING_COURSE_ADJUST_PIN = 28;
const uint TIMING_ADC_CHANNEL = 2;

const uint TRIGGERS_PER_REVOLUTION = 1;
const uint IGN_TIMING_LIGHT_PULSE_US = 0; // 100 to enable
const float IGN_DWELL_MS = 1.0f;

static void core0_main() {
  Trigger_t trigger = trigger_init(
    TRIGGER_TYPE_DIGITAL,
    TRIGGER_PIN,
    TRIGGERS_PER_REVOLUTION
  );

  trigger_go(trigger);
}

static void core1_main() {
  Ignition_t ignition = ignition_init(
    IGN_COIL_PIN,
    TIMING_LIGHT_PIN,
    IGN_DWELL_MS,
    IGN_TIMING_LIGHT_PULSE_US
  );

  ignition_go(ignition);
}

static void init() {

}

static int main() {
  state_init();

  multicore_launch_core1(core1_main);
}
