#ifndef IGNITION_H
#define IGNITION_H

#include "pico/stdlib.h"
#include "hardware/gpio.h"

typedef struct ignition {
  uint coil_pin;
  uint indicator_pin;
  float dwell_ms;
  uint timing_light_pulse_us;
}* Ignition_t;

/**
 * Construct a new Ignition_t 
 */
Ignition_t ignition_init(
  uint coil_pin,
  uint indicator_pin,
  float dwell_ms,
  uint timing_light_pulse_us
);

void ignition_init_io(Ignition_t ign);

/**
 * Using a timer, schedule the spark to occur in a number of degrees relative to the period.
 * The scheduled time is offset to correct for dwell.
 */
void ignition_schedule_spark_in_degrees(Ignition_t ign, float degrees, uint64_t period);

/**
 * Using a timer, schedule the dwell to begin after the specified delay
 */
void ignition_schedule_dwell_in_us(Ignition_t ign, uint64_t delay);

/*
 * Provide a short pulse to the ignition coil.
 * The dwell will begin on the leading edge of said pulse.
 * The spark is then triggered by the falling edge of the pulse.
 * This function may alse serve as a timer callback itself.
 */
int64_t ignition_start_dwell(alarm_id_t id, void* ign);

/*
 * End the pulse to the ignition coil. This ends the dwell period and triggers the spark.
 * If the timing light is enabled, the light is turned on and scheduled to be turned off.
 */
int64_t ignition_end_dwell(alarm_id_t id, void* ign);

/**
 * End the pulse to the timing light
 */
int64_t ignition_end_timing_light(alarm_id_t, void* ign);

#endif
