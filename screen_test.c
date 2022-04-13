/**
 * Copyright (c) 2022 Nova
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

#include "buffer.h"

const uint LED_PIN = PICO_DEFAULT_LED_PIN;
const uint SCREEN_BACKLIGHT_PIN = 0;

const uint IGN_TRIGGER_PIN = 26;
const uint IGN_TRIGGER_ADC_CHANNEL = 0;
const uint IGN_COIL_PIN = 22;

const uint IGN_COIL_PULSE_DURATION_MS_SHORT = 1;
const uint IGN_COIL_PULSE_DURATION_MS = 2;
const uint IGN_COIL_PULSE_DURATION_MS_LONG = 3;

const float ADC_VOLTAGE_CONVERSION = 3.3f / (1 << 12);
const float DERIVITIVE_CONTROL_FACTOR = 30000.0f;

const uint PULSE_BUFFER_SIZE = 6;

void init_io();
uint read_ign_trigger();
void start_coil_pulse_in_us(uint64_t delay, uint* dur);
void start_coil_pulse_in_degrees(float degrees, uint64_t period, uint* dur);
int64_t start_coil_pulse(alarm_id_t id, void* dur);
int64_t end_coil_pulse_callback();
float get_timing(uint rpm);

int main() {
  init_io();

  Buffer pulse_time_delta_buffer = buffer_init(PULSE_BUFFER_SIZE);
  Buffer pulse_magnitude_buffer = buffer_init(PULSE_BUFFER_SIZE);

  bool debounce = false;
  bool advance_mode = false;
  bool running = false;
  uint64_t last_trigger_leading_edge_timestamp = 0; // Initial RPMs will be approximately zero
  uint last_rpm = 0;
  float stator_position = 16.0f;
  float timing_offset = 6.0f;

  gpio_put(IGN_COIL_PIN, 1);
  sleep_ms(1000);
  gpio_put(IGN_COIL_PIN, 0);

	while (true) {		
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
      float drpm_dt = (float) rpm_delta / (float) trigger_delta;
      float desired_ignition_time = get_timing(rpm) + timing_offset - drpm_dt * DERIVITIVE_CONTROL_FACTOR;
      
      /*
       * These conditionals handle entering and exiting "advance mode"
       *
       * The controller is said to be in "advance mode" when the desired timing precedes the stator timing
       * When entering advance mode, two sparks are scheduled: one at the present time (stator timing),
       * and one at the desired time for the next cycle.
       * Conversely, when exiting advance mode, the current spark is skipped, as one should be already scheduled
       * at approximately the right time by the previous advance mode cycle.
       */

      if (rpm > 60 && last_rpm > 60) {
        if (desired_ignition_time <= stator_position || !advance_mode) {
          if (!advance_mode) {
            float ignite_in_degrees = MAX(0, stator_position - desired_ignition_time);
            start_coil_pulse_in_degrees(ignite_in_degrees, trigger_delta, &IGN_COIL_PULSE_DURATION_MS);
          }
          advance_mode = false;
        }

        if (desired_ignition_time > stator_position) {
          float ignite_in_degrees = 360.0f - (desired_ignition_time - stator_position);
          start_coil_pulse_in_degrees(ignite_in_degrees, trigger_delta, &IGN_COIL_PULSE_DURATION_MS_LONG);
          advance_mode = true;
        }
      } else {
        start_coil_pulse(NULL, &IGN_COIL_PULSE_DURATION_MS_SHORT);
        advance_mode = false;
      }

      last_trigger_leading_edge_timestamp = trigger_leading_edge_timestamp;
      last_rpm = rpm;
      debounce = true;
    } else if (millivolts <= 75) {
      debounce = false;
      // TODO: use falling edge to update rpm and deltas
    }
	}
}

void init_io() {
  // Init stdio
  stdio_init_all();

	// Init screen and LED
	gpio_init(LED_PIN);
	gpio_set_dir(LED_PIN, GPIO_OUT);

  gpio_init(SCREEN_BACKLIGHT_PIN);
	gpio_set_dir(SCREEN_BACKLIGHT_PIN, GPIO_OUT);

  gpio_init(IGN_COIL_PIN);
  gpio_set_dir(IGN_COIL_PIN, GPIO_OUT);

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

void start_coil_pulse_in_degrees(float degrees, uint64_t period, uint* dur) {
  uint64_t ignite_in_us = (uint64_t) ((degrees / 360.0f) * period);
  start_coil_pulse_in_us(ignite_in_us, dur);
}

void start_coil_pulse_in_us(uint64_t delay, uint* dur) {
  add_alarm_in_us(delay, start_coil_pulse, dur, true);
}

/*
 * Provide a short pulse to the ignition coil.
 * The coil will trigger on the leading edge of said pulse.
 * The pulse is triggered immediately and is ended by a timer.
 * This function may alse serve as a timer callback itself.
 */
int64_t start_coil_pulse(alarm_id_t id, void* dur) {
  gpio_put(IGN_COIL_PIN, 1);
  gpio_put(LED_PIN, 1);
  uint duration = *((uint*) dur);
  add_alarm_in_ms(duration, end_coil_pulse_callback, NULL, true);

  return 0;
}

int64_t end_coil_pulse_callback() {
  gpio_put(IGN_COIL_PIN, 0);
  gpio_put(LED_PIN, 0);

  return 0;
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