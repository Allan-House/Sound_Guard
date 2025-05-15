#include <stdio.h>
#include <pigpio.h>
#include <unistd.h>

#define LED_GPIO 17 // GPIO 17 (pino físico 11)
#define ADS1115_ADDR 0x48 // Endereço padrão do ADS1115
#define I2C_BUS 1 // I2C1 = GPIO 2 (SDA) e GPIO 3 (SCL)

int main() {
    
    if (gpioInitialise() < 0) {
        fprintf(stderr, "Erro ao inicilizalizar pigpiod.\n");
        return -1;
    }

    int handle = i2cOpen(I2C_BUS, ADS1115_ADDR, 0);
    if (handle < 0) {
        fprintf(stderr, "Erro ao abrir comunicação I2C com o ADS1115.\n");
        gpioTerminate();
        return 1;
    }

    
    while(1) {
        // Configurar o registrador de configuração (0x01)
        // Modo: Canal AIN0, ganho ±4.096V, single-shot, 128 SPS
        uint8_t config[3];
        config[0] = 0x01;  // Registrador de configuração
        config[1] = 0b11000010;  // MSB: Iniciar conversão, AIN0, FSR ±4.096V, single-shot
        config[2] = 0b11100011;  // LSB: 128 SPS, modo single-shot
    
        i2cWriteDevice(handle, (char*)config, 3);
    
        // Aguardar a conversão (~8 ms para 128 SPS)
        usleep(8000);

        // Ler o resultado (registrador 0x00)
        i2cWriteByte(handle, 0x00);  // Seleciona o registrador de conversão
        uint8_t result[2];
        i2cReadDevice(handle, (char*)result, 2);
    
        // Combinar os dois bytes lidos (big endian)
        int16_t value = (result[0] << 8) | result[1];
    
        // Converter para tensão (±4.096V = 2^15)
        float voltage = (value / 32768.0f) * 4.096f;
    
        printf("Valor bruto: %d\n", value);
        printf("Tensão: %.4f V\n", voltage);
        printf("--------------------\n");

        usleep(50000);
    }

    /*
    gpioSetMode(LED_GPIO, PI_OUTPUT);

    // Liga o LED por 2 segundos.
    printf("Acendendo LED...\n");
    gpioWrite(LED_GPIO, 1); 
    sleep(2);

    // Desliga o LED.
    printf("Apagando o LED.\n");
    gpioWrite(LED_GPIO, 0);
    */

    // Finaliza a biblioteca.
    gpioTerminate();
    return 0;
}