#ifndef CONFIG_H
#define CONFIG_H

// GPIO Configuration
#define LED_GPIO 17             // GPIO 17 (pino físico 11), modo BCM usado pelo wiringPiSetupGpio()

// ADS1115 Configuration
#define ADS1115_ADDR 0x48       // Endereço padrão do ADS1115 no barramento I2C
#define CONVERSION_DELAY 4000   // 4ms para conversão a 250 SPS
#define NUM_SAMPLES 4           // Quantidade de amostragens

// Configuração do ADS1115:
// - Bit 15: Iniciar conversão single-shot (1)
// - Bits 14-12: Canal AIN0 (100)
// - Bits 11-9: Ganho ±2.048V (010)
// - Bit 8: Modo single-shot (1)
// - Bits 7-5: Data rate 250 SPS (101)
// - Bits 4-0: Comparator disabled (00011)
#define ADS1115_CONFIG 0b1100010110100011

// LCD Configuration
#define LCD_I2C_ADDR 0x27       // Endereço padrão do módulo I2C (PCF8574)
#define LCD_BACKLIGHT 0x08
#define LCD_ENABLE    0x04
#define LCD_CMD       0
#define LCD_CHR       1

// Audio Processing Configuration
#define BAR_WIDTH 80            // Dimensão da barra visual
#define MAX_RMS 0.707f          // Nível RMS de referência para 0 dBFS
#define DC_OFFSET 1.25f         // Offset de tensão do MAX9814
#define MIN_NORMALIZED 0.001f   // Valor mínimo de escala log, evitando log(0)

// Timing Configuration
#define TARGET_INTERVAL_NS 33330000  // Intervalo de tempo de ~33.33ms em nanosegundos (30 FPS)

#endif // CONFIG_H