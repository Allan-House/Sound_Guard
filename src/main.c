#include <stdio.h>
#include <pigpio.h>
#include <unistd.h>

#define LED_GPIO 17 // GPIO 17 (pino físico 11)

int main() {
    
    if (gpioInitialise() < 0) {
        fprintf(stderr, "Erro ao inicilizalizar pigpiod.\n");
        return -1;
    }

    gpioSetMode(LED_GPIO, PI_OUTPUT);

    // Liga o LED por 2 segundos.
    printf("Acendendo LED...\n");
    gpioWrite(LED_GPIO, 1); 
    sleep(2);

    // Desliga o LED.
    printf("Apagando o LED.\n");
    gpioWrite(LED_GPIO, 0);

    // Finaliza a biblioteca.
    gpioTerminate();
    return 0;
}