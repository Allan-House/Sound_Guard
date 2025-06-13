#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <time.h>

// Definições dos endereços I2C
#define ADS1115_ADDR 0x48
#define LCD_ADDR 0x27
#define LED_GPIO 17

// Cores para output (funciona na maioria dos terminais)
#define COLOR_GREEN "\033[32m"
#define COLOR_RED "\033[31m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BLUE "\033[34m"
#define COLOR_RESET "\033[0m"

// Estrutura para manter estatísticas dos testes
typedef struct {
    int total_tests;
    int passed_tests;
    int failed_tests;
} test_stats_t;

// Função para imprimir cabeçalho
void print_header() {
    printf("\n");
    printf("╔══════════════════════════════════════╗\n");
    printf("║        SOUND GUARD DIAGNOSTIC        ║\n");
    printf("║            Versão 1.0                ║\n");
    printf("╚══════════════════════════════════════╝\n");
    printf("\n");
}

// Função para imprimir resultado de teste
void print_test_result(const char* test_name, int passed, const char* details) {
    if (passed) {
        printf("✅ %s[PASS]%s %s", COLOR_GREEN, COLOR_RESET, test_name);
    } else {
        printf("❌ %s[FAIL]%s %s", COLOR_RED, COLOR_RESET, test_name);
    }
    
    if (details && strlen(details) > 0) {
        printf(" - %s", details);
    }
    printf("\n");
}

// Função para pausar e aguardar entrada do usuário
void wait_for_user(const char* message) {
    printf("%s%s%s", COLOR_YELLOW, message, COLOR_RESET);
    getchar(); // Consome o \n anterior
    getchar(); // Aguarda Enter
}

// Teste 1: Verificação do WiringPi
int test_wiringpi(test_stats_t* stats) {
    printf("\n%s1. Testando inicialização WiringPi...%s\n", COLOR_BLUE, COLOR_RESET);
    
    if (wiringPiSetupGpio() >= 0) {
        print_test_result("WiringPi GPIO Setup", 1, "Biblioteca inicializada com sucesso");
        stats->passed_tests++;
        return 1;
    } else {
        print_test_result("WiringPi GPIO Setup", 0, "Falha na inicialização - execute como root");
        stats->failed_tests++;
        return 0;
    }
}

// Teste 2: Escaneamento de dispositivos I2C
int test_i2c_device(int address, const char* device_name, test_stats_t* stats) {
    int handle = wiringPiI2CSetup(address);
    if (handle < 0) {
        char details[100];
        snprintf(details, sizeof(details), "Endereço 0x%02X não respondeu", address);
        print_test_result(device_name, 0, details);
        stats->failed_tests++;
        return 0;
    }
    
    // Tenta ler do dispositivo
    int result = wiringPiI2CRead(handle);
    if (result >= 0) {
        char details[100];
        snprintf(details, sizeof(details), "Endereço 0x%02X respondeu (0x%02X)", address, result & 0xFF);
        print_test_result(device_name, 1, details);
        stats->passed_tests++;
        return 1;
    } else {
        char details[100];
        snprintf(details, sizeof(details), "Endereço 0x%02X sem resposta", address);
        print_test_result(device_name, 0, details);
        stats->failed_tests++;
        return 0;
    }
}

void test_i2c_scan(test_stats_t* stats) {
    printf("\n%s2. Escaneando barramento I2C...%s\n", COLOR_BLUE, COLOR_RESET);
    
    stats->total_tests += 2;
    test_i2c_device(ADS1115_ADDR, "ADS1115 (ADC)", stats);
    test_i2c_device(LCD_ADDR, "LCD I2C Adapter", stats);
}

// Teste 3: Verificação do GPIO (LED)
void test_gpio_led(test_stats_t* stats) {
    printf("\n%s3. Testando GPIO (LED)...%s\n", COLOR_BLUE, COLOR_RESET);
    
    stats->total_tests++;
    
    pinMode(LED_GPIO, OUTPUT);
    
    printf("Teste do LED - GPIO %d:\n", LED_GPIO);
    printf("• Ligando LED...\n");
    digitalWrite(LED_GPIO, HIGH);
    wait_for_user("LED deve estar LIGADO. Pressione Enter para continuar...");
    
    printf("• Desligando LED...\n");
    digitalWrite(LED_GPIO, LOW);
    wait_for_user("LED deve estar DESLIGADO. Pressione Enter para continuar...");
    
    printf("O LED funcionou corretamente? (s/N): ");
    char response;
    scanf(" %c", &response);
    
    if (response == 's' || response == 'S') {
        print_test_result("GPIO LED Test", 1, "Confirmado pelo usuário");
        stats->passed_tests++;
    } else {
        print_test_result("GPIO LED Test", 0, "Falha confirmada pelo usuário");
        stats->failed_tests++;
    }
}

// Teste 4: Teste funcional do ADS1115
int test_ads1115_functional(test_stats_t* stats) {
    printf("\n%s4. Teste funcional ADS1115...%s\n", COLOR_BLUE, COLOR_RESET);
    
    stats->total_tests++;
    
    int handle = wiringPiI2CSetup(ADS1115_ADDR);
    if (handle < 0) {
        print_test_result("ADS1115 Functional", 0, "Não foi possível conectar");
        stats->failed_tests++;
        return 0;
    }
    
    // Configura o ADS1115 para leitura single-ended do canal 0
    // Config: AINp=AIN0, AINn=GND, gain=±2.048V, single-shot
    int config = 0xC1E3; // Binary: 1100000111100011
    
    if (wiringPiI2CWriteReg16(handle, 0x01, ((config & 0xFF) << 8) | ((config >> 8) & 0xFF)) < 0) {
        print_test_result("ADS1115 Functional", 0, "Falha ao escrever configuração");
        stats->failed_tests++;
        return 0;
    }
    
    // Aguarda conversão
    delay(100);
    
    // Lê o resultado da conversão
    int raw_value = wiringPiI2CReadReg16(handle, 0x00);
    if (raw_value < 0) {
        print_test_result("ADS1115 Functional", 0, "Falha ao ler dados");
        stats->failed_tests++;
        return 0;
    }
    
    // Converte byte order (big-endian para little-endian)
    raw_value = ((raw_value & 0xFF) << 8) | ((raw_value >> 8) & 0xFF);
    if (raw_value > 32767) raw_value -= 65536; // Converte para signed
    
    // Converte para tensão (±2.048V range, 16-bit)
    float voltage = (raw_value * 2.048) / 32767.0;
    
    char details[100];
    snprintf(details, sizeof(details), "Leitura: %.3fV (Raw: %d)", voltage, raw_value);
    print_test_result("ADS1115 Functional", 1, details);
    stats->passed_tests++;
    
    return 1;
}

// Teste 5: Teste completo do sistema
void test_system_integration(test_stats_t* stats) {
    printf("\n%s5. Teste de integração do sistema...%s\n", COLOR_BLUE, COLOR_RESET);
    
    stats->total_tests++;
    
    printf("Executando 5 leituras consecutivas do ADC...\n");
    
    int handle = wiringPiI2CSetup(ADS1115_ADDR);
    if (handle < 0) {
        print_test_result("System Integration", 0, "ADC não disponível");
        stats->failed_tests++;
        return;
    }
    
    int successful_reads = 0;
    float voltage_sum = 0.0;
    
    for (int i = 0; i < 5; i++) {
        // Configura e lê
        int config = 0xC1E3;
        if (wiringPiI2CWriteReg16(handle, 0x01, ((config & 0xFF) << 8) | ((config >> 8) & 0xFF)) >= 0) {
            delay(100);
            int raw_value = wiringPiI2CReadReg16(handle, 0x00);
            if (raw_value >= 0) {
                raw_value = ((raw_value & 0xFF) << 8) | ((raw_value >> 8) & 0xFF);
                if (raw_value > 32767) raw_value -= 65536;
                float voltage = (raw_value * 2.048) / 32767.0;
                voltage_sum += voltage;
                successful_reads++;
                printf("  Leitura %d: %.3fV\n", i+1, voltage);
                
                // Pisca o LED para cada leitura
                digitalWrite(LED_GPIO, HIGH);
                delay(100);
                digitalWrite(LED_GPIO, LOW);
                delay(200);
            }
        }
    }
    
    if (successful_reads == 5) {
        char details[100];
        float avg_voltage = voltage_sum / successful_reads;
        snprintf(details, sizeof(details), "5/5 leituras OK - Média: %.3fV", avg_voltage);
        print_test_result("System Integration", 1, details);
        stats->passed_tests++;
    } else {
        char details[100];
        snprintf(details, sizeof(details), "Apenas %d/5 leituras bem-sucedidas", successful_reads);
        print_test_result("System Integration", 0, details);
        stats->failed_tests++;
    }
}

// Função para imprimir resultado final
void print_final_results(test_stats_t* stats) {
    printf("\n");
    printf("╔══════════════════════════════════════╗\n");
    printf("║            RESULTADO FINAL           ║\n");
    printf("╚══════════════════════════════════════╝\n");
    
    printf("\nEstatísticas dos testes:\n");
    printf("• Total de testes: %d\n", stats->total_tests);
    printf("• %sAprovados: %d%s\n", COLOR_GREEN, stats->passed_tests, COLOR_RESET);
    printf("• %sFalharam: %d%s\n", COLOR_RED, stats->failed_tests, COLOR_RESET);
    
    float success_rate = (float)stats->passed_tests / stats->total_tests * 100.0;
    printf("• Taxa de sucesso: %.1f%%\n", success_rate);
    
    printf("\n");
    if (stats->failed_tests == 0) {
        printf("🎉 %sTodos os testes passaram! Sistema pronto para uso.%s\n", COLOR_GREEN, COLOR_RESET);
        printf("\nVocê pode executar o Sound Guard com confiança:\n");
        printf("  sudo ./Sound_Guard\n");
        printf("  sudo ./Sound_Guard -l -15.0\n");
    } else if (success_rate >= 80.0) {
        printf("⚠️  %sA maioria dos testes passou, mas há alguns problemas.%s\n", COLOR_YELLOW, COLOR_RESET);
        printf("Verifique as conexões dos componentes que falharam.\n");
    } else {
        printf("❌ %sMuitos testes falharam. Verifique as conexões e tente novamente.%s\n", COLOR_RED, COLOR_RESET);
        printf("\nSugestões:\n");
        printf("• Verifique se todos os cabos estão conectados\n");
        printf("• Confirme os endereços I2C dos módulos\n");
        printf("• Execute como root (sudo)\n");
        printf("• Verifique se o I2C está habilitado no Raspberry Pi\n");
    }
    printf("\n");
}

// Função para imprimir ajuda
void print_help(const char* program_name) {
    printf("Uso: %s [OPÇÕES]\n", program_name);
    printf("\nDESCRIÇÃO:\n");
    printf("  Programa de diagnóstico para o sistema Sound Guard\n");
    printf("  Verifica conexões e funcionalidade dos componentes\n");
    printf("\nOPÇÕES:\n");
    printf("  -h, --help          Mostra esta mensagem de ajuda\n");
    printf("  -q, --quick         Executa apenas testes básicos\n");
    printf("  -v, --verbose       Modo verboso com detalhes extras\n");
    printf("\nEXEMPLOS:\n");
    printf("  sudo %s             # Executa todos os testes\n", program_name);
    printf("  sudo %s -q          # Executa apenas testes rápidos\n", program_name);
    printf("\nNOTA:\n");
    printf("  Este programa deve ser executado como root (sudo)\n");
}

int main(int argc, char* argv[]) {
    int quick_mode = 0;
    int verbose_mode = 0;
    
    // Processa argumentos da linha de comando
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_help(argv[0]);
            return EXIT_SUCCESS;
        }
        else if (strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "--quick") == 0) {
            quick_mode = 1;
        }
        else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            verbose_mode = 1;
        }
        else {
            fprintf(stderr, "Opção desconhecida: %s\n", argv[i]);
            print_help(argv[0]);
            return EXIT_FAILURE;
        }
    }
    
    // Inicializa estatísticas
    test_stats_t stats = {0, 0, 0};
    
    print_header();
    
    if (getuid() != 0) {
        printf("%s⚠️  AVISO: Este programa deve ser executado como root (sudo)%s\n\n", COLOR_YELLOW, COLOR_RESET);
    }
    
    // Executa os testes
    stats.total_tests++; // WiringPi test
    if (!test_wiringpi(&stats)) {
        printf("\n%sAborting tests due to WiringPi failure.%s\n", COLOR_RED, COLOR_RESET);
        print_final_results(&stats);
        return EXIT_FAILURE;
    }
    
    test_i2c_scan(&stats);
    
    if (!quick_mode) {
        test_gpio_led(&stats);
        test_ads1115_functional(&stats);
        test_system_integration(&stats);
    }
    
    print_final_results(&stats);
    
    return (stats.failed_tests == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}