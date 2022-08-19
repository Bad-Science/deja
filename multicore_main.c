/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <pico/stdlib.h>
#include <pico/multicore.h>
#include "scheduler.h"
#include "timing.h"
#include "ignition.h"
#include "trigger.h"
#include "state.h"
#include "helpers.h"

#define IGN_MANUAL_TRIGGER_PIN 16
#define TRIGGER_PIN 26
#define IGN_COIL_PIN 22
#define IGN_TIMING_LIGHT_PIN 15
#define TIMING_COURSE_ADJUST_PIN = 28
#define TIMING_ADC_CHANNEL 2

#define TRIGGERS_PER_REVOLUTION 1
#define IGN_TIMING_LIGHT_PULSE_US 0 // 100 to enable
#define IGN_DWELL_US 1000

#define SCHEDULER_0_ALARM 1
#define SCHEDULER_1_ALARM 2

static void core0_main() {
  Scheduler_t scheduler = scheduler_init(SCHEDULER_0_ALARM);

  Trigger_t trigger = trigger_init(
    TRIGGER_COIL_DIGITAL,
    TRIGGER_PIN,
    TRIGGERS_PER_REVOLUTION,
    0
  );

  event_t trigger_event = scheduler_event_init(trigger_event_callback, EVENT_RELATIVE_TIME, TRIGGER_POLL_PERIOD, trigger);
  scheduler_add_event(scheduler, trigger_event);

  tight_loop_contents();
}

static void core1_main() {
  Scheduler_t scheduler = scheduler_init(SCHEDULER_1_ALARM);

  Ignition_t ignition = ignition_init(
    IGN_COIL_PIN,
    IGN_TIMING_LIGHT_PIN,
    IGN_DWELL_US,
    IGN_TIMING_LIGHT_PULSE_US,
    timing_static
  );

  event_t ignition_event = scheduler_event_init(ignition_event_callback, EVENT_RELATIVE_TIME, 0, ignition);
  scheduler_add_event(scheduler, ignition_event);

  tight_loop_contents();
}

static void init() {

}

int main() {
  state_init();

  core0_main();
  multicore_launch_core1(core1_main);

  return 0;
}
