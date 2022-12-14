/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef STATE_H
#define STATE_H

#include <pico/stdlib.h>

#define STATE_MAX_LISTENERS 4

/** Use to get RPMs from Period */
#define RPM(period) (6E7 / (period))

typedef struct state State_t;
typedef void (*state_listener_func_t)(State_t*);
typedef uint64_t engine_clock_t;

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
struct state {
  /** Engine is physically moving */
  bool running;

  /** Next top dead center. May or may not be a "waste spark" */
  uint64_t next_tdc;

  /** Running degree clock value of next_tdc */
  engine_clock_t clock;

  /** Physical engine rotation period. Accounts for "waste" TDCs if there are any */
  uint32_t physical_period;

  /** Time between ignition cycles. Equal to state.period / local trigger frequency */
  uint32_t ignition_period;

  /** Airflow into the intake */
  uint8_t airflow;

  /** Head temperature in degrees celcius */
  uint8_t head_temp;

  /** User set trigger offset in degrees */
  float trigger_timing_offset; // TODO move to persistent config
};

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

/** Neat lil state helper babies UwU */

/** Updates state running status */
static inline void state_set_running(State_t* state, bool running) {
  if (!running) {
    state->clock = 0;
  }
  state->running = running;
}

#endif
