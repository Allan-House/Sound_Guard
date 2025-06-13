#ifndef ADC_H
#define ADC_H

#include <stdint.h>

// ADC control functions
int adc_init(void);
int16_t adc_read_sample(int handle);
float adc_calculate_rms(int handle);
int16_t swap_bytes(int16_t val);

#endif // ADC_H