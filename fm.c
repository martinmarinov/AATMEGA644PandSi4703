#include "fm.h"
#include <avr/sleep.h> 
#include "i2c.h"
#include "config.h"
#include <string.h>

#define SETH(reg,id) reg |= (1 << id)
#define SETL(reg,id) reg &= ~(1 << id)

#include <util/delay_basic.h>
#include <util/delay.h>

uint16_t si4703_registers[16]; //There are 16 registers, each 16 bits large
uint8_t isc_buffer[32]; // the size of registers time 2 (since registers are 2 bytes each)

#define SI4703_READ  0x21
#define SI4703_WRITE  0x20

#define I2C_FAIL_MAX 10 //This is the number of attempts we will try to contact the device before erroring out
#define SEEK_DOWN 0 //Direction used for seeking. Default is down
#define SEEK_UP 1

//Define the register names
#define DEVICEID 0x00
#define CHIPID 0x01
#define POWERCFG 0x02
#define CHANNEL 0x03
#define SYSCONFIG1 0x04
#define SYSCONFIG2 0x05
#define STATUSRSSI 0x0A
#define READCHAN 0x0B
#define RDSA 0x0C
#define RDSB 0x0D
#define RDSC 0x0E
#define RDSD 0x0F

//Register 0x02 - POWERCFG
#define SMUTE 15
#define DMUTE 14
#define SKMODE 10
#define SEEKUP 9
#define SEEK 8

//Register 0x03 - CHANNEL
#define TUNE 15

//Register 0x04 - SYSCONFIG1
#define RDS 12
#define DE 11

//Register 0x05 - SYSCONFIG2
#define SPACE1 5
#define SPACE0 4

//Register 0x0A
#define RDSR 15
#define STC 14
#define SFBL 13
#define AFCRL 12
#define RDSS 11
#define STEREO 8

// RDS specific stuff
#define RDS_PS (0)
#define RDS_RT (2)

// allowed chars in RDS
#define FIRST_ALLOWED_CHAR (0x20)
#define LAST_ALLOWED_CHAR (0x7f)

// rds buffering
char rdsdata[9];
char radiotext[65];
int rdschanged = 0;
int fakerds = 1;

void clearStringBuff(char * buf, int textsize) {
	memset(buf, ' ', textsize);
	buf[textsize] = 0;
}

// draws a uint8 onto a string and returns position of last char
int str_putuint8(char * str, uint8_t val, int start) {
	
	unsigned char buf[3];
	int8_t ptr;
	for(ptr=0;ptr<3;++ptr) {
		buf[ptr] = (val % 10);
		val /= 10;
	}
	for(ptr=2;ptr>0;--ptr) {
		if (buf[ptr] != 0) break;
	}
	for(;ptr>=0;--ptr) {
		str[start] = '0'+buf[ptr];
		start++;	
	}
	return start;
}

int str_putrawfreq(char * str, uint16_t freq, int start) {
	uint8_t fract = freq % 10;
	uint8_t whole = freq / 10;
	
	start = str_putuint8(str, whole, start);
	str[start++] = '.';
	start = str_putuint8(str, fract, start);
	return start;
}

int str_putfreq(char * str, uint16_t freq, int start) {
	start = str_putrawfreq(str, freq, start);
	str[start++] = 'M';
	str[start++] = 'H';
	str[start++] = 'z';
	return start;
}

void clearRDSBuff(void) {
	uint16_t channel = (si4703_registers[READCHAN] & 0x03FF) + 875;
	clearStringBuff(rdsdata, 8);
	clearStringBuff(radiotext, 64);
	str_putfreq(rdsdata, channel, 0);
	rdschanged = 1;
	fakerds = 1;
}

//Read the entire register control set from 0x00 to 0x0F
uint8_t fm_readRegisters(void){
	int i = 0;
	int x;

	//Si4703 begins reading from register upper register of 0x0A and reads to 0x0F, then loops to 0x00.
	uint8_t result = read_i2c_device(SI4703_READ, 32, isc_buffer);
	i2c_timerproc();

	//Remember, register 0x0A comes in first so we have to shuffle the array around a bit
	for(x = 0x0A ; ; x++) { //Read in these 32 bytes
		if(x == 0x10) x = 0; //Loop back to zero
		si4703_registers[x] = isc_buffer[i++] << 8;
		si4703_registers[x] |= isc_buffer[i++];
		if(x == 0x09) break; //We're done!
	}

	return result;
}

//Write the current 9 control registers (0x02 to 0x07) to the Si4703
//It's a little weird, you don't write an I2C addres
//The Si4703 assumes you are writing to 0x02 first, then increments
uint8_t updateRegisters(void) {

	int i = 0;
	int regSpot;
	uint8_t res;

	//A write command automatically begins with register 0x02 so no need to send a write-to address
	//First we send the 0x02 to 0x07 control registers
	//In general, we should not write to registers 0x08 and 0x09
	for(regSpot = 0x02 ; regSpot < 0x08 ; regSpot++) {
		isc_buffer[i++] = si4703_registers[regSpot] >> 8; //Upper 8 bits
		isc_buffer[i++] = si4703_registers[regSpot] & 0x00FF; //Lower 8 bits
	}

	res = write_i2c_device(SI4703_WRITE, i, isc_buffer);
	i2c_timerproc();
	return res;
}


void fm_spi_init(void)
{
	SETH(FM_DDRRESET, FM_RESET);
	SETH(FM_DDRSDIO, FM_SDIO);

	_delay_ms(1);

	SETL(FM_PORTSDIO, FM_SDIO);
	SETL(FM_PORTRESET, FM_RESET); //Put Si4703 into reset
	_delay_ms(1); //Some delays while we allow pins to settle
	SETH(FM_PORTRESET, FM_RESET); //Bring Si4703 out of reset with SDIO set to low and SEN pulled high with on-board resistor
	_delay_ms(1); //Allow Si4703 to come out of reset

	FM_DDRSDIO = 0 ;
	FM_PORTSDIO = 0 ;

	_delay_ms(1);

	//we target 57600
	// TWBR=((8e6/57600 - 16)/2)/(4^(TWPS))
	// TWPS = 0; TWBR = 61;
	// TWPS = 1; TWBR = 15;
	// TWPS = 2; TWBR = 4;
	// TWPS = 3; TWBR = 1
	// 4 2 gives 7692 SCL_FREQ via (8e6)/(16+2*(TWBR)*4^(TWSR))
	TWSR = 4 ;
	TWCR = 2 ;
	TWBR = 2 ;
}

uint8_t fm_get_isc_address(void)
{
	uint8_t addr = 0;
	for (addr = 0; addr < 255; addr++) {
		fm_spi_init();
		if (read_i2c_device(addr, 32, isc_buffer) == 0) return addr;
	}
	return 0xFF;
}

uint8_t fm_turn_on(void) 
{
  uint8_t result = 0;

  fm_spi_init();

  result = fm_readRegisters(); if (result != 0) return result; //Read the current register set
  //si4703_registers[0x07] = 0xBC04; //Enable the oscillator, from AN230 page 9, rev 0.5 (DOES NOT WORK, wtf Silicon Labs datasheet?)
  si4703_registers[0x07] = 0x8100; //Enable the oscillator, from AN230 page 9, rev 0.61 (works)
  result = updateRegisters(); if (result != 0) return result;  //Update

  _delay_ms(500); //Wait for clock to settle - from AN230 page 9

  result = fm_readRegisters(); if (result != 0) return result;  //Read the current register set
  si4703_registers[POWERCFG] = 0x4001; //Enable the IC
  //si4703_registers[POWERCFG] |= (1<<SMUTE) | (1<<DMUTE); //Disable Mute, disable softmute
  si4703_registers[SYSCONFIG1] |= (1<<RDS); //Enable RDS

  si4703_registers[SYSCONFIG1] |= (1<<DE); //50kHz Europe setup
  si4703_registers[SYSCONFIG2] |= (1<<SPACE0); //100kHz channel spacing for Europe

  si4703_registers[SYSCONFIG2] &= 0xFFF0; //Clear volume bits
  si4703_registers[SYSCONFIG2] |= 0x0001; //Set volume to lowest
  result = updateRegisters(); if (result != 0) return result;  //Update

  _delay_ms(110); //Max powerup time, from datasheet page 13
  return result;
}

uint8_t fm_setChannel(uint16_t channel)
{
  uint8_t attempts;
  uint8_t result = 0;
  //Freq(MHz) = 0.200(in USA) * Channel + 87.5MHz
  //97.3 = 0.2 * Chan + 87.5
  //9.8 / 0.2 = 49
  uint16_t newChannel = channel * 10; //973 * 10 = 9730

  newChannel -= 8750; //9730 - 8750 = 980
  newChannel /= 10; //980 / 10 = 98

  //These steps come from AN230 page 20 rev 0.5
  result = fm_readRegisters();  if (result != 0) return result; 
  si4703_registers[CHANNEL] &= 0xFE00; //Clear out the channel bits
  si4703_registers[CHANNEL] |= newChannel; //Mask in the new channel
  si4703_registers[CHANNEL] |= (1<<TUNE); //Set the TUNE bit to start
  result = updateRegisters();  if (result != 0) return result; 

  //_delay_ms(60); //Wait 60ms - you can use or skip this delay

  //Poll to see if STC is set
  attempts=0;
  while(1) {
    result = fm_readRegisters();  if (result != 0) return result; 
    if( (si4703_registers[STATUSRSSI] & (1<<STC)) != 0) break; //Tuning complete!
    if ((attempts++) == 0xFF) return 123;
  }

  result = fm_readRegisters();  if (result != 0) return result; 
  si4703_registers[CHANNEL] &= ~(1<<TUNE); //Clear the tune after a tune has completed
  result = updateRegisters();  if (result != 0) return result; 

  //Wait for the si4703 to clear the STC as well
  attempts=0;
  while(1) {
    result = fm_readRegisters();
    if( (si4703_registers[STATUSRSSI] & (1<<STC)) == 0) break; //Tuning complete!
    if ((attempts++) == 0xFF) return 124;
  }

  clearRDSBuff();

  return result;
}

uint8_t fm_setVolume(uint16_t volume)
{
  uint8_t result = 0;
  result = fm_readRegisters();  if (result != 0) return result;  //Read the current register set
  if(volume < 0) volume = 0;
  if (volume > 15) volume = 15;
  si4703_registers[SYSCONFIG2] &= 0xFFF0; //Clear volume bits
  si4703_registers[SYSCONFIG2] |= volume; //Set new volume
  result = updateRegisters();  if (result != 0) return result;  //Update
  return result;
}

uint16_t fm_getChannel(uint8_t * rssi, uint8_t * stereo) {
  fm_readRegisters();
  uint16_t channel = si4703_registers[READCHAN] & 0x03FF; //Mask out everything but the lower 10 bits
  //Freq(MHz) = 0.100(in Europe) * Channel + 87.5MHz
  //X = 0.1 * Chan + 87.5
  channel += 875; //98 + 875 = 973
  *rssi = si4703_registers[STATUSRSSI] & 0xFF;
  *stereo = (( si4703_registers[STATUSRSSI] & (1<<STEREO)) != 0);
  return channel;
}

uint8_t fm_seek(uint8_t seek_up){
  fm_readRegisters();
  //Set seek mode wrap bit
  si4703_registers[POWERCFG] |= (1<<SKMODE); //Allow wrap
  //si4703_registers[POWERCFG] &= ~(1<<SKMODE); //Disallow wrap - if you disallow wrap, you may want to tune to 87.5 first
  if(!seek_up) si4703_registers[POWERCFG] &= ~(1<<SEEKUP); //Seek down is the default upon reset
  else si4703_registers[POWERCFG] |= 1<<SEEKUP; //Set the bit to seek up

  si4703_registers[POWERCFG] |= (1<<SEEK); //Start seek
  updateRegisters(); //Seeking will now start

  //Poll to see if STC is set
  while(1) {
    fm_readRegisters();
    if((si4703_registers[STATUSRSSI] & (1<<STC)) != 0) break; //Tuning complete!
  }

  fm_readRegisters();
  uint16_t valueSFBL = si4703_registers[STATUSRSSI] & (1<<SFBL); //Store the value of SFBL
  si4703_registers[POWERCFG] &= ~(1<<SEEK); //Clear the seek bit after seek has completed
  updateRegisters();

  //Wait for the si4703 to clear the STC as well
  while(1) {
    fm_readRegisters();
    if( (si4703_registers[STATUSRSSI] & (1<<STC)) == 0) break; //Tuning complete!
  }

  clearRDSBuff();

return (!valueSFBL);
}

static inline void considerrdschar(char * buf, int place, char ch) {
	if (ch < FIRST_ALLOWED_CHAR || ch > LAST_ALLOWED_CHAR)
		return;

	buf[place] = ch;
}


int fm_readRDS(char* ps, char* rt)
{ 
	fm_readRegisters();
	if(si4703_registers[STATUSRSSI] & (1<<RDSR)) {
		if (fakerds) {
			memset(rdsdata, ' ', 8);
			rdschanged = 1;
		}
		
		fakerds = 0;

		//const uint16_t a = si4703_registers[RDSA];
		const uint16_t b = si4703_registers[RDSB];
		const uint16_t c = si4703_registers[RDSC];
		const uint16_t d = si4703_registers[RDSD];		
				
		const uint8_t groupid = (b & 0xF000) >> 12;
		//uint8_t version = b & 0x10;
		switch(groupid) {
			case RDS_PS: {
				const uint8_t index = (b & 0x3)*2;
				char Dh = (d & 0xFF00) >> 8;
				char Dl = d;

				considerrdschar(rdsdata, index, Dh);
				considerrdschar(rdsdata, index +1, Dl);
		
				rdschanged = 1;
			};
			break;
			case RDS_RT: {
				const uint8_t index = (b & 0xF)*4;

				char Ch = (c & 0xFF00) >> 8;
				char Cl = c;
				char Dh = (d & 0xFF00) >> 8;
				char Dl = d;

				considerrdschar(radiotext, index, Ch);
				considerrdschar(radiotext, index +1, Cl);
				considerrdschar(radiotext, index +2, Dh);
				considerrdschar(radiotext, index +3, Dl);
			};
			break;
		}
	}
      			
	const int change = rdschanged;
	if (change) {
		strcpy(ps, rdsdata);
		strcpy(rt, radiotext);
	}
	rdschanged = 0;
	return (change) ? ((fakerds) ? (RDS_FAKE) : (RDS_AVAILABLE)) : (RDS_NO);
}
