#ifndef __FM_DRIVER
#define __FM_DRIVER

#include <avr/io.h>
	
	#define RDS_NO (0)
	#define RDS_FAKE (1)
	#define RDS_AVAILABLE (2)

	uint8_t fm_turn_on(void);
	uint8_t fm_setChannel(uint16_t channel);
	uint8_t fm_setVolume(uint16_t volume);
	uint16_t fm_getChannel(uint8_t * rssi, uint8_t * stereo);
	uint8_t fm_seek(uint8_t seek_up); // true on success
	int fm_readRDS(char* ps, char* rt);

	// auto discover the address of the device. For debugging purposes
	// returns the i2c address or 0xFF for an error
	// radio won't work after this command
	uint8_t fm_get_isc_address(void);
	int str_putrawfreq(char * str, uint16_t freq, int start);

#endif
