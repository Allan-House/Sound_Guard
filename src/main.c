#include <stdio.h>
#include <stdint.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <unistd.h>

#define LED_GPIO 17       // GPIO 17 (pino físico 11), modo BCM usado pelo wiringPiSetupGpio()
#define ADS1115_ADDR 0x48 // Endereço padrão do ADS1115 no barramento I2C
#define CONVERSION_DELAY 8000 // 8ms para conversão a 128 SPS
#define LOOP_DELAY 50000 // 50ms entre leituras

// Configuração do ADS1115:
// - Bit 15: Iniciar conversão single-shot (1)
// - Bits 14-12: Canal AIN0 (100)
// - Bits 11-9: Ganho ±4.096V (010)
// - Bit 8: Modo single-shot (1)
// - Bits 7-5: Data rate 128 SPS (111)
// - Bits 4-0: Comparator disabled (00011)
#define ADS1115_CONFIG 0b1100001011100011

int main() {
    // Inicializa a biblioteca WiringPi para usar numeração BCM (GPIO direto)
    if (wiringPiSetupGpio() < 0) {
        fprintf(stderr, "Erro ao inicializar WiringPi.\n");
        return -1;
    }

    // Configura o pino do LED como saída (se for usar LED)
    // pinMode(LED_GPIO, OUTPUT);

    // Abre a comunicação I2C com o ADS1115
    int handle = wiringPiI2CSetup(ADS1115_ADDR);
    if (handle < 0) {
        fprintf(stderr, "Erro ao abrir comunicação I2C com o ADS1115.\n");
        return 1;
    }

    printf("Iniciando leitura do ADS1115...\n");
    printf("Configuração: 0x%04X\n", ADS1115_CONFIG);
    printf("Pressione Ctrl+C para sair.\n");

    while(1) {
        // Prepara e envia a configuração (com swap de bytes para big-endian)
        wiringPiI2CWriteReg16(handle, 0x01, (ADS1115_CONFIG >> 8) | (ADS1115_CONFIG << 8));
        
        // Aguarda a conversão
        usleep(CONVERSION_DELAY);

        // Lê e ajusta o valor (swap de bytes)
        int16_t value = wiringPiI2CReadReg16(handle, 0x00);
        value = ((value & 0xFF00) >> 8) | ((value & 0x00FF) << 8);

        // Converte para tensão
        float voltage = (value / 32768.0f) * 4.096f;

        // Exibe os resultados
        printf("Valor bruto: %6d | Tensão: %+7.4f V\n", value, voltage);
        fflush(stdout);

        // Intervalo entre leituras
        usleep(LOOP_DELAY);
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