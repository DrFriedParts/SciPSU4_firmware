#include <avr/io.h>
#include <avr/interrupt.h>
#include "utilities.h"
#include "ui.h"

/*
 * thermal.c
 * Thermal management components
 * Created: 9/24/2012 12:02:40 AM
 *  Author: CircuitHub
 */ 


void init_thermal(){
	//FANS are located at PF4 (Fan0) and PF5 (Fan1)
	PORTF.DIRSET = 0x30; //pins 4 and 5 to output
	PORTF.OUTSET &= B8(11001111); //pins 4 and 5 to low (off)
		
	TCF1.CTRLA = 0x07; //enable; div1024
	TCF1.CTRLB = 0x13; //Output Channel A enable; Single-slope PWM
	TCF1.PER = 0x00FF; //Set the top of the counter to basically force 8 bit operation; we do this for speed when calling dimming functions in the future
	TCF1.CCA = 0x0010; //Default to off-level brightness
}
