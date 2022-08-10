/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <pico/sync.h>
#include "state.h"

static State_t state;
static spin_lock_t* state_spin_lock;
static uint32_t state_spin_lock_save;

void state_init() {
  uint lock_num = next_striped_spin_lock_num();
  spin_lock_claim(lock_num);
  state_spin_lock = spin_lock_instance(lock_num);
  state->run = false;
  state->rpm = 0;
  state->last_tdc = 0;
  state->next_tdc = 0;
  state->airflow = 0;
  state->head_temp = 0;
}

inline State_t state_get() {
  state_spin_lock_save = spin_lock_blocking(state_spin_lock);
  State_t state_copy = state;
  spin_unlock(state_spin_lock, state_spin_lock_save);
  return state_copy;
}

// TODO: No need for pass by value for writing at this point
inline State_t state_begin_write() {
  state_spin_lock_save = spin_lock_blocking(state_spin_lock);
  return state;
}

inline bool state_commit_write(State_t* new_state) {
  state = *new_state;
  spin_unlock(state_spin_lock, state_spin_lock_save);
}

inline uint64_t state_offset_next_tdc_by_degrees(State_t* state, float degrees) {
  float period = state->next_tdc - state->last_tdc;
  float offset = degrees * (period / 360.f);
  return state->next_tdc - offset;
}
