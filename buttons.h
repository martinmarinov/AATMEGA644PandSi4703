#ifndef __BUTTONS
#define __BUTTONS

	#include <avr/io.h>
	#define BUTTONS_UP (0)
	#define BUTTONS_DOWN (1)
	#define BUTTONS_PRESSED (2)
	
	/** The Hardware Configuration should be written to config.h
	The driver expects the following preprocessor defines to be present in the file
		
	BUTTONS_PORT – the port to which the keypad is connected
	BUTTONS_DDR – the DDR of this port
	BUTTONS_PIN – the PIN of this port

	The ports that the keypad is connected to don't need to be pre-initialized, the buttons_init will do that for you.
	
	The keypad needs to be connected on pins 0 to 6 starting from the control wires.
	**/

	/* read from config BUTTONS_PORT and BUTTONS_DDR. Buttons should be connected from 0 to 6 */
	void buttons_init(void);

	/* returns 1 on success and 0 on failure
	val is the char
	event is an event BUTTONS_UP or BUTTONS_DOWN
	if function returns 0, the value of val and state is undefined */
	uint8_t buttons_poll(uint8_t * val, uint8_t * event);	

#endif
