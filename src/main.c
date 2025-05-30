#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>

#define LED_GPIO 17             // GPIO 17 (pino físico 11), modo BCM usado pelo wiringPiSetupGpio()
#define ADS1115_ADDR 0x48       // Endereço padrão do ADS1115 no barramento I2C
#define CONVERSION_DELAY 4000   // 4ms para conversão a 250 SPS
#define NUM_SAMPLES 4           // Quantidade de amostragens
#define BAR_WIDTH 80            // Dimensão da barra visual
#define MAX_RMS 0.707f          // Nível RMS de referência para 0 dBFS
#define DC_OFFSET 1.25f         // Offset de tensão do MAX9814
#define MIN_NORMALIZED 0.001f   // Valor mínimo de escala log, evitando log(0)
#define TARGET_INTERVAL_NS 33330000  // Intervalo de tempo de ~33.33ms em nanosegundos (30 FPS)

#define LCD_I2C_ADDR 0x27       // Endereço padrão do módulo I2C (PCF8574)
#define LCD_BACKLIGHT 0x08
#define LCD_ENABLE    0x04
#define LCD_CMD       0
#define LCD_CHR       1

// Configuração do ADS1115:
// - Bit 15: Iniciar conversão single-shot (1)
// - Bits 14-12: Canal AIN0 (100)
// - Bits 11-9: Ganho ±2.048V (010)
// - Bit 8: Modo single-shot (1)
// - Bits 7-5: Data rate 250 SPS (101)
// - Bits 4-0: Comparator disabled (00011)
#define ADS1115_CONFIG 0b1100010110100011

volatile int keepRunning = 1;
int lcd_fd;

void lcd_toggle_enable(uint8_t bits) {
    wiringPiI2CWrite(lcd_fd, bits | LCD_ENABLE | LCD_BACKLIGHT);
    usleep(500);
    wiringPiI2CWrite(lcd_fd, (bits & ~LCD_ENABLE) | LCD_BACKLIGHT);
    usleep(100);
}

void lcd_send_byte(uint8_t bits, int mode) {
    uint8_t high = mode | (bits & 0xF0) | LCD_BACKLIGHT;
    uint8_t low  = mode | ((bits << 4) & 0xF0) | LCD_BACKLIGHT;
    wiringPiI2CWrite(lcd_fd, high);
    lcd_toggle_enable(high);
    wiringPiI2CWrite(lcd_fd, low);
    lcd_toggle_enable(low);
}

void lcd_init() {
    lcd_fd = wiringPiI2CSetup(LCD_I2C_ADDR);
    if (lcd_fd < 0) {
        fprintf(stderr, "Erro ao inicializar LCD I2C.\n");
        exit(EXIT_FAILURE);
    }

    usleep(50000);                // Espera >40ms após VCC
    lcd_send_byte(0x33, LCD_CMD); // Inicialização
    lcd_send_byte(0x32, LCD_CMD); // Define para 4 bits
    lcd_send_byte(0x28, LCD_CMD); // 2 linhas, 5x8 matriz
    lcd_send_byte(0x0C, LCD_CMD); // Display on, cursor off
    lcd_send_byte(0x06, LCD_CMD); // Incrementa cursor
    lcd_send_byte(0x01, LCD_CMD); // Limpa display
    usleep(2000);
}

void lcd_write(const char *line1, const char *line2) {
    lcd_send_byte(0x80, LCD_CMD);  // Linha 1
    for (int i = 0; i < 16 && line1[i]; i++)
        lcd_send_byte(line1[i], LCD_CHR);

    lcd_send_byte(0xC0, LCD_CMD);  // Linha 2
    for (int i = 0; i < 16 && line2[i]; i++)
        lcd_send_byte(line2[i], LCD_CHR);
}

// Handler de sinal pra terminação limpa (Ctrl + C)
void intHandler(int dummy){
    keepRunning = 0;
}

int16_t swap_bytes(int16_t val) {
    return (val << 8) | ((val >> 8) & 0xFF);   
}

// Calcula diferença de tempo em nanosegundos
long long timespec_diff_ns(struct timespec *start, struct timespec *end) {
    return (end->tv_sec - start->tv_sec) * 1000000000LL + (end->tv_nsec - start->tv_nsec);
}

int main() {
    // Inicializa a biblioteca WiringPi para usar numeração BCM (GPIO direto)
    if (wiringPiSetupGpio() < 0) {
        fprintf(stderr, "Erro ao inicializar WiringPi.\n");
        return EXIT_FAILURE;
    }
    
    // Ativa o handling de sinal pra terminação limpa (Ctrl + C)
    signal(SIGINT, intHandler);

    pinMode(LED_GPIO, OUTPUT);

    lcd_init();
    lcd_write("Iniciando...", "Aguarde...");

    // Abre a comunicação I2C com o ADS1115
    int handle = wiringPiI2CSetup(ADS1115_ADDR);
    if (handle < 0) {
        fprintf(stderr, "Erro ao abrir comunicação I2C com o ADS1115.\n");
        return EXIT_FAILURE;
    }
    
    printf("Iniciando leitura...\n");
    printf("Pressione Ctrl+C encerrar.\n");

    float dbfs_limit = -12.0f;
    float dbfs_sum = 0.0f;
    float dbfs_avg = 0.0f;
    int count = 0;

    struct timespec loop_start, loop_end;
    struct timespec avg_start, avg_current;
    int avg_initialized = 0;
    
    // Pré-calculando constantes para otimização
    const float voltage_scale = 2.048f / 32768.0f;
    const float rms_scale = 1.0f / MAX_RMS;
    const float log_scale = 20.0f / log(10.0f);
    const float samples_inv = 1.0f / NUM_SAMPLES;

    while (keepRunning) {
        // Marca o início do loop
        clock_gettime(CLOCK_MONOTONIC, &loop_start);

        float sumSquares = 0.0f;

        for (int i = 0; i < NUM_SAMPLES; i++) {
            // Envia o comando de configuração para o ADS1115 e espera a conversão
            wiringPiI2CWriteReg16(handle, 0x01, (ADS1115_CONFIG >> 8) | (ADS1115_CONFIG << 8));
            usleep(CONVERSION_DELAY);

            int16_t value = wiringPiI2CReadReg16(handle, 0x00);
            value = swap_bytes(value);

            // Calcula o valor da tensão do sinal
            float voltage = value * voltage_scale;
            // Remove o offset DC do MAX9814
            float ac_voltage = voltage - DC_OFFSET;
            // Soma os quadrados das tensões amostradas para cálculo do RMS
            sumSquares += ac_voltage * ac_voltage;

            // Teste
            // printf("Raw: %6d | V: %1.3f | AC: %1.3f\n", value, voltage, ac_voltage);
        }

        // Calcula o valor RMS do sinal
        float rms = sqrtf(sumSquares * samples_inv);
        // Normaliza o sinal com base no RMS de uma senóide de pico 1V (0 = silencioso, 1 = máximo) e a limita
        float normalized = rms * rms_scale;
        if (normalized > 1.0f) normalized = 1.0f;

        // Cálculo do valor dBFS do sinal RMS
        float dbfs = log_scale * logf((rms > 0.0001f ? rms : 0.0001f) * rms_scale);
        
        // Calcula o comprimento da barra e a imprime com os valores de RMS e dBFS
        float norm_clamped = (normalized < MIN_NORMALIZED) ? MIN_NORMALIZED : normalized;
        int barLength = (int)((logf(norm_clamped) - logf(MIN_NORMALIZED)) / 
                              (-logf(MIN_NORMALIZED)) * BAR_WIDTH);
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

        // Inicializa o timestamp para média se ainda não foi feito
        if (!avg_initialized) {
            clock_gettime(CLOCK_MONOTONIC, &avg_start);
            avg_initialized = 1;
        }
        
        dbfs_sum += dbfs;
        count++;

        // Verifica se passou 1 segundo desde o início da média
        clock_gettime(CLOCK_MONOTONIC, &avg_current);
        long long avg_elapsed_ns = timespec_diff_ns(&avg_start, &avg_current);
        
        if (avg_elapsed_ns >= 1000000000LL) { // 1 segundo em nanosegundos
            // Calcula a média
            dbfs_avg = dbfs_sum / count;
            printf("Average dBFS: %6.1f dB (%d samples in %.2f s)\n", 
                   dbfs_avg, count, avg_elapsed_ns / 1000000000.0);

            char lcd_line1[17], lcd_line2[17];
            snprintf(lcd_line1, sizeof(lcd_line1), "Nivel Medio:");
            snprintf(lcd_line2, sizeof(lcd_line2), "%6.1f dBFS", dbfs_avg);
            lcd_write(lcd_line1, lcd_line2);

            if (dbfs_avg > dbfs_limit) {
                printf("LED on...\n");
                digitalWrite(LED_GPIO, HIGH);               
            } else {
                printf("LED off..\n");
                digitalWrite(LED_GPIO, LOW);
            }
            
            // Reset para próxima média
            dbfs_sum = 0.0f;
            count = 0;
            clock_gettime(CLOCK_MONOTONIC, &avg_start);
        }

        // Marca o fim do loop e calcula o tempo decorrido
        clock_gettime(CLOCK_MONOTONIC, &loop_end);
        long long elapsed_ns = timespec_diff_ns(&loop_start, &loop_end);
        
        // Calcula o tempo restante para atingir o intervalo
        long long remaining_ns = TARGET_INTERVAL_NS - elapsed_ns;
        
        // Se ainda há tempo restante, aguarda
        if (remaining_ns > 0) {
            struct timespec sleep_time;
            sleep_time.tv_sec = remaining_ns / 1000000000LL;
            sleep_time.tv_nsec = remaining_ns % 1000000000LL;
            nanosleep(&sleep_time, NULL);
        }
    }

    lcd_send_byte(0x01, LCD_CMD);
    usleep(2000);
    lcd_write("Encerrando...", "Tchau!");
    printf("\nTerminando o programa.\n");
    return EXIT_SUCCESS;
}