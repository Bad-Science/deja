/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <pico/time.h>
#include "scheduler.h"
#include "state.h"

struct scheduler {
  event_t items[SCHEDULER_MAX_ITEMS];
  uint8_t num_items;
  alarm_pool_t* alarm_pool;
};

static inline absolute_time_t event_to_absolute_time(engine_time_t* time, State_t* state) {
  absolute_time_t abs_time;
  uint64_t value = event_to_us_since_boot(time, state);
  update_us_since_boot(&abs_time, value);
  return abs_time;
}

static int64_t scheduler_alarm_callback(alarm_id_t id, void* data) {
  event_t* item = data;
  if (item->what(data)) {
    State_t state = state_get();
    return event_to_us_since_boot(&(item->when), &state);
  }
  // If callback returns false, do not repeat
  return 0;
}

static alarm_id_t scheduler_add_alarm(Scheduler_t sched, event_t* item) {
  State_t state = state_get();
  absolute_time_t time = event_to_absolute_time(&(item->when), &state);
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
  float degrees,
  int64_t us,
  void* param
) {
  event_t item;
  item.what = what;
  item.when.degrees = degrees;
  item.when.us = us;
  item.param = param;
  return item;
}

event_id_t scheduler_add_event(Scheduler_t sched, event_t item) {
  sched->items[sched->num_items] = item;
  event_t* sched_item = &(sched->items[sched->num_items]);
  sched_item->id = sched->num_items;

  scheduler_add_alarm(sched, sched_item);

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