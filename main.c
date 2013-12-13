/* these two lines take care of the IO definitions for the device, and the interrupts available for them */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h> 
#include <avr/wdt.h>

#define SETH(reg,id) reg |= (1 << id)
#define SETL(reg,id) reg &= ~(1 << id)

#define F_CPU 8E6				   // 8MHz
#include <util/delay_basic.h>
#include <util/delay.h>

#include <stdio.h>
#include <string.h>

#include "config.h"
#include "lcd.h"
#include "buttons.h"
#include "fm.h"
#include "spi2.h"
#include "bitmaps.h"

#define LCD_X     84
#define LCD_Y     48

/* put comments in here for future reference, showing what the device is intended to do, which
device is to be used, and the fuse settings (and why) */


/* This is a template created by cutting bits from a stepper motor drive
   Some port direction commands are included as examples */



/* Intended for ATmega644P */
/* Clock rate is 8.0 MHz  (8MHz internal oscillator (default), divide by 8 (default), to give CLKIO of 1MHz */

// Fuse high byte is default (0x99)			See data sheet page 297

/* Fuse low byte is 0xE2:  ((NB fuses active low)   See data sheet page 297
 * CLKDIV8 	1 (div8 inactive)
 * CLKOUT 	1 (clk out inactive)
 * SUT1   	1 (fast rising power)
 * SUT0 	0
 * CKSEL3	0  8MHz internal osc
 * CKSEL2	0
 * CKSEL1	1
 * CKSEL0	0
 */


/* Interrupt vectors */
// If we were using interrupts the interrupt vectors would be here
// there is a useful list at    http://www.nongnu.org/avr-libc/user-manual/group__avr__interrupts.html


/* Port direction setting */
	
// TODO! move that in LCD
inline void port_direction_init() {

/* DDR is Data Direction Register
   PORT is used when referring to outputs
   PIN  is used when referring to inputs 
   See ATMEGA644P datasheet section 13  */

	DDRB = 0xBF ;
	PORTB = 0x1F ;					// SS, CS, reset inactive	

// the led
	SETH(DDRB, PB0); // for backlight on the LCD

// PORTC all inputs, no pullups
	DDRC = 0 ;
	PORTC = 0xFF ;	

// PORTD all inputs, no pullups
	DDRD = 0;
	PORTD = 0xFF;

// power reduction
	PRR = 0x7B;
}

char rdsname[9] = "HELLO :)";
char rdsrt[65] = "Welcome to radio";

#define MAXSCALEFREQ (1097)
#define MINSCALEFREQ (864)
#define LCD_BACKLIGHT_TIMEOUT (2000)
uint16_t lcd_timeout = LCD_BACKLIGHT_TIMEOUT;
uint16_t freq;
uint8_t vol = 1;
uint8_t rds = 0;
uint8_t rdschartoshow = 0;

void drawScreen(void) {
	uint8_t rssi, stereo;
	freq = fm_getChannel(&rssi, &stereo);
	
	
	const uint8_t xpointerpx = (bmp_scale.width * (freq - MINSCALEFREQ)) / (MAXSCALEFREQ - MINSCALEFREQ);

	bitmap_t * scale_font = &font_normal;
	bitmap_t * scale_font_dec = &font_normal_dec;
	
	const uint8_t freqsize = lcd_freqfont_measure(scale_font, scale_font_dec, freq, 10, 2, 1);
	const uint8_t startx = (xpointerpx < 2 + (freqsize >> 1)) ? 0 : ( (xpointerpx + (freqsize >> 1) + 2 > LCD_X) ? (LCD_X - freqsize - 4) : (xpointerpx - (freqsize >> 1) - 2) );	
	const uint8_t sizeoftext = scale_font->height * 8;
	const uint8_t pos_of_vfo = LCD_Y-16+sizeoftext;
	const uint8_t size_of_vfo = 16-sizeoftext;

	lcd_bitmap(&bmp_scale, 0, 0, 4);
	lcd_clearrect(startx, LCD_Y-18, freqsize+3, sizeoftext+3);	
	lcd_fillrect(startx+1, LCD_Y-17, freqsize+1, sizeoftext+1);
	lcd_clearrect(xpointerpx - 4, pos_of_vfo, 1, size_of_vfo);		
	lcd_fillrect(xpointerpx - 3, pos_of_vfo, 3, size_of_vfo);
	lcd_clearrect(xpointerpx, pos_of_vfo, 1, size_of_vfo);	
	lcd_fillrect(xpointerpx + 1, pos_of_vfo, 3, size_of_vfo);	
	lcd_clearrect(xpointerpx + 4, pos_of_vfo, 1, size_of_vfo);
	lcd_freqfont(scale_font, scale_font_dec, freq, startx+2, 4, 1);

	lcd_stringlarge(rdsname, 0, 1);

	if (rdschartoshow<64)
		lcd_string(&rdsrt[rdschartoshow], 0, 3);
	if (rdschartoshow>64-18)
		lcd_string(rdsrt, 5*(65-rdschartoshow), 3);
		

	lcd_bitmap(&bmp_sound, (vol == 0) ? (0) : (1+5 * (vol-1) / 14), 0, 0);
	if (rds == RDS_AVAILABLE) lcd_bitmap(&bmp_rds, 0, 20, 0);
	if (stereo) lcd_bitmap(&bmp_stereo, 0, 43, 0);
	lcd_bitmap(&bmp_rssi, (rssi < 20) ? (0) : ( (rssi > 55) ? (6) : (1+5 * (rssi-20) / (55-20))), 64, 0);
}

int main(void) {	

// Initialise ports and SPI
	port_direction_init();
	buttons_init();	
	init_spi(3);	

	lcd_init();
	uint8_t fmstat = fm_turn_on();
	
// Let everything settle
	_delay_ms(1000);

	SREG |= 0x80 ;

	lcd_clear();
	drawScreen();
	lcd_repaint();
	lcd_timeout = LCD_BACKLIGHT_TIMEOUT;
	lcd_backlight(1);

	uint8_t letter;
	uint8_t state;
	fm_setVolume(vol);

	while (1)
	{
		if (lcd_timeout == 1) {
			lcd_timeout = 0;
			lcd_backlight(0);
			lcd_clear();
			drawScreen();			 
			lcd_repaint();
			lcd_timeout = LCD_BACKLIGHT_TIMEOUT;
		} else if (lcd_timeout != 0)
			lcd_timeout--;
		
		if (buttons_poll(&letter, &state) && state == BUTTONS_PRESSED) {
			lcd_timeout = LCD_BACKLIGHT_TIMEOUT;
			lcd_backlight(1);

			switch(letter) {
			case '1':
				fm_seek(0);
				rdschartoshow = 0;
				break;
			case '3':
				fm_seek(1);
				rdschartoshow = 0;
				break;
			case '#':
				if (vol < 15) vol++;
				fm_setVolume(vol);
				break;
			case '*':
				if (vol > 0) vol--;
				fm_setVolume(vol);
				break;
			case '6':
				if (freq == 1080)
					freq = 875;
				else
					freq++;
				fm_setChannel(freq);
				rdschartoshow = 0;
				break;
			case '4':
				if (freq == 875)
					freq = 1080;
				else
					freq--;
				fm_setChannel(freq);
				rdschartoshow = 0;
				break;
			case '9':
				if (rdschartoshow == 65)
					rdschartoshow = 0;
				else
					rdschartoshow++;
				break;
			case '7':
				if (rdschartoshow == 0)
					rdschartoshow = 65;
				else
					rdschartoshow--;
				break;
			}

			lcd_clear(); 
			drawScreen();			 
			lcd_repaint();

		} else if (rds = fm_readRDS(rdsname, rdsrt)) {
			lcd_clear();
			drawScreen();			 
			lcd_repaint();
		}		
	}
}		



