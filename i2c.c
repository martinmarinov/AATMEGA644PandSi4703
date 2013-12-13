#include "i2c.h"
#include <util/delay_basic.h>
#include <util/delay.h>

/* 3/2/2012  svn 499  */



static volatile uint8_t i2c_timer ;

void i2c_timerproc (void) {
	
	uint8_t n ;	
	n=i2c_timer ;
	if (n) {
		i2c_timer = --n ;
	}
}


uint8_t read_i2c_device(uint8_t address,uint8_t rbytes,uint8_t *rdata) {

	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN) ;			// START. Note TWIE not set so polling, not interrupts
	i2c_timer = 3 ;
	while ( (!(TWCR & (1<<TWINT))) && i2c_timer)			// wait for flag = START sent
		;
	if (i2c_timer == 0) {
		return 1 ;
	}
	if ((TWSR & 0xF8) != TW_START) {
		_delay_us(30);
		return 2 ;
	}
	TWDR = (address | 0x01) ;								// reading so add in Read bit in case it wasn't set in address
	TWCR = (1<<TWINT) | (1<<TWEN) ;							// Send address
	i2c_timer = 3 ;
	while ( (!(TWCR & (1<<TWINT))) && i2c_timer)			// wait for flag = address sent
		;
	if (i2c_timer == 0) {
		return 3 ;
	}
	switch (TWSR & 0xF8) {

		case TW_MR_SLA_ACK:
			break;
		case TW_MR_SLA_NACK:
			_delay_us(30);
			return 4;
			break;
		case TW_MR_ARB_LOST:
			_delay_us(30);
			return 5;
			break;
		default:
			TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO) ;	// Send stop
			_delay_us(30);
			return 6;
	}
	if (rbytes == 1) {
		TWCR = (1<<TWINT) | (1<<TWEN);						// get data. Ack if > 1 byte expected				
	} else {
		TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
	}

	while (rbytes) {
	
		i2c_timer = 3 ;
		while ( (!(TWCR & (1<<TWINT))) && i2c_timer)		// wait for flag = data sent
			;
		if (i2c_timer == 0) {
			return 7 ;
		}
		switch (TWSR & 0xF8) {

			case TW_MR_DATA_ACK:
				*rdata++ = TWDR ;						// read the data
				rbytes-- ;
				if (rbytes > 1) {						// Ack unless last one
					TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);				
				} else {
					TWCR = (1<<TWINT) | (1<<TWEN);
				}
				break;
			case TW_MR_DATA_NACK:
				*rdata++ = TWDR ;						// read the data
				rbytes = 0 ;							// force stop
				break;

		}
	}	
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO) ;			// Send stop
	_delay_us(30);
	return 0 ;	
}




uint8_t write_i2c_device(uint8_t address,uint8_t wbytes,uint8_t *wdata) {
	
	uint8_t i ;
	
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN) ;			// START. Note TWIE not set to polling, not interrupts
	i2c_timer = 3 ;
	while ( (!(TWCR & (1<<TWINT))) && i2c_timer)			// wait for flag = START sent
		;
	if (i2c_timer == 0) {
		return 17 ;
	}
	if ( (TWSR & 0xF8) != TW_START) {
		_delay_us(30);
		return 18 ;
	}
	TWDR = (address & 0xFE) ;								// writing so mask off Read bit in case it was set in address
	TWCR = (1<<TWINT) | (1<<TWEN) ;							// Send address
	i2c_timer = 3 ;
	while ( (!(TWCR & (1<<TWINT))) && i2c_timer)			// wait for flag = address sent
		;
	if (i2c_timer == 0) {
		return 19 ;
	}
	if( (TWSR & 0xF8) != TW_MT_SLA_ACK ) {
		_delay_us(30);
		return 20 ;
	}
	for (i=0;i<wbytes;i++) {
		TWDR = wdata[i] ;
		TWCR = (1<<TWINT) | (1<<TWEN) ;						// Send data
		i2c_timer = 3 ;
		while ( (!(TWCR & (1<<TWINT))) && i2c_timer)		// wait for flag = data sent
			;
		if (i2c_timer == 0) {
			return 23 ;
		}
		if ((TWSR & 0xF8) != TW_MT_DATA_ACK) {
			_delay_us(30);
			return 24 ;
		}
	}			
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO) ;			// Send stop
	_delay_us(30);
	return 0 ;
}



uint8_t write_read_i2c_device(uint8_t address,uint8_t wbytes,uint8_t *wdata,uint8_t rbytes,uint8_t *rdata) {
	
	uint8_t i ;
	
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN) ;			// START. Note TWIE not set so polling, not interrupts
	i2c_timer = 3 ;
	while ( (!(TWCR & (1<<TWINT))) && i2c_timer)			// wait for flag = START sent
		;
	if (i2c_timer == 0) {
		return 33 ;
	}
	if ( (TWSR & 0xF8) != TW_START) {
		_delay_us(30);
		return 34 ;
	}
	TWDR = (address & 0xFE) ;								// writing so mask off Read bit in case it was set in address
	TWCR = (1<<TWINT) | (1<<TWEN) ;							// Send address
	i2c_timer = 3 ;
	while ( (!(TWCR & (1<<TWINT))) && i2c_timer)			// wait for flag = address sent
		;
	if (i2c_timer == 0) {
		return 35 ;
	}
	if( (TWSR & 0xF8) != TW_MT_SLA_ACK ) {
		_delay_us(30);
		return 36 ;
	}
	for (i=0;i<wbytes;i++) {
		TWDR = wdata[i] ;
		TWCR = (1<<TWINT) | (1<<TWEN) ;						// Send data
		i2c_timer = 3 ;
		while ( (!(TWCR & (1<<TWINT))) && i2c_timer)		// wait for flag = data sent
			;
		if (i2c_timer == 0) {
			return 39 ;
		}
		if ((TWSR & 0xF8) != TW_MT_DATA_ACK) {
			_delay_us(30);
			return 40 ;
		}
	}
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTA) ;			// Send repeated start
	i2c_timer = 3 ;
	while ( (!(TWCR & (1<<TWINT))) && i2c_timer)			// wait for flag = START sent
		;
	if (i2c_timer == 0) {
		_delay_us(30);
		return 41 ;
	}
	if ( (TWSR & 0xF8) != TW_REP_START) {
		_delay_us(30);
		return 42 ;
	}

	TWDR = (address | 0x01) ;								// reading so add in Read bit in case it wasn't set in address
	TWCR = (1<<TWINT) | (1<<TWEN) ;							// Send address
	i2c_timer = 3 ;
	while ( (!(TWCR & (1<<TWINT))) && i2c_timer)			// wait for flag = address sent
		;
	if (i2c_timer == 0) {
		return 43 ;
	}
	switch (TWSR & 0xF8) {

		case TW_MR_SLA_ACK:
			break;
		case TW_MR_SLA_NACK:
			_delay_us(30);
			return 44;
			break;
		case TW_MR_ARB_LOST:
			_delay_us(30);
			return 45;
			break;
		default:
			TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO) ;	// Send stop
			_delay_us(30);
			return 46;
	}
	if (rbytes == 1) {
		TWCR = (1<<TWINT) | (1<<TWEN);						// get data. Ack if > 1 byte expected				
	} else {
		TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
	}

	while (rbytes) {
	
		i2c_timer = 3 ;
		while ( (!(TWCR & (1<<TWINT))) && i2c_timer)		// wait for flag = data sent
			;
		if (i2c_timer == 0) {
			return 47 ;
		}
		switch (TWSR & 0xF8) {

			case TW_MR_DATA_ACK:
				*rdata++ = TWDR ;							// read the data
				rbytes-- ;
				if (rbytes > 1) {					// Ack unless last one
					TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);				
				} else {
					TWCR = (1<<TWINT) | (1<<TWEN);
				}
				break;
			case TW_MR_DATA_NACK:
				*rdata++ = TWDR ;							// read the data
				rbytes = 0 ;							// force stop
				break;

		}
	}	
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO) ;			// Send stop
	_delay_us(30);
	return 0 ;	
}
