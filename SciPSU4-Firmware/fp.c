#include <avr/io.h>
#include <avr/interrupt.h>
#include "utilities.h"
#include "brain.h"
#include "fp.h"
#include "ui.h" //access to the "Master" LED is provided in ui.h because it shares a timer with internal UI components (motherboard-mounted LED's and Audio)
#include "quadrature.h"

//Handles all FRONT PANEL UI hardware (buttons, switches, and rotary controls)
//LCD is handled separately

//LED's are handled with inverted logic so that the MASK function is easier to work with

void init_fp(){
	/*
		Resource Map
		============
		PJ1 - PJ4: LED A - LED D		 
		PK0 - PK3: Switch A - Switch D
		PK4: Master Switch
		PK5: Rotary Center Switch
		PK6: Rotary Channel A
		PK7: Rotary Channel B
				
	*/
	//LED's
	PORTJ.PIN1CTRL = B8(01000000); //invert pin logic
	PORTJ.PIN2CTRL = B8(01000000); //invert pin logic
	PORTJ.PIN3CTRL = B8(01000000); //invert pin logic
	PORTJ.PIN4CTRL = B8(01000000); //invert pin logic
	PORTJ.OUTCLR = B8(00011110); //pins to low (LED's off, remember: inverted!)	
	PORTJ.DIRSET = B8(00011110); //pins to output	
	
	fp_channel_mask = 0x00;
	
	//Switches
	//	SciPSU FP switches have hardware pull-up and hardware debounce
	PORTK.DIRCLR = B8(00111111); //This is the default condition, but just to be safe
}

//#############################################################
//## LEDs
//#############################################################

// Uses generic defines for channel (see main.h)
void fp_led_enable(uint8_t which){
	switch(which){
		case CHANNEL_A:
			PORTJ.OUTSET = B8(00000010); //PJ1
			break;
		case CHANNEL_B:
			PORTJ.OUTSET = B8(00000100); //PJ2
			break;
		case CHANNEL_C:
			PORTJ.OUTSET = B8(00001000); //PJ3
			break;
		case CHANNEL_D:
			PORTJ.OUTSET = B8(00010000); //PJ4
			break;			
		case CHANNEL_M:
			PORTJ.OUTSET = STATE_power_channels << 1; //left shift 1 to move [0-3] state to [1-4] pin locations
			led_on(LED_3); //Master LED is dimmable so its over in the ui.h module
			break;
	}
}

void fp_led_disable(uint8_t which){
	switch(which){
		case CHANNEL_A:
			PORTJ.OUTCLR = B8(00000010); //PJ1
			break;
		case CHANNEL_B:
			PORTJ.OUTCLR = B8(00000100); //PJ2
			break;
		case CHANNEL_C:
			PORTJ.OUTCLR = B8(00001000); //PJ3
			break;
		case CHANNEL_D:
			PORTJ.OUTCLR = B8(00010000); //PJ4
			break;
		case CHANNEL_M:
			fp_counter = 0; fp_counter2 = 0; fp_updown = 1;
			break;
	}
}


//#############################################################
//## BUTTONS -- Polling
//#############################################################

//t0 = most recent --> t2 = oldest values
void _fp_read_switches(){
	//rotate
	fp_button_time2 = fp_button_time1;
	fp_button_time1 = fp_button_time0;
	fp_button_time0 = PORTK.IN;
}

//Button was pressed
void _fp_switch_pressed(uint8_t which){
	audio_beep(1, 10);
	switch(which){
		case FP_SWITCH_A:
			brain_power(CHANNEL_A);			
			break;
		case FP_SWITCH_B:
			brain_power(CHANNEL_B);
			break;
		case FP_SWITCH_C:
			brain_power(CHANNEL_C);
			break;
		case FP_SWITCH_D:
			brain_power(CHANNEL_D);
			break;
		case FP_SWITCH_M:
			brain_power_master();
			break;
		case FP_SWITCH_R:
			brain_debug(); //xxx
			break;		
	}
}

//Button was released
void _fp_switch_released(uint8_t which){
	switch(which){
		case FP_SWITCH_A:
			break;
		case FP_SWITCH_B:
			break;
		case FP_SWITCH_C:
			break;
		case FP_SWITCH_D:
			break;
		case FP_SWITCH_M:
			break;
		case FP_SWITCH_R:
			break;
	}
}

void _process_switch(uint8_t current, uint8_t change, uint8_t which){
	if (current != 0) {return;} //unstable -- wait for things to settle
	if ((change & _BV(which)) == 0){return;} //no change
	//Pin has been changed!
	if ((fp_button_time0 & _BV(which)) == 0){_fp_switch_pressed(which);}
	else {_fp_switch_released(which);}
}

void _fp_process_switches(){
	uint8_t current = fp_button_time0 ^ fp_button_time1;
	uint8_t change = current ^ fp_button_time2;
	_process_switch(current, change, FP_SWITCH_A);
	_process_switch(current, change, FP_SWITCH_B);
	_process_switch(current, change, FP_SWITCH_C);
	_process_switch(current, change, FP_SWITCH_D);
	_process_switch(current, change, FP_SWITCH_M);
	_process_switch(current, change, FP_SWITCH_R);
}




//#############################################################
//## ROTARY CONTROL
//#############################################################

inline void _fp_process_rotary(){
	static int16_t blanking_counter = -1;
	
	//Software blank rotary dial in menu navigation mode
	if (STATE_menu != MENU_DIAL){
		if (blanking_counter >= 0){
			blanking_counter++;
			quad_up(); quad_down(); //clear state changes during blanking period
			if (blanking_counter > 500){blanking_counter = -1;}
		}
		else {
			if (quad_up()) {blanking_counter = 0; brain_menu_change(QUAD_UP);}
			if (quad_down()) {blanking_counter = 0; brain_menu_change(QUAD_DOWN);}
		}		
	}
	//Use full dial resolution in adjustment mode
	else {
		if (quad_up()) {brain_menu_change(QUAD_UP);}
		if (quad_down()) {brain_menu_change(QUAD_DOWN);}
	}	
}



//#############################################################
//## SERVICE ROUTINE
//#############################################################

void service_fp(){	
	uint8_t fp_channel_mask = STATE_power_channels << 1;
	//LEDs
	if (STATE_power_output == DISABLE){
				
		//Channel LED's			
		if (fp_counter == 0) {PORTJ.OUTSET = fp_channel_mask;} 
		if (fp_counter == FP_ON_LENGTH) {PORTJ.OUTCLR = fp_channel_mask;}
		fp_counter++;
		if (fp_counter >= FP_ON_LENGTH + FP_OFF_LENGTH) {fp_counter = 0;}
			
		//Master LED
		fp_counter2 += fp_updown;
		if (fp_counter2 < 128) {
			if (fp_counter2 % 2 == 0){
				led_dim(LED_3, fp_counter2);
			}
		}
		else {
			if (fp_updown > 0) {fp_counter2 += 2;}
			else {fp_counter2 += 2;}
			led_dim(LED_3, fp_counter2);
		}			
		if ((fp_counter2 >= 255) || (fp_counter2 <= 0)) {
			fp_updown = -1 * fp_updown;
		}
	}
	
	//SWITCHes
	_fp_read_switches();
	_fp_process_switches();
	_fp_process_rotary();
}
