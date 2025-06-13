#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <wiringPi.h>

#include "config.h"
#include "lcd.h"
#include "adc.h"
#include "audio.h"
#include "timing.h"

volatile int keep_running = 1;

// Handler de sinal para terminação limpa (Ctrl + C)
void intHandler(int dummy) {
    keep_running = 0;
}

void print_usage(const char *program_name);

int main(int argc, char *argv[]) {

    float dbfs_limit = -12.0f;

    if (wiringPiSetupGpio() < 0) {
        fprintf(stderr, "Erro ao inicializar WiringPi.\n");
        return EXIT_FAILURE;
    }
    
    signal(SIGINT, intHandler);

    pinMode(LED_GPIO, OUTPUT);

    lcd_init();
    lcd_write("Iniciando...", "Aguarde...");

    int adc_handle = adc_init();
    if (adc_handle < 0) {
        return EXIT_FAILURE;
    }
    
    printf("Iniciando leitura...\n");
    printf("Pressione Ctrl+C encerrar.\n");

    float dbfs_sum = 0.0f;
    float dbfs_avg = 0.0f;
    int count = 0;

    struct timespec loop_start;
    struct timespec avg_start, avg_current;
    int avg_initialized = 0;

    while (keep_running) {
        // Marca o início do loop
        clock_gettime(CLOCK_MONOTONIC, &loop_start);

        float rms = adc_calculate_rms(adc_handle);
        
        float dbfs = audio_calculate_dbfs(rms);
        
        audio_print_bar(rms, dbfs);
        
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

        // Aguarda para manter o intervalo de tempo desejado
        timing_wait_for_interval(&loop_start);
    }

    lcd_cleanup();
    printf("\nTerminando o programa.\n");
    return EXIT_SUCCESS;
}