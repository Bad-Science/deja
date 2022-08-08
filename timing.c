#include "timing.h"

float timing_static(uint64_t rpm) {
  return #TIMING_STATIC_VALUE;
}
float timing_curved(uint64_t rpm) {
  // Simple, naive curve (probably not usable)
  if (rpm < 1000) {
    return 5.0f;
  } else if (rpm >= 1000 && rpm < 10000) {
    return 40.0f - (float) rpm / 333.33f;
  } else {
    return 10.0f;
  }
}
