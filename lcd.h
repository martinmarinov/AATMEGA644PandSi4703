#ifndef __NOKIA_DRIVER
#define __NOKIA_DRIVER

#include <avr/io.h>

	typedef struct bitmap bitmap_t;

	static struct bitmap {
		uint8_t width;
		uint8_t height;
		uint8_t images; // how many frames does this bitmap have
		uint8_t data[];
	};

	void lcd_backlight(uint8_t state);					
	void lcd_repaint(void);
	void lcd_char(uint8_t character, uint8_t x, uint8_t y);
	void lcd_charlargewhite(uint8_t character, uint8_t x, uint8_t y);
	void lcd_string(uint8_t *characters, uint8_t x, uint8_t y);
	void lcd_stringwhite(uint8_t *characters, uint8_t x, uint8_t y);
	void lcd_stringlarge(uint8_t *characters, uint8_t x, uint8_t y);
	void lcd_uint8(uint8_t val, uint8_t x, uint8_t y);	
	void lcd_uint16(uint16_t val, uint8_t x, uint8_t y);
	void lcd_uint16bin(uint16_t val, uint8_t x, uint8_t y);
	void lcd_uint8hex(uint8_t val, uint8_t x, uint8_t y);
	void lcd_uint8bin(uint8_t val, uint8_t x, uint8_t y);
	void lcd_clear(void);
	void lcd_init(void);
	void lcd_bitmap(bitmap_t * bit, uint8_t id, uint8_t x, uint8_t y);

	void lcd_fillrect(uint8_t x, uint8_t y, uint8_t width, uint8_t height);
	void lcd_clearrect(uint8_t x, uint8_t y, uint8_t width, uint8_t height);
	void lcd_setpixel(uint8_t x, uint8_t y);

#endif
