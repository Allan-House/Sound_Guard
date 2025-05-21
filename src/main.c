#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>

#define LED_GPIO 17             // GPIO 17 (pino físico 11), modo BCM usado pelo wiringPiSetupGpio()
#define ADS1115_ADDR 0x48       // Endereço padrão do ADS1115 no barramento I2C
#define CONVERSION_DELAY 4000   // 4ms para conversão a 250 SPS
#define NUM_SAMPLES 4           // Quantidade de amostragens
#define BAR_WIDTH 128           // Dimensão da barra visual
#define MAX_RMS 0.707f          // Nível RMS de referência para 0 dBFS
#define DC_OFFSET 1.25f         // Offset de tensão do MAX9814
#define MIN_NORMALIZED 0.001f   // Valor mínimo de escala log, evitando log(0)

// Configuração do ADS1115:
// - Bit 15: Iniciar conversão single-shot (1)
// - Bits 14-12: Canal AIN0 (100)
// - Bits 11-9: Ganho ±2.048V (010)
// - Bit 8: Modo single-shot (1)
// - Bits 7-5: Data rate 250 SPS (101)
// - Bits 4-0: Comparator disabled (00011)
#define ADS1115_CONFIG 0b1100010110100011

int16_t swap_bytes(int16_t val) {
    return (val << 8) | ((val >> 8) & 0xFF);   
}

int main() {
    // Inicializa a biblioteca WiringPi para usar numeração BCM (GPIO direto)
    if (wiringPiSetupGpio() < 0) {
        fprintf(stderr, "Erro ao inicializar WiringPi.\n");
        return -1;
    }
    
    // Abre a comunicação I2C com o ADS1115
    int handle = wiringPiI2CSetup(ADS1115_ADDR);
    if (handle < 0) {
        fprintf(stderr, "Erro ao abrir comunicação I2C com o ADS1115.\n");
        return 1;
    }

    printf("Iniciando leitura...\n");
    printf("Pressione Ctrl+C encerrar.\n");

    float dbfs_sum = 0.0f;
    float dbfs_avg = 0.0f;
    int count = 0;

    while (1) {
        float sumSquares = 0.0f;

        for (int i = 0; i < NUM_SAMPLES; i++) {
            // Envia o comando de configuração para o ADS1115 e espera a conversão
            wiringPiI2CWriteReg16(handle, 0x01, (ADS1115_CONFIG >> 8) | (ADS1115_CONFIG << 8));
            usleep(CONVERSION_DELAY);

            int16_t value = wiringPiI2CReadReg16(handle, 0x00);
            value = swap_bytes(value);

            // Calcula o valor da tensão do sinal
            float voltage = (value / 32768.0f) * 2.048f;

            // Remove o offset DC do MAX9814
            float ac_voltage = voltage - DC_OFFSET;

            // Soma os quadrados das tensões amostradas para cálculo do RMS
            sumSquares += ac_voltage * ac_voltage;

            // Teste
            // printf("Raw: %6d | V: %1.3f | AC: %1.3f\n", value, voltage, ac_voltage);
        }

        // Calcula o valor RMS do sinal
        float rms = sqrt(sumSquares / NUM_SAMPLES);
        
        // Normaliza o sinal com base no RMS de uma senóide de pico 1V (0 = silencioso, 1 = máximo) e a limita
        float normalized = rms / MAX_RMS;
        if (normalized > 1.0f) normalized = 1.0f;

        // Cálculo do valor dBFS do sinal RMS
        float dbfs = 20.0f * log10((rms > 0.0001f ? rms : 0.0001f) / MAX_RMS);
        
        // Calcula o comprimento da barra e a imprime com os valores de RMS e dBFS
        float norm_clamped = (normalized < MIN_NORMALIZED) ? MIN_NORMALIZED : normalized;
        int barLength = (int)((log10f(norm_clamped) - log10f(MIN_NORMALIZED)) / (0 - log10f(MIN_NORMALIZED)) * BAR_WIDTH);
        if (barLength < 0) barLength = 0;
        if (barLength > BAR_WIDTH) barLength = BAR_WIDTH;

        printf("Volume: ");
        for (int i = 0; i < BAR_WIDTH; i++) {
            if (i < barLength)
                printf("█");
            else
                printf(" ");
        }
        printf(" | RMS: %5.3f V | dBFS: %6.1f dB\n", rms, dbfs);
        
        fflush(stdout);
        
        dbfs_sum += dbfs;
        count++;

        if (!(count <= 31)) {
            // Calcula a média
            dbfs_avg = dbfs_sum / 32;
            dbfs_sum = 0;
            count = 0;
            //printf("Average dBFS: %6.1f\n", dbfs_avg);
        }

        usleep(16670); // atualização de 16.67ms ~60 FPS
    }

    return 0;
}