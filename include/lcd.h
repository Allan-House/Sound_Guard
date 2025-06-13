#ifndef LCD_H
#define LCD_H

#include <stdint.h>

// LCD control functions
void lcd_init(void);
void lcd_write(const char *line1, const char *line2);
void lcd_send_byte(uint8_t bits, int mode);
void lcd_toggle_enable(uint8_t bits);
void lcd_cleanup(void);

#endif // LCD_H