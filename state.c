/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <pico/sync.h>
#include "state.h"

typedef struct state_listener {
  state_listener_func_t update;
  void* param;
} state_listener_t;

static State_t state[2] = { { 0 }, { 0 } };
static bool state_index;
static spin_lock_t* state_spin_lock;
static uint32_t state_spin_lock_save;
static state_listener_t state_listeners[STATE_MAX_LISTENERS] = { 0 };
static uint8_t state_num_listeners = 0;

void state_init() {
  uint lock_num = next_striped_spin_lock_num();
  spin_lock_claim(lock_num);
  state_spin_lock = spin_lock_instance(lock_num);
  state_index = 0;
}

State_t state_get() {
  return state[state_index];
}

// TODO: Consider copying state[index] to state[!index] here and returning a pointer
State_t state_begin_write() {
  state_spin_lock_save = spin_lock_blocking(state_spin_lock);
  return state_get();
}

void state_commit_write(State_t* new_state) {
  state[!state_index] = *new_state;
  // By swapping the state index, we make the update atomic from the reader's
  // perspective. This makes the race condition benign at the expense of doubling
  // the memory used to store the state, to the end of making reads non-blocking
  state_index = !state_index;
  spin_unlock(state_spin_lock, state_spin_lock_save);

}
