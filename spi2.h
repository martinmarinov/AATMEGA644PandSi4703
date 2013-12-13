
/* 23/2/2011  svn 282  */


// this library only requires the definitions of registers in the Atmel.
/******************		
******************
	IMPORTANT
		The spi function built in to the Microcontroller needs to
		be initialised before the device is used. Use the init_spi() funtion here
******************		
******************/

#ifndef SPI2_H
#define SPI2_H


#include <avr/io.h>
		

/**
 * \brief 	Initialise the rate for the SPI bus.
 *
 * \param   rate    Value whose bottom 2 bits are used to set the SPI bus rate.
 *
 *             Requires I/O to have been set correctly independently
 *				See Atmel datasheet for the correct value to use in this function to get the SPI rate required.
 *				rate is masked so only the bottom 2 bits are valid.
 *			rate 0 0   fosc/4
 *				 0 1   fosc/16
 *				 1 0   fosc/64
 *				 1 1   fosc/128
 */
inline void init_spi(uint8_t rate);


/**
 * \brief 	Transmit an 8 bit value to the SPI bus.
 *
 * \param   data     Value to send via SPI bus.
 *
 *             Requires spi_init() to have been run, and I/O set correctly
 */
inline void xmit_spi(uint8_t data);


/**
 * \brief  	Receive an 8 bit value from the SPI bus.
 *
 * \return     Value from SPI bus.
 *
 *             Requires spi_init() to have been run, and I/O set correctly
 */
inline uint8_t recv_spi(void);


#endif
