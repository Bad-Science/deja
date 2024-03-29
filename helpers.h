/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef HELPERS_H
#define HELPERS_H

#include <pico/stdlib.h>
#include <hardware/adc.h>

static const float ADC_VOLTAGE_CONVERSION = 3.3f / (1 << 12);
static const uint8_t ADC_CHANNEL_OFFSET = 26;

static inline uint64_t read_adc_channel(uint adc_channel) {
  adc_select_input(adc_channel);
  uint16_t adc_result = adc_read();
  uint64_t adc_result_millivolts = (adc_result * ADC_VOLTAGE_CONVERSION * 1000);
  //printf("Raw value: 0x%03x, voltage: %f V\n, ", adc_result, adc_result * ADC_VOLTAGE_CONVERSION);

  return adc_result_millivolts;
}

static inline absolute_time_t to_absolute_time_t(uint64_t time) {
  absolute_time_t abs_time;
  update_us_since_boot(&abs_time, time);
  return abs_time;
}

#endif
