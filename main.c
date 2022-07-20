/**
 * Copyright (c) 2022 Nova
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

#include "ignition.h"
#include "buffer.h"

const uint TIMING_LIGHT_PIN = 15;
const uint SCREEN_BACKLIGHT_PIN = 0;

const uint IGN_MANUAL_TRIGGER_PIN = 16;
const uint IGN_TRIGGER_PIN = 26;
const uint IGN_TRIGGER_ADC_CHANNEL = 0;
const uint IGN_COIL_PIN = 22;

const uint IGN_MANUAL_DEBOUNCE_MS = 500;
const uint IGN_TIMING_LIGHT_PULSE_US = 1000;
const float IGN_DWELL_MS = 1.0f;

const float ADC_VOLTAGE_CONVERSION = 3.3f / (1 << 12);

const uint PULSE_BUFFER_SIZE = 6;

void init_io();
uint read_ign_trigger();
float get_timing(uint rpm);
bool read_ign_manual_trigger();
void manual_trigger(Ignition_t ignition, uint debounce);

int main() {
  init_io();

  Ignition_t ignition = ignition_init(
    IGN_COIL_PIN,
    TIMING_LIGHT_PIN,
    IGN_DWELL_MS,
    IGN_TIMING_LIGHT_PULSE_US
  );

  Buffer pulse_time_delta_buffer = buffer_init(PULSE_BUFFER_SIZE);
  Buffer pulse_magnitude_buffer = buffer_init(PULSE_BUFFER_SIZE);

  bool debounce = false;
  bool advance_mode = false;
  uint64_t last_trigger_leading_edge_timestamp = 0; // Initial RPMs will be approximately zero
  uint last_rpm = 0;
  float stator_position = 16.0f;
  float timing_offset = 6.0f;

  while (true) {
    // Manual trigger / kill switch.
    if (read_ign_manual_trigger()) {
      manual_trigger(ignition, IGN_MANUAL_DEBOUNCE_MS);
    }

    // TODO: We really should just be pulling a value from the FIFO if it exists so we don't tie up the cpu...
    uint millivolts = read_ign_trigger();

    buffer_unshift(pulse_magnitude_buffer, millivolts);
    uint magnitude_average_head = buffer_average_head(pulse_magnitude_buffer);
    uint magnitude_average_tail = buffer_average_tail(pulse_magnitude_buffer);
    //if (magnitude_average_head < 10 && magnitude_average_tail >= 10) { /*...*/ }
    
    if (millivolts > 100 && !debounce) {
      uint64_t trigger_leading_edge_timestamp = to_us_since_boot(get_absolute_time());
      uint64_t trigger_delta = trigger_leading_edge_timestamp - last_trigger_leading_edge_timestamp;
      uint rpm = 6E7 / trigger_delta;
      int rpm_delta = rpm - last_rpm;
      float dwell_degrees = (ignition->dwell_ms * 1000.0f) * (360.0f / trigger_delta);
      // Desired spark timing in degrees before TDC
      float desired_ignition_time = get_timing(rpm) + timing_offset;

      /*
       * These conditionals handle entering and exiting "advance mode"
       *
       * The controller is said to be in "advance mode" when the desired timing precedes the stator timing
       * When entering advance mode, two sparks are scheduled: one at the present time (stator timing),
       * and one at the desired time for the next cycle.
       * Conversely, when exiting advance mode, the current spark is skipped, as one should be already scheduled
       * at approximately the right time by the previous advance mode cycle.
       */

      if (rpm > 60) {
        if (desired_ignition_time <= stator_position || !advance_mode) {
          if (!advance_mode) {
            float ignite_in_degrees = MAX(0, stator_position - desired_ignition_time);
            ignition_schedule_spark_in_degrees(ignition, ignite_in_degrees, trigger_delta);
          }
          advance_mode = false;
        }

        if (desired_ignition_time > stator_position) {
          float ignite_in_degrees = 360.0f - (desired_ignition_time - stator_position);
          ignition_schedule_spark_in_degrees(ignition, ignite_in_degrees, trigger_delta);
          advance_mode = true;
        }
      } else {
        ignition_schedule_dwell_in_us(ignition, 0);
        advance_mode = false;
      }

      last_trigger_leading_edge_timestamp = trigger_leading_edge_timestamp;
      last_rpm = rpm;
      debounce = true;
    } else if (millivolts <= 75) {
      debounce = false;
    }
	}
}

void test() {
  
}

bool read_ign_manual_trigger() {
  return gpio_get(IGN_MANUAL_TRIGGER_PIN);
}

/**
 * Test procedure to manually trigger an ignition pulse.
 * This function blocks for `manual_trigger_debounce` milliseconds.
 */
void manual_trigger(Ignition_t ignition, uint debounce) {
  ignition_schedule_dwell_in_us(ignition, 0);
  sleep_ms(debounce);
}

void init_io() {
  // Init stdio
  stdio_init_all();

	// Init screen
  gpio_init(SCREEN_BACKLIGHT_PIN);
	gpio_set_dir(SCREEN_BACKLIGHT_PIN, GPIO_OUT);

  // Init manual trigger
  gpio_init(IGN_MANUAL_TRIGGER_PIN);
  gpio_set_dir(IGN_MANUAL_TRIGGER_PIN, GPIO_OUT);

	// Init coil input
  adc_init();
  
  adc_gpio_init(IGN_TRIGGER_PIN);
}

uint read_ign_trigger() {
  adc_select_input(IGN_TRIGGER_ADC_CHANNEL);
  uint16_t adc_result = adc_read();
  uint adc_result_millivolts = (uint) (adc_result * ADC_VOLTAGE_CONVERSION * 1000);
  //printf("Raw value: 0x%03x, voltage: %f V\n, ", adc_result, adc_result * ADC_VOLTAGE_CONVERSION);

  return adc_result_millivolts;
}



/*
 * Returns correct timing as a function of RPMs
 * Result is in absolute degrees before top dead center
 */
float get_timing(uint rpm) {
  // flattttttt
  return 16.0f;
  // Simple, naive curve (probably not usable)
  if (rpm < 1000) {
    return 5.0f;
  } else if (rpm >= 1000 && rpm < 10000) {
    return 40.0f - (float) rpm / 333.33f;
  } else {
    return 10.0f;
  }
}

// 5Hz   - 9.2ms after trigger,  200ms period, 
// 10Hz  - 4.6ms after trigger,  100ms period, 4.6% = 16.5def after triffer  = -0.5deg BTDC = -5.5 error 
// 20Hz  - 2.0ms before trigger, 50ms period,  4.0% = 14.4deg before trigger = 30.4deg BTDC = -6.0 error
//                                                              target = 36.4deg BTDC
// 50Hz  - 0.5ms before trigger, 20ms period,  2.5% = 9.0deg before trigger  = 25.0deg BTDC = -6.0 error
// 100Hz - 0.0ms before trigger, 10ms period,  0.0% = 0.0deg before trigger  = 16.0deg BTDC = -6.0 error
// 150Hz - .175ms after trigger. 6.7ms period, 2.6% = 9.45deg after trigger  = 6.55deg BTDC = -6.45 error
// 200Hz - .175ms after trigger, 5ms period,  3.5% = 12.6deg after trigger  =  3.4deg BTDC = -6.6 error