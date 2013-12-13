#ifndef __NOKIA_DRIVER
#define __NOKIA_DRIVER

#include <avr/io.h>

	typedef struct bitmap bitmap_t;

	static struct bitmap {
		uint8_t width;
		uint8_t height;
		uint8_t images;
		uint8_t data[];
	};

	void lcd_backlight(uint8_t state);					
	void lcd_repaint(void);
	void lcd_char(uint8_t character, uint8_t x, uint8_t y);
	void lcd_charlarge(uint8_t character, uint8_t x, uint8_t y);
	void lcd_string(uint8_t *characters, uint8_t x, uint8_t y);
	void lcd_stringlarge(uint8_t *characters, uint8_t x, uint8_t y);
	void lcd_uint8(uint8_t val, uint8_t x, uint8_t y);
	uint8_t lcd_uint8font(bitmap_t * font, uint8_t val, uint8_t x, uint8_t y, uint8_t padding);
	void lcd_freqfont(bitmap_t * font, bitmap_t * decimalsymbol, uint16_t freq, uint8_t x, uint8_t y, uint8_t padding);
	uint8_t lcd_freqfont_measure(bitmap_t * font, bitmap_t * decimalsymbol, uint16_t freq, uint8_t x, uint8_t y, uint8_t padding);	
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
