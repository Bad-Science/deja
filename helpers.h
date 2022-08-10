/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef HELPERS_H
#define HELPERS_H

#include <pico/stdlib.h>
#include <hardware/adc.h>

const float ADC_VOLTAGE_CONVERSION = 3.3f / (1 << 12);
const uint8_t ADC_CHANNEL_OFFSET = 26;

uint read_adc_channel(uint adc_channel) {
  adc_select_input(adc_channel);
  uint16_t adc_result = adc_read();
  uint64_t adc_result_millivolts = (adc_result * ADC_VOLTAGE_CONVERSION * 1000);
  //printf("Raw value: 0x%03x, voltage: %f V\n, ", adc_result, adc_result * ADC_VOLTAGE_CONVERSION);

  return adc_result_millivolts;
}

#endif
