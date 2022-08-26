/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <pico/stdlib.h>
#include "state.h"

#define SCHEDULER_MAX_ITEMS 4
#define EVENT_RELATIVE_TIME -1
#define EVENT_ABSOLUTE_TIME -2
#define EVENT_CANCEL 0
#define EVENT_NEXT_CYCLE 1
#define EVENT_THIS_CYCLE 2

typedef struct scheduler* Scheduler_t;
typedef struct scheduled_event scheduled_event_t;
typedef struct event event_t;
typedef struct engine_time engine_time_t;
typedef int8_t (*event_func_t)(event_t*);
typedef uint8_t event_id_t;

struct engine_time {
  float degrees;
  int64_t us;
};

struct scheduled_event {
  event_id_t id;
  uint64_t clock;
  event_t event;
}

struct event {
  event_func_t what;
  engine_time_t when;
  void* param;
};

Scheduler_t scheduler_init(uint8_t alarm_num);

event_id_t scheduler_add_event(Scheduler_t sched, event_t item);

event_t scheduler_event_init(
  event_func_t what,
  float degrees,
  int64_t us,
  void* param
);

#endif
