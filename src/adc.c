#include "adc.h"
#include "config.h"

#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <wiringPiI2C.h>

int16_t swap_bytes(int16_t value) {
    return (value << 8) | ((value >> 8) & 0xFF);   
}

int adc_init(void) {
    int handle = wiringPiI2CSetup(ADS1115_ADDR);
    if (handle < 0) {
        fprintf(stderr, "Erro ao abrir comunicação I2C com o ADS1115.\n");
        return -1;
    }
    return handle;
}

int16_t adc_read_sample(int handle) {
    wiringPiI2CWriteReg16(handle, 0x01, (ADS1115_CONFIG >> 8) | (ADS1115_CONFIG << 8));
    usleep(CONVERSION_DELAY);
    
    int16_t value = wiringPiI2CReadReg16(handle, 0x00);
    return swap_bytes(value);
}

float adc_calculate_rms(int handle) {
    const float voltage_scale = 2.048f / 32768.0f;
    const float samples_inv = 1.0f / NUM_SAMPLES;

    float sum_squares = 0.0f;

    for (int i = 0; i < NUM_SAMPLES; i++) {
        int16_t value = adc_read_sample(handle);

        float voltage = value * voltage_scale;
        float ac_voltage = voltage - DC_OFFSET;
        sum_squares += ac_voltage * ac_voltage;
        //printf("Raw: %6d | V: %1.3f | AC: %1.3f\n", value, voltage, ac_voltage);
    }

    return sqrtf(sum_squares * samples_inv);
}