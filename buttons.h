#ifndef __BUTTONS
#define __BUTTONS

	#include <avr/io.h>
	#define BUTTONS_UP (0)
	#define BUTTONS_DOWN (1)
	#define BUTTONS_PRESSED (2)

	// read from config BUTTONS_PORT and BUTTONS_DDR. Buttons should be connected from 0 to 7
	void buttons_init(void);

	// returns 1 on success and 0 on failure
	// val is the char
	// event is an event BUTTONS_UP or BUTTONS_DOWN
	// if function returns 0, the value of val and state is undefined
	uint8_t buttons_poll(uint8_t * val, uint8_t * event);	

#endif
