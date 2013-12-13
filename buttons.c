#include "buttons.h"
#include "config.h"
#include <avr/cpufunc.h>

#define SETH(reg,id) reg |= (1 << id)
#define SETL(reg,id) reg &= ~(1 << id)
#define GETSTATE(what, id) (( what & (1<<id)) != 0)

#define KB_D 0
#define KB_E 1
#define KB_F 2
#define KB_G 3
#define KB_H 4
#define KB_J 5
#define KB_K 6
const int btn_mask = (1 << KB_K) | (1 << KB_J) | (1 << KB_H) | (1 << KB_G);
uint8_t btn_last = 0;

// read from config BUTTONS_PORT and BUTTONS_DDR. Buttons should be connected from 0 to 7
void buttons_init(void) {
	BUTTONS_DDR = 0;

	SETH(BUTTONS_DDR, KB_D);
	SETH(BUTTONS_DDR, KB_E);
	SETH(BUTTONS_DDR, KB_F);

	BUTTONS_PORT = 0;
	SETH(BUTTONS_PORT, KB_D);
	SETH(BUTTONS_PORT, KB_E);
	SETH(BUTTONS_PORT, KB_F);

	_NOP();
}

inline uint8_t getState(uint8_t D, uint8_t E, uint8_t F) {
	if (D) SETH(BUTTONS_PORT, KB_D); else SETL(BUTTONS_PORT, KB_D);
	if (E) SETH(BUTTONS_PORT, KB_E); else SETL(BUTTONS_PORT, KB_E);
	if (F) SETH(BUTTONS_PORT, KB_F); else SETL(BUTTONS_PORT, KB_F);
	_NOP();

	return BUTTONS_PIN;	
}

// returns ascii char pressed or 0 on failure
uint8_t buttons_poll(uint8_t * val, uint8_t * event) {
	uint8_t state = BUTTONS_PIN;
	uint8_t result = 0;

	// if no button has been pressed, otherwise
	if ((state & btn_mask) == 0)
		return 0;
	
	state = getState(1, 0, 0);
	if (GETSTATE(state, KB_K))
		result = '3';
	else if (GETSTATE(state, KB_J))
		result = '6';
	else if (GETSTATE(state, KB_H))
		result = '9';
	else if (GETSTATE(state, KB_G))
		result = '#';

	if (result == 0) {
		state = getState(0, 1, 0);
		if (GETSTATE(state, KB_K))
			result = '2';
		else if (GETSTATE(state, KB_J))
			result = '5';
		else if (GETSTATE(state, KB_H))
			result = '8';
		else if (GETSTATE(state, KB_G))
			result = '0';

		if (result == 0) {
		state = getState(0, 0, 1);
			if (GETSTATE(state, KB_K))
				result = '1';
			else if (GETSTATE(state, KB_J))
				result = '4';
			else if (GETSTATE(state, KB_H))
				result = '7';
			else if (GETSTATE(state, KB_G))
				result = '*';
		}
	}

	SETH(BUTTONS_PORT, KB_D);
	SETH(BUTTONS_PORT, KB_E);
	SETH(BUTTONS_PORT, KB_F);
	_NOP();

	if (result == 0) {
		return 0;
	} else if (result != btn_last) {
		*event = (btn_last == 0) ? BUTTONS_DOWN : BUTTONS_UP;
		*val = (btn_last == 0) ? result : btn_last;
		btn_last = result;
		return 1;
	} else {
		*event = BUTTONS_PRESSED;
		*val = result;
		return 1;	
	}
}
