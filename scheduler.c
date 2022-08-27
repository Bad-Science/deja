/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <pico/stdlib.h>
#include <pico/time.h>
#include "scheduler.h"
#include "state.h"

struct scheduler {
  scheduled_event_t items[SCHEDULER_MAX_ITEMS];
  uint8_t num_items;
  alarm_pool_t* alarm_pool;
};

static inline uint64_t event_to_us_since_boot(scheduled_event_t* item, State_t* state) {
  uint64_t next_time;

  // Cancel event
  if (item->event.mode == EVENT_CANCEL) {
    // TODO: Remove the thing
    next_time = 0;
  }

  // Relative time mode
  if (item->event.mode == RELATIVE_US) {
    next_time = time_us_64() + item->event.when.us;
    item->clock = state->clock;
  }

  // Absolute time mode
  if (item->event.mode == ABSOLUTE_US) {
    next_time = item->event.when.us;
    item->clock = state->clock;
  }

  // Degree mode - Schedule for current cycle (next tdc)
  if (item->event.mode == SAME_CYCLE && item->clock == state->clock) {
    next_time = state->next_tdc - item->event.when.degrees * state->physical_period / 360.f;
  }

  // Degree mode - Schedule for next cycle (next tdc)
  if (item->event.mode == NEXT_CYCLE && item->clock == state->clock - 1) {
    next_time = state->next_tdc - item->event.when.degrees * state->physical_period / 360.f;
    item->clock = state->clock;
  }

  // Degree mode - Schedule for current cycle (previous tdc)
  if (item->event.mode == SAME_CYCLE && item->clock == state->clock - 1) {
    next_time = state->next_tdc - state->ignition_period - item->event.when.degrees * state->physical_period / 360.f;
  }

  // Degree mode - Schedule for next cycle (previous tdc)
  if (next_time == NEXT_CYCLE && item->clock == state->clock) {
    next_time = state->next_tdc + state->ignition_period - item->event.when.degrees * state->physical_period / 360.f;
    item->clock = state->clock;
  }

  return next_time;
}

static inline absolute_time_t event_to_absolute_time(scheduled_event_t* item, State_t* state) {
  absolute_time_t abs_time;
  uint64_t value = event_to_us_since_boot(item, state);
  update_us_since_boot(&abs_time, value);
  return abs_time;
}

static int64_t scheduler_alarm_callback(alarm_id_t id, void* data) {
  scheduled_event_t* item = data;
  //uint64_t previous_clock = item->clock;
  item->event.what(&(item->event));
  State_t state = state_get();

  return event_to_us_since_boot(item, &state);
}

static alarm_id_t scheduler_add_alarm(Scheduler_t sched, scheduled_event_t* item) {
  State_t state = state_get();
  
  absolute_time_t time = event_to_absolute_time(item, &state);
  return alarm_pool_add_alarm_at(sched->alarm_pool, time, scheduler_alarm_callback, item, true);
}

Scheduler_t scheduler_init(uint8_t alarm_num) {
  Scheduler_t sched = malloc(sizeof(struct scheduler));
  sched->alarm_pool = alarm_pool_create(alarm_num, SCHEDULER_MAX_ITEMS);
  sched->num_items = 0;

  return sched;
}

event_t scheduler_event_init(
  event_func_t what,
  schedule_mode_t mode,
  float degrees,
  int64_t us,
  void* param
) {
  event_t item;
  item.what = what;
  item.mode = mode;
  item.when.degrees = degrees;
  item.when.us = us;
  item.param = param;
  return item;
}

event_id_t scheduler_add_event(Scheduler_t sched, event_t event) {
  scheduled_event_t* sched_event = &(sched->items[sched->num_items]);
  State_t state = state_get();

  sched_event->event = event;
  sched_event->clock = state.clock;
  sched_event->id = sched->num_items;

  scheduler_add_alarm(sched, sched_event);

  return sched->num_items++;
}




/*
static uint64_t get_time() {
  uint32_t lo = timer_hw->timerlr;
  uint32_t hi = timer_hw->timerhr;
  return ((uint64_t) hi << 32u) | lo;
}

static void scheduler_irq_handler() {

}

static void scheduler_in_us(uint8_t alarm_num, uint32_t delay_us, ) {
  // Enable the interrupt for our alarm (the timer outputs 4 alarm irqs)
  hw_set_bits(&timer_hw->inte, 1u << alarm_num);
  // Set irq handler for alarm irq
  irq_set_exclusive_handler(alarm_num, alarm_irq);
  // Enable the alarm irq
  irq_set_enabled(ALARM_IRQ, true);
  // Enable interrupt in block and at processor

  // Alarm is only 32 bits so if trying to delay more
  // than that need to be careful and keep track of the upper
  // bits
  uint64_t target = timer_hw->timerawl + delay_us;

  // Write the lower 32 bits of the target time to the alarm which
  // will arm it
  timer_hw->alarm[ALARM_NUM] = (uint32_t) target;
}
*/