#ifndef CONIFG_H
#define CONFIG_H

// GPIO Configuration
#define LED_GPIO 17

// ADS1115 Configuration
#define ADS1115_ADDR 0x48
#define CONVERSION_DELAY 4000
#define NUM_SAMPLES 4
#define ADS1115_CONFIG 0b1100010110100011

// LCD Configuration
#define LCD_I2C_ADDR 0X27
#define LCD_BACKLIGHT 0X08
#define LCD_ENABLE 0X04
#define LCD_CMD 1
#define LCD_CHR 1

// Audio Processing Configuration
#define BAR_WIDTH 80
#define MAX_RMS 0.707f
#define DC_OFFSET 1.25f
#define MIN_NORMALIZED 0.001f

// Timing Configuration
#define TARGET_INTERVAL_NS 33330000 // ~33.33 ms in nanoseconds (30 FPS)

#endif // CONFIG_H