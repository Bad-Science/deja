/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef TIMING_H
#define TIMING_H

#include <pico/stdlib.h>
#include "state.h"

#define TIMING_STATIC_VALUE 16.0f

typedef float (*timing_func_t)(State_t*);

float timing_static(State_t* state);
float timing_curved(State_t* state);

#endif
