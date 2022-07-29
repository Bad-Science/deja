#ifndef HELPERS_H
#define HELPERS_H

const float ADC_VOLTAGE_CONVERSION = 3.3f / (1 << 12);
const uint8_t ADC_CHANNEL_OFFSET = 26;

uint read_adc_channel(uint adc_channel) {
  adc_select_input(adc_channel);
  uint16_t adc_result = adc_read();
  uint adc_result_millivolts = (uint) (adc_result * ADC_VOLTAGE_CONVERSION * 1000);
  //printf("Raw value: 0x%03x, voltage: %f V\n, ", adc_result, adc_result * ADC_VOLTAGE_CONVERSION);

  return adc_result_millivolts;
}

#endif