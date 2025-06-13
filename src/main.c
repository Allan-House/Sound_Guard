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
int parse_arguments(int argc, char *argv[], float *dbfs_limit);

int main(int argc, char *argv[]) {

    float dbfs_limit;

    // Processa argumentos da linha de comando
    int parse_result = parse_arguments(argc, argv, &dbfs_limit);
    if (parse_result == 0) {
        return EXIT_SUCCESS; // --help foi chamado
    }
    if (parse_result == -1) {
        return EXIT_FAILURE; // Erro nos argumentos
    }

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

void print_usage(const char *program_name) {
    printf("Uso: %s [OPÇÕES]\n", program_name);
    printf("\nDESCRIÇÃO:\n");
    printf("  Monitor de nível de áudio com display LCD e controle de LED\n");
    printf("  Monitora o nível de áudio através do ADS1115 e ativa o LED\n");
    printf("  quando o nível médio ultrapassa o limite configurado.\n");
    printf("\nOPÇÕES:\n");
    printf("  -l, --limit VALOR    Define o limite dBFS para ativação do LED\n");
    printf("                       (padrão: -12.0 dBFS)\n");
    printf("  -h, --help          Mostra esta mensagem de ajuda\n");
    printf("\nEXEMPLOS:\n");
    printf("  %s                  # Usa limite padrão de -12.0 dBFS\n", program_name);
    printf("  %s -l -15.0         # Define limite para -15.0 dBFS\n", program_name);
    printf("  %s --limit -10      # Define limite para -10.0 dBFS\n", program_name);
    printf("\nNOTAS:\n");
    printf("  • O programa deve ser executado com privilégios de root (sudo)\n");
    printf("  • Pressione Ctrl+C para encerrar o programa\n");
    printf("  • Valores dBFS típicos: -60 a 0 (0 = máximo, -60 = muito baixo)\n");
}

int parse_arguments(int argc, char *argv[], float *dbfs_limit) {
    *dbfs_limit = -12.0f; // Valor padrão
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0; // Retorna 0 para indicar que deve sair (mas não é erro)
        }
        else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--limit") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Erro: Opção '%s' requer um valor.\n", argv[i]);
                print_usage(argv[0]);
                return -1;
            }
            
            char *endptr;
            float value = strtof(argv[i + 1], &endptr);
            
            if (*endptr != '\0') {
                fprintf(stderr, "Erro: Valor '%s' não é um número válido.\n", argv[i + 1]);
                print_usage(argv[0]);
                return -1;
            }
            
            if (value > 0.0f) {
                fprintf(stderr, "Aviso: Valor dBFS %.1f é positivo. Valores dBFS são tipicamente negativos.\n", value);
                printf("Continuar mesmo assim? (s/N): ");
                char response;
                scanf(" %c", &response);
                if (response != 's' && response != 'S') {
                    printf("Operação cancelada.\n");
                    return -1;
                }
            }
            
            *dbfs_limit = value;
            i++; // Pula o próximo argumento (valor do limite)
        }
        else {
            fprintf(stderr, "Erro: Opção desconhecida '%s'.\n", argv[i]);
            print_usage(argv[0]);
            return -1;
        }
    }
    
    return 1; // Sucesso, continuar execução
}
