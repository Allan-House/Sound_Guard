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
#define BAR_WIDTH 20            // Dimensão da barra visual
#define MAX_RMS 0.707f          // Nível RMS de referência para 0 dBFS
#define DC_OFFSET 1.25f         // Offset de tensão do MAX9814

// Configuração do ADS1115:
// - Bit 15: Iniciar conversão single-shot (1)
// - Bits 14-12: Canal AIN0 (100)
// - Bits 11-9: Ganho ±4.096V (010)
// - Bit 8: Modo single-shot (1)
// - Bits 7-5: Data rate 250 SPS (101)
// - Bits 4-0: Comparator disabled (00011)
#define ADS1115_CONFIG 0b1100010110100011

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

    float smoothedRMS = 0.0f;

    while (1) {
        float sumSquares = 0.0f;

        for (int i = 0; i < NUM_SAMPLES; i++) {
            wiringPiI2CWriteReg16(handle, 0x01, (ADS1115_CONFIG >> 8) | (ADS1115_CONFIG << 8));
            usleep(CONVERSION_DELAY);

            int16_t value = wiringPiI2CReadReg16(handle, 0x00);
            value = ((value & 0xFF00) >> 8) | ((value & 0x00FF) << 8);

            float voltage = (value / 32768.0f) * 4.096f;
            float ac_voltage = voltage - DC_OFFSET;
            sumSquares += ac_voltage * ac_voltage;
        }

        float rms = sqrt(sumSquares / NUM_SAMPLES);

        float normalized = rms / MAX_RMS;
        if (normalized > 1.0f) normalized = 1.0f;

        float dbfs = 20.0f * log10((rms > 0.0001f ? rms : 0.0001f) / MAX_RMS);

        int barLength = (int)(normalized * BAR_WIDTH);

        printf("Volume: ");
        for (int i = 0; i < BAR_WIDTH; i++) {
            if (i < barLength)
                printf("#");
            else
                printf(" ");
        }
        printf(" | RMS: %5.3f V | dBFS: %6.1f dB\n", rms, dbfs);

        fflush(stdout);

        usleep(16670); // atualização de 16.67ms ~60 FPS
    }

    /*
    // Configura o pino do LED como saída
    pinMode(LED_GPIO, OUTPUT);

    // Liga o LED por 2 segundos
    printf("Acendendo LED...\n");
    digitalWrite(LED_GPIO, HIGH);
    sleep(2);

    // Desliga o LED
    printf("Apagando o LED.\n");
    digitalWrite(LED_GPIO, LOW);
    */

    return 0;
}