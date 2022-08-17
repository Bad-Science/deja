/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "timing.h"

float timing_static(State_t* state) {
  return TIMING_STATIC_VALUE;
}
float timing_curved(State_t* state) {
  // Simple, naive curve (probably not usable)
  if (state->rpm < 1000) {
    return 5.0f;
  } else if (state->rpm >= 1000 && state->rpm < 10000) {
    return 40.0f - (float) state->rpm / 333.33f;
  } else {
    return 10.0f;
  }
}
