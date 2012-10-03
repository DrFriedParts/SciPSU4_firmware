#include <avr/io.h>
#include <avr/interrupt.h>
#include "utilities.h"
#include "pwr_main.h"
#include "brain.h"
#include "ui.h"

//Handles control of the main power channels

void init_pwr_main(){
	//LED's are located at PF0 (Lower) and PF1 (Upper)
	PORTC.DIRSET = B8(00110011); //pins 0,1,4,5 to output
	PORTC.OUTCLR = B8(00110011); //pins 0,1,4,5 to low output value (off)
	pwr_main_off(CHANNEL_ALL);				
}

//#############################################################
//## OUTPUT MUTE
//#############################################################

void inline pwr_main_on(uint8_t which){
	uint8_t mask;
	switch(which){
		case CHANNEL_A:
			PORTC.OUTSET = B8(00000001);
			break;
		case CHANNEL_B:
			PORTC.OUTSET = B8(00000010);
			break;
		case CHANNEL_C:
			PORTC.OUTSET = B8(00010000);
			break;
		case CHANNEL_D:
			PORTC.OUTSET = B8(00100000);
			break;
		case CHANNEL_ALL:
			PORTC.OUTSET = B8(00110011);
			break;
		case CHANNEL_RESTORE:
			mask = STATE_power_channels >> 2; //[000000DC] remove lower channels
			mask = mask << 4; //[00DC0000] position upper channels
			mask |= (STATE_power_channels & B8(00000011)); //[00DC00BA] merge in lower channels
			PORTC.OUTSET = mask;
			break;
	}
}

void inline pwr_main_off(uint8_t which){
	switch(which){
		case CHANNEL_A:
		PORTC.OUTCLR = B8(00000001);
		break;
		case CHANNEL_B:
		PORTC.OUTCLR = B8(00000010);
		break;
		case CHANNEL_C:
		PORTC.OUTCLR = B8(00010000);
		break;
		case CHANNEL_D:
		PORTC.OUTCLR = B8(00100000);
		break;
		case CHANNEL_ALL:
		PORTC.OUTCLR = B8(00110011);
		break;
	}
}

void service_pwr_main(){
}

//#############################################################
//## OUTPUT SENSING
//#############################################################
