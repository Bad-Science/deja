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
 * quickly as possible. Reads block if a write is currently being performed (can assume
 * the block will be extremely short or non-existant) and return a most recent copy of
 * the state. Care should be taken when using a copy for a long time if you care about
 * having the absolutely most current state.
 * 
 * Only mission-critical, small attributes should be added to the state because of the
 * copy-on-read nature. I guess. There's 256K of RAM, you be the judge.
 */

typedef struct state {
  bool run;
  uint64_t rpm;
  uint64_t last_tdc;
  uint64_t next_tdc;
  float airflow;
  float head_temp;
} State_t;

/**
 * Initializes the state singleton with zero values
 */
void state_init();

/**
 * Returns a copy of the engine state. Blocks if a write is being performed.
 */
inline State_t state_get();

/**
 * Obtains a lock on the state and returns a safe copy.
 * Please be fast, this blocks reads.
 */
inline State_t state_begin_write();

/**
 * To be used in conjuction with `state_begin_write()`.
 */
inline void state_commit_write(State_t*);

/**
 * Returns a timestamp in us equal to `degrees` before the next tdc as defined in `state`.
 * `degrees` should be positive for degrees BTDC
 */
inline uint64_t state_offset_next_tdc_by_degrees(State_t* state, float degrees);

#endif
