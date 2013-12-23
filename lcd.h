#ifndef __NOKIA_DRIVER
#define __NOKIA_DRIVER

#include <avr/io.h>

	/** The Hardware Configuration should be written to config.h
		The driver expects the following preprocessor defines to be present in the file
		
	SPI_PORT, SPI_DDR, SPI_PIN, SPI_SCK, SPI_MISO and SPI_MOSI - used for SPI communication

	The SPI needs to be manually initialized as well as lcd_init needs to be called. Manual initialization example:
		DDRB = 0xBF ;
		PORTB = 0x1F ;	// SS, CS, reset inactive	
		SETH(DDRB, PB0); // for backlight on the LCD
	
	Also lcd.c contains a mapping for the LCD_LED which defines where the LCD backlight is being powered from.
	**/

	/* A bitmap that can store several images to be shown on screen */
	typedef struct bitmap bitmap_t;
	static struct bitmap {
		uint8_t width;
		uint8_t height; // row number 0 to 5
		uint8_t images; // how many frames does this bitmap have
		uint8_t data[]; // the actual pixel data
	};

	/* If state == 1, backlight is turned on, otherwise backlight is turned off */
	void lcd_backlight(uint8_t state);					
	
	/* Send the frame buffer to the LCD */
	void lcd_repaint(void);
	
	/* Show a single char in black of size 5x8 px using the in-built 5x8 px font. Y is row number (0 to 5). X is a pixel (0 to 83) */
	void lcd_char(uint8_t character, uint8_t x, uint8_t y);
	
	/* Show a single char in white of size 10x16 px using the in-built 5x8 px font (scaled by 2). Y is row number (0 to 5). X is a pixel (0 to 83) */
	void lcd_charlargewhite(uint8_t character, uint8_t x, uint8_t y);
	
	/* Show a null terminated string in black. Y is row number (0 to 5). X is a pixel (0 to 83)  */
	void lcd_string(uint8_t *characters, uint8_t x, uint8_t y);
	
	/* Show a null terminated string in white. Y is row number (0 to 5). X is a pixel (0 to 83)  */
	void lcd_stringwhite(uint8_t *characters, uint8_t x, uint8_t y);
	
	/* Show a null terminated string. Each char is 10x16 px (the in-built 5x8 px font scaled by 2) */
	void lcd_stringlarge(uint8_t *characters, uint8_t x, uint8_t y);
	
	/* Show a byte. See remarks for y in lcd_char */
	void lcd_uint8(uint8_t val, uint8_t x, uint8_t y);
	
	/* Show a short. See remarks for y in lcd_char */
	void lcd_uint16(uint16_t val, uint8_t x, uint8_t y);
	
	/* Show a short in binary. See remarks for y in lcd_char */
	void lcd_uint16bin(uint16_t val, uint8_t x, uint8_t y);
	
	/* Show a byte in hexadecimal. See remarks for y in lcd_char */
	void lcd_uint8hex(uint8_t val, uint8_t x, uint8_t y);
	
	/* Show a byte in binary. See remarks for y in lcd_char */
	void lcd_uint8bin(uint8_t val, uint8_t x, uint8_t y);
	
	/* Clear frame buffer */
	void lcd_clear(void);
	
	/* Initialize SPI connection with the LCD */
	void lcd_init(void);
	
	/* Draw a bitmap. Y is row number (0 to 5). X is a pixel (0 to 83) */
	void lcd_bitmap(bitmap_t * bit, uint8_t id, uint8_t x, uint8_t y);

	/* Efficiently fills a rectangle in black. Width is in pixels from 0 to 83 and height is in pixels from 0 to 47. */
	void lcd_fillrect(uint8_t x, uint8_t y, uint8_t width, uint8_t height);
	
	/* Efficiently fills a rectangle in white. Width is in pixels from 0 to 83 and height is in pixels from 0 to 47. */
	void lcd_clearrect(uint8_t x, uint8_t y, uint8_t width, uint8_t height);
	
	/* Make an individual pixel black */
	void lcd_setpixel(uint8_t x, uint8_t y);

#endif
