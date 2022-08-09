/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef STATE_H
#define STATE_H

#include <pico/stdlib.h>

typedef struct state {
  bool run;
  uint64_t rpm;
  uint64_t last_tdc;
  uint64_t next_tdc;
  float airflow;
  float head_temp;
} State_t;

void state_init();
inline State_t state_get();
inline State_t state_begin_write();
inline void state_commit_write(State_t*);

/**
 * Returns a timestamp in us equal to `degrees` before the next tdc as defined in `state`.
 * `degrees` should be positive for degrees BTDC
 */
uint64_t state_offset_next_tdc_by_degrees(State_t* state, float degrees);

#endif
