# Random project notes

## Gotchas
- The RP2040 only has 4 alarms. When exactly does an alarm free?
- Low current mode. When using low current on the 3.3v supply, it enters
PFM mode (high noise). Set GPIO23 high to disable low current mode
and force PWM mode (low noise)
- Analog power & ground. These are lower noise than the main supply and
digital ground, and should be used and separated for good ADC performance.
Additionally, a 3.0V shunt (LM4040) can be tied to ADC_VREF
