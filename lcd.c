#include "lcd.h"

#include <util/delay.h>				// _delay_us() and _delay_ms() functions
#include <util/delay_basic.h>

#include "spi2.h"					// Uses spi


// All on PORTB, these definitions were in the original code, I have modified them 
// to match the pins I used on my sample, but these definitions are never used.
// NOte that PB4 needs to be an output to force SPI Master mode
#define LED_PORT  PORTB
#define PIN_LED	  PB0   // display pin 1, the square pin, is LED backlight
/*#define PIN_SCLK  PB7   // display pin 2
#define PIN_SDIN  PB5   // display pin 3 */
#define PIN_DC    PB2   // display pin 4
#define PIN_RESET PB1   // display pin 5
#define PIN_SCE   PB3   // display pin 6
                        // display pin 7 is GND
                        // display pin 8 is 3.3V

#define GETSTATE(what, id) (( what & (1<<id)) != 0)


//The DC pin tells the LCD if we are sending a command or data
#define LCD_COMMAND 0 
#define LCD_DATA  1

//You may find a different size screen, but this one is 84 by 48 pixels
#define LCD_X     84
#define LCD_Y     48

//This table contains the hex values that represent pixels
//for a font that is 5 pixels wide and 8 pixels high
static const uint8_t ASCII[][5] = {
  {0x00, 0x00, 0x00, 0x00, 0x00} // 20  
  ,{0x00, 0x00, 0x5f, 0x00, 0x00} // 21 !
  ,{0x00, 0x07, 0x00, 0x07, 0x00} // 22 "
  ,{0x14, 0x7f, 0x14, 0x7f, 0x14} // 23 #
  ,{0x24, 0x2a, 0x7f, 0x2a, 0x12} // 24 $
  ,{0x23, 0x13, 0x08, 0x64, 0x62} // 25 %
  ,{0x36, 0x49, 0x55, 0x22, 0x50} // 26 &
  ,{0x00, 0x05, 0x03, 0x00, 0x00} // 27 '
  ,{0x00, 0x1c, 0x22, 0x41, 0x00} // 28 (
  ,{0x00, 0x41, 0x22, 0x1c, 0x00} // 29 )
  ,{0x14, 0x08, 0x3e, 0x08, 0x14} // 2a *
  ,{0x08, 0x08, 0x3e, 0x08, 0x08} // 2b +
  ,{0x00, 0x50, 0x30, 0x00, 0x00} // 2c ,
  ,{0x08, 0x08, 0x08, 0x08, 0x08} // 2d -
  ,{0x00, 0x60, 0x60, 0x00, 0x00} // 2e .
  ,{0x20, 0x10, 0x08, 0x04, 0x02} // 2f /
  ,{0x3e, 0x51, 0x49, 0x45, 0x3e} // 30 0
  ,{0x00, 0x42, 0x7f, 0x40, 0x00} // 31 1
  ,{0x42, 0x61, 0x51, 0x49, 0x46} // 32 2
  ,{0x21, 0x41, 0x45, 0x4b, 0x31} // 33 3
  ,{0x18, 0x14, 0x12, 0x7f, 0x10} // 34 4
  ,{0x27, 0x45, 0x45, 0x45, 0x39} // 35 5
  ,{0x3c, 0x4a, 0x49, 0x49, 0x30} // 36 6
  ,{0x01, 0x71, 0x09, 0x05, 0x03} // 37 7
  ,{0x36, 0x49, 0x49, 0x49, 0x36} // 38 8
  ,{0x06, 0x49, 0x49, 0x29, 0x1e} // 39 9
  ,{0x00, 0x36, 0x36, 0x00, 0x00} // 3a :
  ,{0x00, 0x56, 0x36, 0x00, 0x00} // 3b ;
  ,{0x08, 0x14, 0x22, 0x41, 0x00} // 3c <
  ,{0x14, 0x14, 0x14, 0x14, 0x14} // 3d =
  ,{0x00, 0x41, 0x22, 0x14, 0x08} // 3e >
  ,{0x02, 0x01, 0x51, 0x09, 0x06} // 3f ?
  ,{0x32, 0x49, 0x79, 0x41, 0x3e} // 40 @
  ,{0x7e, 0x11, 0x11, 0x11, 0x7e} // 41 A
  ,{0x7f, 0x49, 0x49, 0x49, 0x36} // 42 B
  ,{0x3e, 0x41, 0x41, 0x41, 0x22} // 43 C
  ,{0x7f, 0x41, 0x41, 0x22, 0x1c} // 44 D
  ,{0x7f, 0x49, 0x49, 0x49, 0x41} // 45 E
  ,{0x7f, 0x09, 0x09, 0x09, 0x01} // 46 F
  ,{0x3e, 0x41, 0x49, 0x49, 0x7a} // 47 G
  ,{0x7f, 0x08, 0x08, 0x08, 0x7f} // 48 H
  ,{0x00, 0x41, 0x7f, 0x41, 0x00} // 49 I
  ,{0x20, 0x40, 0x41, 0x3f, 0x01} // 4a J
  ,{0x7f, 0x08, 0x14, 0x22, 0x41} // 4b K
  ,{0x7f, 0x40, 0x40, 0x40, 0x40} // 4c L
  ,{0x7f, 0x02, 0x0c, 0x02, 0x7f} // 4d M
  ,{0x7f, 0x04, 0x08, 0x10, 0x7f} // 4e N
  ,{0x3e, 0x41, 0x41, 0x41, 0x3e} // 4f O
  ,{0x7f, 0x09, 0x09, 0x09, 0x06} // 50 P
  ,{0x3e, 0x41, 0x51, 0x21, 0x5e} // 51 Q
  ,{0x7f, 0x09, 0x19, 0x29, 0x46} // 52 R
  ,{0x46, 0x49, 0x49, 0x49, 0x31} // 53 S
  ,{0x01, 0x01, 0x7f, 0x01, 0x01} // 54 T
  ,{0x3f, 0x40, 0x40, 0x40, 0x3f} // 55 U
  ,{0x1f, 0x20, 0x40, 0x20, 0x1f} // 56 V
  ,{0x3f, 0x40, 0x38, 0x40, 0x3f} // 57 W
  ,{0x63, 0x14, 0x08, 0x14, 0x63} // 58 X
  ,{0x07, 0x08, 0x70, 0x08, 0x07} // 59 Y
  ,{0x61, 0x51, 0x49, 0x45, 0x43} // 5a Z
  ,{0x00, 0x7f, 0x41, 0x41, 0x00} // 5b [
  ,{0x02, 0x04, 0x08, 0x10, 0x20} // 5c  backslash, but line continuation overrides comment
  ,{0x00, 0x41, 0x41, 0x7f, 0x00} // 5d ]
  ,{0x04, 0x02, 0x01, 0x02, 0x04} // 5e ^
  ,{0x40, 0x40, 0x40, 0x40, 0x40} // 5f _
  ,{0x00, 0x01, 0x02, 0x04, 0x00} // 60 `
  ,{0x20, 0x54, 0x54, 0x54, 0x78} // 61 a
  ,{0x7f, 0x48, 0x44, 0x44, 0x38} // 62 b
  ,{0x38, 0x44, 0x44, 0x44, 0x20} // 63 c
  ,{0x38, 0x44, 0x44, 0x48, 0x7f} // 64 d
  ,{0x38, 0x54, 0x54, 0x54, 0x18} // 65 e
  ,{0x08, 0x7e, 0x09, 0x01, 0x02} // 66 f
  ,{0x0c, 0x52, 0x52, 0x52, 0x3e} // 67 g
  ,{0x7f, 0x08, 0x04, 0x04, 0x78} // 68 h
  ,{0x00, 0x44, 0x7d, 0x40, 0x00} // 69 i
  ,{0x20, 0x40, 0x44, 0x3d, 0x00} // 6a j 
  ,{0x7f, 0x10, 0x28, 0x44, 0x00} // 6b k
  ,{0x00, 0x41, 0x7f, 0x40, 0x00} // 6c l
  ,{0x7c, 0x04, 0x18, 0x04, 0x78} // 6d m
  ,{0x7c, 0x08, 0x04, 0x04, 0x78} // 6e n
  ,{0x38, 0x44, 0x44, 0x44, 0x38} // 6f o
  ,{0x7c, 0x14, 0x14, 0x14, 0x08} // 70 p
  ,{0x08, 0x14, 0x14, 0x18, 0x7c} // 71 q
  ,{0x7c, 0x08, 0x04, 0x04, 0x08} // 72 r
  ,{0x48, 0x54, 0x54, 0x54, 0x20} // 73 s
  ,{0x04, 0x3f, 0x44, 0x40, 0x20} // 74 t
  ,{0x3c, 0x40, 0x40, 0x20, 0x7c} // 75 u
  ,{0x1c, 0x20, 0x40, 0x20, 0x1c} // 76 v
  ,{0x3c, 0x40, 0x30, 0x40, 0x3c} // 77 w
  ,{0x44, 0x28, 0x10, 0x28, 0x44} // 78 x
  ,{0x0c, 0x50, 0x50, 0x50, 0x3c} // 79 y
  ,{0x44, 0x64, 0x54, 0x4c, 0x44} // 7a z
  ,{0x00, 0x08, 0x36, 0x41, 0x00} // 7b {
  ,{0x00, 0x00, 0x7f, 0x00, 0x00} // 7c |
  ,{0x00, 0x41, 0x36, 0x08, 0x00} // 7d }
  ,{0x10, 0x08, 0x08, 0x10, 0x08} // 7e ~
  ,{0x78, 0x46, 0x41, 0x46, 0x78} // 7f DEL
};									

uint8_t lcd_buffer[LCD_X * LCD_Y / 8];		

void lcd_mem(uint8_t data_or_command);
void lcd_raw(uint8_t data);							
								
void lcd_goto(uint8_t x, uint8_t y) {
	lcd_mem(LCD_COMMAND);
	lcd_raw(0x80 | x);  // Column.
	lcd_raw(0x40 | y);  // Row.  ?
}

void lcd_backlight(uint8_t state) {
	if (state) {
		LED_PORT |= (1<<PIN_LED) ;					// D/C
	} else {
		LED_PORT &= ~(1<<PIN_LED);
	}
}

//This takes a large array of bits and sends them to the LCD
void lcd_repaint(void){
	uint16_t index;
	lcd_goto(0, 0);

	lcd_mem(LCD_DATA);
	for (index = 0 ; index < (LCD_X * LCD_Y / 8) ; index++)
		lcd_raw(lcd_buffer[index]);
	
}



// write a char at pixel x and row y
void lcd_char(uint8_t character, uint8_t x, uint8_t y) {

	uint8_t index;
	const uint8_t pmax = x+5;
	const uint8_t max = (pmax > LCD_X) ? (LCD_X) : (pmax);

	for (index = x ; index < max ; index++)
		lcd_buffer[y*LCD_X+index] |= ASCII[character - 0x20][index-x];
}

void lcd_charwhite(uint8_t character, uint8_t x, uint8_t y) {

	uint8_t index;
	const uint8_t pmax = x+5;
	const uint8_t max = (pmax > LCD_X) ? (LCD_X) : (pmax);

	for (index = x ; index < max ; index++)
		lcd_buffer[y*LCD_X+index] &= ~ASCII[character - 0x20][index-x];
}

// write a char at pixel x and row y
void lcd_charlarge(uint8_t character, uint8_t x, uint8_t y) {

	uint8_t index;
	uint8_t dispx = 0;
	uint8_t i;
	
	const uint8_t firstrow = y*LCD_X;
	const uint8_t secondrow = (y+1)*LCD_X;
	
	for (index = 0 ; index < 5 ; index++) {
		const uint8_t val = ASCII[character - 0x20][index];
		uint16_t val16 = 0;
		for (i = 0; i < 8; i++)
			if (( val & (1<<i)) != 0) val16 |= (3 << (i*2));

		const uint8_t f = val16 & 0x00FF;
		const uint8_t s = (val16 & 0xFF00) >> 8;

		lcd_buffer[firstrow+x+dispx] |= f;
		lcd_buffer[secondrow+x+(dispx++)] |= s;
		
		lcd_buffer[firstrow+x+dispx] |= f;
		lcd_buffer[secondrow+x+(dispx++)] |= s;
	}
}

void lcd_bitmap(bitmap_t * bit, uint8_t id, uint8_t posx, uint8_t posy) {
	uint8_t x;
	uint8_t y;

	const uint8_t bw = bit->width;
	const uint8_t bh = bit->height;
	uint8_t * data = bit->data;
	const uint16_t offset = bw * bh * id;

	for (x = 0; x < bw; x++)
		for (y = 0; y < bh; y++)
			lcd_buffer[(y+posy)*LCD_X+x + posx] ^= data[y*bw+x+offset];
}

void lcd_string(uint8_t *characters, uint8_t x, uint8_t y) {
	uint8_t pos = 0;
	while (*characters && pos < LCD_X) {
		lcd_char(*characters++, x+pos, y);
		pos+=5;	
	}
}

void lcd_stringwhite(uint8_t *characters, uint8_t x, uint8_t y) {
	uint8_t pos = 0;
	while (*characters && pos < LCD_X) {
		lcd_charwhite(*characters++, x+pos, y);
		pos+=5;	
	}
}

void lcd_stringlarge(uint8_t *characters, uint8_t x, uint8_t y) {
	uint8_t pos = 0;
	while (*characters) {
		lcd_charlarge(*characters++, x+pos, y);
		pos+=10;	
	}
}

void lcd_uint8(uint8_t val, uint8_t x, uint8_t y) {
	unsigned char buf[3];
	uint8_t pos = 0;
	int8_t ptr;
	for(ptr=0;ptr<3;++ptr) {
		buf[ptr] = (val % 10) + '0';
		val /= 10;
	}
	for(ptr=2;ptr>0;--ptr) {
		if (buf[ptr] != '0') break;
	}
	for(;ptr>=0;--ptr) {
		lcd_char(buf[ptr], x+pos, y);
		pos+=5;	
	}
}

void lcd_uint16(uint16_t val, uint8_t x, uint8_t y) {
	unsigned char buf[5];
	uint8_t pos = 0;
	int8_t ptr;
	for(ptr=0;ptr<5;++ptr) {
		buf[ptr] = (val % 10) + '0';
		val /= 10;
	}
	for(ptr=4;ptr>0;--ptr) {
		if (buf[ptr] != '0') break;
	}
	for(;ptr>=0;--ptr) {
		lcd_char(buf[ptr], x+pos, y);
		pos+=5;
	}
}

void lcd_uint8hex(uint8_t val, uint8_t x, uint8_t y) {
	const uint8_t high = val & 0xF;
	const uint8_t low = val >> 4;

	if (low < 10)
		lcd_char('0'+low, x, y);
	else
		lcd_char('A'+low-10, x, y);
	
	if (high < 10)
		lcd_char('0'+high, x+5, y);
	else
		lcd_char('A'+high-10, x+5, y);
}

void lcd_uint8bin(uint8_t val, uint8_t x, uint8_t y) {
	lcd_char(GETSTATE(val, 0) ? '1' : '0', x, y); 
	lcd_char(GETSTATE(val, 1) ? '1' : '0', x+5, y); 
	lcd_char(GETSTATE(val, 2) ? '1' : '0', x+10, y); 
	lcd_char(GETSTATE(val, 3) ? '1' : '0', x+15, y); 
	lcd_char(GETSTATE(val, 4) ? '1' : '0', x+20, y); 
	lcd_char(GETSTATE(val, 5) ? '1' : '0', x+25, y); 
	lcd_char(GETSTATE(val, 6) ? '1' : '0', x+30, y); 
	lcd_char(GETSTATE(val, 7) ? '1' : '0', x+35, y); 
}

void lcd_uint16bin(uint16_t val, uint8_t x, uint8_t y) {
	uint8_t * blah = (uint8_t *) &val;
	lcd_uint8bin(blah[0], x, y);
	lcd_uint8bin(blah[1], x+43, y);
}

void lcd_setpixel(uint8_t x, uint8_t y) {
	if (x < 0 || y < 0 || x > LCD_X || y > LCD_Y) return;
	const uint16_t yrow = y/8;
	lcd_buffer[yrow*LCD_X+x] |= 1 << (y-yrow*8);
}

void lcd_fillrect(uint8_t x, uint8_t y, uint8_t width, uint8_t height) {
	const uint16_t yrow = y >> 3;
	const uint8_t yrowbyteid=y-(yrow << 3);
	const uint8_t yrowbyteidr = 8-yrowbyteid;
	uint16_t index =  yrow*LCD_X+x;
	uint8_t left;	
	uint8_t xid;
		
	if (width == 0 || height == 0) return;
	if (x + width > LCD_X) {width = LCD_X - x; if (width == 0) return;};
	if (y + height > LCD_Y) {height = LCD_Y - y; if (height == 0) return;};
	
	left = height;

	// draw vertical lines
	
	if (yrowbyteid != 0) {
		const uint8_t first_byte = ((1 << (yrowbyteidr)) - 1) << (yrowbyteid);
		if (yrowbyteidr >= height) {
			const uint8_t behind = yrowbyteidr - height;
			const uint8_t last_byte = ((1 << (behind)) - 1) << (8-behind);
			left = 0;	
			for (xid = 0; xid < width; xid++) lcd_buffer[index+xid] |= (first_byte ^ last_byte);	
		} else {		
			left-=yrowbyteidr;
			for (xid = 0; xid < width; xid++) lcd_buffer[index+xid] |= first_byte;
			index+=LCD_X;
		}
	}

	while ((left >> 3) > 0) {
		for (xid = 0; xid < width; xid++) lcd_buffer[index+xid] = 0xFF; // if you are doing ^ istead of |, add ^ here
		index+=LCD_X;
		left-=8;
	}

	if (left > 0) {
		const uint8_t last_byte = (1 << left) - 1;	
		for (xid = 0; xid < width; xid++) lcd_buffer[index+xid] |= last_byte;
	}
}

void lcd_clearrect(uint8_t x, uint8_t y, uint8_t width, uint8_t height) {
	const uint16_t yrow = y >> 3;
	const uint8_t yrowbyteid=y-(yrow << 3);
	const uint8_t yrowbyteidr = 8-yrowbyteid;
	uint16_t index =  yrow*LCD_X+x;
	uint8_t left;	
	uint8_t xid;
		
	if (width == 0 || height == 0) return;
	if (x + width > LCD_X) {width = LCD_X - x; if (width == 0) return;};
	if (y + height > LCD_Y) {height = LCD_Y - y; if (height == 0) return;};
	
	left = height;

	// draw vertical lines
	
	if (yrowbyteid != 0) {
		const uint8_t first_byte = ((1 << (yrowbyteidr)) - 1) << (yrowbyteid);
		if (yrowbyteidr >= height) {
			const uint8_t behind = yrowbyteidr - height;
			const uint8_t last_byte = ((1 << (behind)) - 1) << (8-behind);
			left = 0;	
			for (xid = 0; xid < width; xid++) lcd_buffer[index+xid] &= ~ (first_byte ^ last_byte);	
		} else {		
			left-=yrowbyteidr;
			for (xid = 0; xid < width; xid++) lcd_buffer[index+xid] &= ~ first_byte;
			index+=LCD_X;
		}
	}

	while ((left >> 3) > 0) {
		for (xid = 0; xid < width; xid++) lcd_buffer[index+xid] = 0; // if you are doing ^ istead of |, add ^ here
		index+=LCD_X;
		left-=8;
	}

	if (left > 0) {
		const uint8_t last_byte = (1 << left) - 1;	
		for (xid = 0; xid < width; xid++) lcd_buffer[index+xid] &= ~ last_byte;
	}
}



//Clears the LCD by writing zeros to the entire screen
void lcd_clear(void) {
	uint16_t index;
	for (index = 0 ; index < (LCD_X * LCD_Y / 8) ; index++)
		lcd_buffer[index] = 0x00;
}


//This sends the magical commands to the PCD8544
void lcd_init(void) {

//Reset the LCD to a known state
	LED_PORT &= ~(1<<PIN_RESET);
	_delay_us(5);
	LED_PORT |= (1<<PIN_RESET);
	_delay_ms(10);

	lcd_mem(LCD_COMMAND);
	lcd_raw(0x21); //Tell LCD that extended commands follow
	lcd_raw(0xB1); //Set LCD Vop (Contrast): Try 0xB1(good @ 3.3V) or 0xBF if your display is too dark
	lcd_raw(0x04); //Set Temp coefficent
	lcd_raw(0x14); //LCD bias mode 1:48: Try 0x13 or 0x14

	lcd_raw(0x20); //We must send 0x20 before modifying the display control mode
	lcd_raw(0x0C); //Set display control, normal mode. 0x0D for inverse
}


//Whether a data or a command would be sent next
void lcd_mem(uint8_t data_or_command) {

	if (data_or_command) {
		LED_PORT |= (1<<PIN_DC) ;					// D/C
	} else {
		LED_PORT &= ~(1<<PIN_DC) ;
	}
}

//Send data according to a previous setup of LCDSwitchMemory
void lcd_raw(uint8_t data) {	
	LED_PORT &= ~(1<<PIN_SCE) ;					// CE
	xmit_spi(data);
	LED_PORT |= (1<<PIN_SCE) ;
}
