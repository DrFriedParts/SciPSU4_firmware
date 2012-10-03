#include <avr/io.h>
#include <avr/interrupt.h>
#include "utilities.h"
#include "brain.h"
#include "quadrature.h"
#include "ui.h" //debugging

//Quadrature Decoder (linear strip, rotary optical encoder, rotary switches, etc)

void init_quadrature(){
	/*
		Resource Map
		============		
		PK6: Rotary Channel A
		PK7: Rotary Channel B
	*/
	//	SciPSU FP switches have hardware pull-up and hardware debounce
	PORTK.DIRCLR = B8(11000000); //This is the default condition, but just to be safe
	PORTK.INT0MASK = B8(01000000); //Enable PORTK.Interrupt0 channel for PK6 
	PORTK.INT1MASK = B8(10000000); //Enable PORTK.Interrupt1 channel for PK7 
	PORTK.INTCTRL = B8(00001111); //interrupt 0 & 1 channels set to highest priority
	//Setup initial edge look directions -- need to enable global interrupts shortly after doing this (so init the quadrature module last in main.c)
	if ((PORTK.IN & _BV(6)) == 0){PORTK.PIN6CTRL = RISING_EDGE;} 
	else {PORTK.PIN6CTRL = FALLING_EDGE;}
	if ((PORTK.IN & _BV(7)) == 0){PORTK.PIN7CTRL = RISING_EDGE;}
	else {PORTK.PIN7CTRL = FALLING_EDGE;}
		
	quad_count = 0;
	quad_state = QUAD_IDLE;
	
}

/// Reports if the quadrature encoded control has moved up since the last time this function was called.
/* SIDE EFFECT: modifies internal state. If you call this twice, only the first call will report true.*/
boolean quad_up(){
	if (quad_state == QUAD_UP){quad_state = QUAD_IDLE; return true;}
	return false;
}

/// Reports if the quadrature encoded control has moved down since the last time this function was called.
/* SIDE EFFECT: modifies internal state. If you call this twice, only the first call will report true.*/
boolean quad_down(){
if (quad_state == QUAD_DOWN){quad_state = QUAD_IDLE; return true;}
return false;
}


//#############################################################
//## ROTARY CONTROL
//#############################################################

///Interrupt Service Routine (ISR) for quadrature encoder Channel A (PORTK Int0; PK6)
SIGNAL(PORTK_INT0_vect){
	if (PORTK.PIN6CTRL == FALLING_EDGE){
		//Detected FALLING edge on channel A
		if ((PORTK.IN & _BV(7)) == 0){
			//if channel A is falling and channel B is low, direction = DOWN
			quad_count--;
			quad_state = QUAD_DOWN;
		}
		else{
			//if channel A is falling and channel B is high, direction = UP
			quad_count++;
			quad_state = QUAD_UP;
		}
		//Now, look for rising edge
		PORTK.PIN6CTRL = RISING_EDGE;
	}
	if (PORTK.PIN6CTRL == RISING_EDGE){
		//Detected RISING edge on channel A
		if ((PORTK.IN & _BV(7)) == 0){
			//if channel A is rising and channel B is low, direction = UP
			quad_count++;
			quad_state = QUAD_UP;
		}
		else{
			//if channel A is rising and channel B is high, direction = DOWN
			quad_count--;
			quad_state = QUAD_DOWN;
		}
		//Now, look for falling edge
		PORTK.PIN6CTRL = FALLING_EDGE;
	}
	PORTK.INTFLAGS = B8(00000001); //clear interrupt flag just in case
}

///Interrupt Service Routine (ISR) for quadrature encoder Channel B (PORTK Int1; PK7)
SIGNAL(PORTK_INT1_vect){
	if (PORTK.PIN7CTRL == FALLING_EDGE){
		//Detected FALLING edge on channel A
		if ((PORTK.IN & _BV(6)) == 0){
			//if channel B is falling and channel A is low, direction = UP
			quad_count++;
			quad_state = QUAD_UP;
		}
		else{
			//if channel B is falling and channel A is high, direction = DOWN
			quad_count--;
			quad_state = QUAD_DOWN;
		}
		//Now, look for rising edge
		PORTK.PIN7CTRL = RISING_EDGE;
	}
	if (PORTK.PIN7CTRL == RISING_EDGE){
		//Detected RISING edge on channel B
		if ((PORTK.IN & _BV(6)) == 0){
			//if channel B is rising and channel A is low, direction = DOWN
			quad_count--;
			quad_state = QUAD_DOWN;
		}
		else{
			//if channel B is rising and channel A is high, direction = UP
			quad_count++;
			quad_state = QUAD_UP;
		}
		//Now, look for falling edge
		PORTK.PIN7CTRL = FALLING_EDGE;
	}
	PORTK.INTFLAGS = B8(00000010); //clear interrupt flag just in case
}





//#############################################################
//## SERVICE ROUTINE
//#############################################################

void service_quadrature(){	
	
}
