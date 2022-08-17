/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <pico/stdlib.h>

#define SCHEDULER_MAX_ITEMS 4;

typedef struct engine_time {
  float degrees;
  int64_t offset;
} engine_time_t;

typedef struct scheduler scheduler_t;
typedef bool (*event_func_t)(event_t*);
typedef uint8_t event_id_t;

typedef struct event {
  event_func_t what;
  engine_time_t when;
  void* param;
  event_id_t id;
} event_t;

void scheduler_init(scheduler_t*, uint8_t alarm_num);

event_id_t scheduler_add_item(scheduler_t* sched, event_t item);

event_t scheduler_make_item(
  event_func_t what,
  float degrees,
  int64_t offset,
  void* param
);

#endif
