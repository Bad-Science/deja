/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <pico/time.h>
#include "scheduler.h"
#include "state.h"

struct scheduler {
  scheduleable_t items[SCHEDULER_MAX_ITEMS];
  uint8_t num_items;
  alarm_pool_t* alarm_pool;
};

static inline absolute_time_t to_absolute_time(engine_time_t* time, state_t* state) {
  absolute_time_t time;
  uint64_t value = state_offset_next_tdc_by_degrees(&state, item.when.degrees) + item.when.offset;
  update_us_since_boot(&time, value);
  return time;
}

static alarm_pool_t* scheduler_init_alarm_pool(uint8_t alarm_num) {
  //TODO
}

static inline alarm_id_t scheduler_add_alarm(scheduler_t* sched, scheduleable_t* item) {
  State_t state = get_state();
  absolute_time_t time = to_absolute_time(&(item.when), &state);
  return alarm_pool_add_alarm_at(sched->alarm_pool, time, scheduler_alarm_callback, sched_item, true);
}

static scheduler_alarm_callback(alarm_id_t id, void* data) {
  State_t state = get_state();
  absolute_time_t time = to_absolute_time(&(item.when), &state);
  scheduleable_t* item = data;

  if (item->what(item)) {
    return state_offset_next_tdc_by_degrees(&state, item.when.degrees) + item.when.offset;
  }
  return 0;
}

void scheduler_init(scheduler_t* sched, uint8_t alarm_num) {
  sched->alarm_pool = scheduler_init_alarm_pool(alarm_num);
  sched->num_items = 0;
}

scheduleable_t scheduler_item_init(
  schduleable_func_t perform,
  float degrees,
  uint64_t offset,
  void* param
) {
  scheduleable_t item;
  item.perform = perform;
  item.when.degrees = degrees;
  item.when.offset = offset;
  item.param = param;
  return item;
}

scheduleable_id_t scheduler_add_item(scheduler_t* sched, scheduleable_t item) {
  sched->items[sched->num_items] = item;
  scheduleable_t* sched_item = &(sched[sched->num_items]);
  sched_item->id = sched[num_items];

  scheduler_add_alarm(sched, sched_item);

  return sched->num_items++;
}





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
