#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <wiringPiI2C.h>

#include "adc.h"
#include "config.h"

int16_t swap_bytes(int16_t val) {
    return (val << 8) | ((val >> 8) & 0xFF);   
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
    // Envia o comando de configuração para o ADS1115 e espera a conversão
    wiringPiI2CWriteReg16(handle, 0x01, (ADS1115_CONFIG >> 8) | (ADS1115_CONFIG << 8));
    usleep(CONVERSION_DELAY);

    int16_t value = wiringPiI2CReadReg16(handle, 0x00);
    return swap_bytes(value);
}

float adc_calculate_rms(int handle) {
    // Pré-calculando constantes para otimização
    const float voltage_scale = 2.048f / 32768.0f;
    const float samples_inv = 1.0f / NUM_SAMPLES;
    
    float sumSquares = 0.0f;

    for (int i = 0; i < NUM_SAMPLES; i++) {
        int16_t value = adc_read_sample(handle);
        
        // Calcula o valor da tensão do sinal
        float voltage = value * voltage_scale;
        // Remove o offset DC do MAX9814
        float ac_voltage = voltage - DC_OFFSET;
        // Soma os quadrados das tensões amostradas para cálculo do RMS
        sumSquares += ac_voltage * ac_voltage;

        // Teste (comentado)
        // printf("Raw: %6d | V: %1.3f | AC: %1.3f\n", value, voltage, ac_voltage);
    }

    // Calcula o valor RMS do sinal
    return sqrtf(sumSquares * samples_inv);
}