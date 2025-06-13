#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPiI2C.h>

#include "lcd.h"
#include "config.h"

static int lcd_fd = -1;

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

void lcd_init(void) {
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

void lcd_cleanup(void) {
    if (lcd_fd >= 0) {
        lcd_send_byte(0x01, LCD_CMD);
        usleep(2000);
        lcd_write("Encerrando...", "Tchau!");
    }
}