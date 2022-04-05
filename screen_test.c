/**
 * Copyright (c) 2022 Nova
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

const uint LED_PIN = PICO_DEFAULT_LED_PIN;
const uint SCREEN_BACKLIGHT_PIN = 0;

const uint IGN_TRIGGER_PIN = 26;
const uint IGN_TRIGGER_ADC_CHANNEL = 0;
const uint IGN_COIL_PIN = 22;

const uint IGN_COIL_PULSE_DURATION_MS = 1;
const float ADC_VOLTAGE_CONVERSION = 3.3f / (1 << 12);

// const uint PULSE_BUFFER_SIZE = 6

void init_io();
uint read_ign_trigger();
void start_coil_pulse();
int64_t end_coil_pulse_callback();

int main() {
  init_io();

  bool debounce = false;
	while (true) {
		// gpio_put(SCREEN_BACKLIGHT_PIN, 1);
		
    uint millivolts = read_ign_trigger();
    
    if (millivolts > 75 && !debounce) {
      start_coil_pulse();
      debounce = true;
    } else if (millivolts <= 50) {
      debounce = false;
    }

    //sleep_ms(2);
    // gpio_put(LED_PIN, 0);
		// gpio_put(SCREEN_BACKLIGHT_PIN, 0);
		// sleep_ms(250);
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

/*
 * Provide a short pulse to the ignition coil.
 * The coil will trigger on the leading edge of said pulse.
 * The pulse is triggered immediately and is ended by a timer
 */
void start_coil_pulse() {
  gpio_put(IGN_COIL_PIN, 1);
  gpio_put(LED_PIN, 1);
  add_alarm_in_ms(IGN_COIL_PULSE_DURATION_MS, end_coil_pulse_callback, NULL, true);
}

int64_t end_coil_pulse_callback() {
  gpio_put(IGN_COIL_PIN, 0);
  gpio_put(LED_PIN, 0);

  return 0;
}

// typedef struct Buffer {
//   uint size;
//   uint* contents;
// } Buffer;

// Buffer buffer_init(uint length) {
//   Buffer buf;
//   buf->size = length;
//   buf->contents = malloc(sizeof(uint) * length);
// }