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
typedef bool (*schduleable_func_t)(scheduleable_t*);
typedef uint8_t scheduleable_id_t;

typedef struct scheduleable {
  schduleable_func_t what;
  engine_time_t when;
  void* param;
  scheduleable_id_t id;
} scheduleable_t;


void scheduler_init(scheduler_t*, uint8_t alarm_num);
scheduleable_id_t scheduler_add_item(scheduler_t* sched, scheduleable_t item);
scheduleable_t scheduler_make_item(
  schduleable_func_t what,
  float degrees,
  int64_t offset,
  void* param
);

#endif
