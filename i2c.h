
#ifndef I2C_H
#define I2C_H

#include <avr/io.h>

/* 23/2/2011  svn 282  */


#define 	TW_START   0x08
#define 	TW_REP_START   0x10
#define 	TW_MT_SLA_ACK   0x18
#define 	TW_MT_SLA_NACK   0x20
#define 	TW_MT_DATA_ACK   0x28
#define 	TW_MT_DATA_NACK   0x30
#define 	TW_MT_ARB_LOST   0x38
#define 	TW_MR_ARB_LOST   0x38
#define 	TW_MR_SLA_ACK   0x40
#define 	TW_MR_SLA_NACK   0x48
#define 	TW_MR_DATA_ACK   0x50
#define 	TW_MR_DATA_NACK   0x58
#define 	TW_ST_SLA_ACK   0xA8
#define 	TW_ST_ARB_LOST_SLA_ACK   0xB0
#define 	TW_ST_DATA_ACK   0xB8
#define 	TW_ST_DATA_NACK   0xC0
#define 	TW_ST_LAST_DATA   0xC8
#define 	TW_SR_SLA_ACK   0x60
#define 	TW_SR_ARB_LOST_SLA_ACK   0x68
#define 	TW_SR_GCALL_ACK   0x70
#define 	TW_SR_ARB_LOST_GCALL_ACK   0x78read_i2c_device
#define 	TW_SR_DATA_ACK   0x80
#define 	TW_SR_DATA_NACK   0x88
#define 	TW_SR_GCALL_DATA_ACK   0x90
#define 	TW_SR_GCALL_DATA_NACK   0x98
#define 	TW_SR_STOP   0xA0
#define 	TW_NO_INFO   0xF8
#define 	TW_BUS_ERROR   0x00



/* 

Return values from I2C for read,write,write-read
0 	0	0	success
1	17	33	timeout waiting for start condition, device read
2	18	34	failed to send start
3	19	35	timeout waiting for send address
4	20	36	slave failed to ACK address
5			arbitration lost
6			other failure while expecting slave ACK address
7	23	39	timeout waiting for data sent
8	24	40	slave failed to ACK data
		41	timeout on repeated start
		42	failed to assert repeated start on bus
		43	timeout waiting for send address
		44	slave failed to ACK address
		45	arbitration lost
		46	other failure while expectiread_i2c_deviceng slave ACK address
		47	timeout waiting for data sent
		
*/





/* Note that there are no delays padding calls to these functions
 * If two calls occur in quick succession, you must add a delay (10uS should be plenty) to allow the
 * bus to recover. 
 *  You MUST also initialise the I2C bus registers TWSR and TWBR. For 8MHz :
	TWCR = 0x04 ;							// Enable TWI. Overrides IO definitions
 	TWSR = 2 ;								// Clock rate is 8000000/(16 + 2xTWBR * 4 ^ TWSR)
	TWBR = 2 ;								// = 8000000/(16 + (4 * 16)) = 100kHz
 */
 
 
void i2c_timerproc (void); 
 

/** \brief		Read 1 or more bytes from an I2C device to memory.
 *
 *	\param		address		Address if I2C device. The bottom bit is set to enforce an I2C read.
 *	\param		bytes		Number of bytes to be returned.
 *	\param		*rdata	    Pointer to location to store values returned.
 *
 *	\return					returns error code generated from I2C transaction, or 0 for success
 *
 */
uint8_t read_i2c_device(uint8_t address, uint8_t bytes, uint8_t *rdata);



/** \brief		Write 1 or more bytes to an I2C device from memory.
 *
 *	\param		address		Address if I2C device. The bottom bit is cleared to enforce an I2C write.
 *	\param		bytes		Number of data bytes to be sent.
 *	\param		*wdata		Pointer to location in memory of first byte of set of data.
 *
 *	\return					returns error code generated from I2C transaction, or 0 for success
 *
 */
uint8_t write_i2c_device(uint8_t address, uint8_t bytes, uint8_t *wdata);



/** \brief		Write 1 or more bytes to an I2C device followed by a read without releasing the bus.
 *
 *	\param		address		Address if I2C device. The bottom bit is manipulated as appropriate for R/W
 *	\param		wbytes		Number of bytes to write.
 *	\param		*wdata		Pointer to location in memory of first byte to write.
 *	\param		rbytes		Number of bytes to read.
 *	\param		*rdata		Pointer to location in memory of first byte of set of data.
 *
 *	\return					returns error code generated from I2C transaction, or 0 for success
 *
 */
uint8_t write_read_i2c_device(uint8_t address, uint8_t wbytes, uint8_t *wdata, uint8_t rbytes, uint8_t *rdata);

#endif

