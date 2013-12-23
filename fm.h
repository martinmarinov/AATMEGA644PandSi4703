#ifndef __FM_DRIVER
#define __FM_DRIVER

#include <avr/io.h>
	
	#define RDS_NO (0)
	#define RDS_FAKE (1)
	#define RDS_AVAILABLE (2)
	
	/** The Hardware Configuration should be written to config.h
		The driver expects the following preprocessor defines to be present in the file
		
	FM_RESET - the pin that the reset is connected to
	FM_PORTRESET - the port of the above pin
	FM_DDRRESET - the DDR of the above pin
	FM_SDIO  - the pin where the sdio is connected to
	FM_PORTSDIO - the port of the above pin
	FM_DDRSDIO - the DDR of the above pin

	The following defines are required by i2c.c. Have a look at it for more information
	TW_DDR, TW_PIN, TW_PORT, TW_SCK, TW_SDA

	The ports that the radio is connected to don't need to be pre-initialized, the fm_turn_on function will do that for you.
	**/

	/* Sets up the pins, turns on the radio and starts the i2c conversation  */
	uint8_t fm_turn_on(void);
	
	/* Set frequency. The frequency is an integer of the real frequency in MHz multiplied by 10.
	For example if you want to tune to 91.8 MHz, you would set channel == 918 */
	uint8_t fm_setChannel(uint16_t channel);
	
	/* Sets the output volume on the headphones. Values are from 0 to 15 */
	uint8_t fm_setVolume(uint16_t volume);
	
	/* Returns the current FM channel, RSSI level in dBμV and a boolean flag indicating whether stereo signal is currently present. */
	uint16_t fm_getChannel(uint8_t * rssi, uint8_t * stereo);
	
	/* Finds the next frequency that has an RSSI higher than a predefined threshold. This effectively attempts to tune to the next/previous valid FM broadcast.
	If you want to seek up, set seek_up == 1, if you want to seek down, set seek_up == 0. Function returns 1 on success */
	uint8_t fm_seek(uint8_t seek_up);
	
	/* Read the RDS value and return the Program Service (string of 9 chars, including the last null) and Radio Text (string of 65 chars, including the last null).
	Returns either RDS_NO or RDS_AVALABLE or RDS_FAKE. RDS_FAKE means that the PS will contain a string that shows the current frequency in MHz to display in case of
    no RDS is available. */
	int fm_readRDS(char* ps, char* rt);

	/* auto discover the address of the device. For debugging purposes
	returns the i2c address or 0xFF for an error
	radio won't work after this command */
	uint8_t fm_get_isc_address(void);
	
	/* A string utility that takes a channel and prints it into the string with an offset start. */
	int str_putrawfreq(char * str, uint16_t freq, int start);

#endif
