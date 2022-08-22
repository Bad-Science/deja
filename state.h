/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef STATE_H
#define STATE_H

#include <pico/stdlib.h>

/**
 * Multicore-safe global state singleton.
 * 
 * The intended use case is for data collection routines on core0 to write to the state,
 * and for engine running routines on core1 to read from the state. Multicore-safety is
 * accomplished using a spin-lock. Writes block for the spin-lock and disable interrupts
 * (to avoid deadlocks when being called from the IRQ), so writes should be performed as
 * quickly as possible. Reads return the most recent complete copy of the state without
 * blocking, even if called during a write. This is accomplished by keeping a second
 * internal copy of the state. During a write, the inactive copy is first updated, then
 * the active index is updated. Because the index is a single byte, the resulting race
 * condition is benign. Care should be taken when using a copy for a long time if you
 * care about having the absolutely most current state.
 * 
 * Only mission-critical, small attributes should be added to the state because of the
 * copy-on-read nature. I guess. There's 256K of RAM, you be the judge.
 */

typedef struct state {
  bool run;
  uint16_t rpm;
  uint64_t last_tdc;
  uint64_t next_tdc;
  uint8_t airflow;
  uint8_t head_temp;
  float trigger_timing_offset; // TODO move to persistent config
  uint8_t trigger_frequency;   // TODO move to persistent config
} State_t;

/**
 * Initializes the state singleton with zero values
 */
void state_init();

/**
 * Returns a safe copy of the engine state.
 */
State_t state_get();

/**
 * Obtains a lock on the state and returns a safe copy for updating.
 * Please be fast, this disables interrupts for IRQ safety
 */
State_t state_begin_write();

/**
 * Atomically update the state from the write copy and release the lock.
 * To be used in conjuction with `state_begin_write()`.
 */
void state_commit_write(State_t*);

/**
 * Returns a timestamp in us equal to `degrees` before the next tdc as defined in `state`.
 * `degrees` should be positive for degrees BTDC
 */

static inline uint64_t state_offset_next_tdc_by_degrees(State_t* state, float degrees) {
  float period = state->next_tdc - state->last_tdc;
  float offset = degrees * (period / 360.f);
  return state->next_tdc - offset;
}

#endif
