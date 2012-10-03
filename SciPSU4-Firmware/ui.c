#include <avr/io.h>
#include <avr/interrupt.h>
#include "utilities.h"
#include "ui.h"

//Handles all internal UI hardware (2 LEDs + Audio Beeper)

void init_ui(){
	//LED's are located at PF0 (Lower) and PF1 (Upper)
	PORTF.DIRSET = B8(00001111); //pins 0,1,2,3 to output	
	PORTF.PIN0CTRL = B8(01000000); //Invert the pin (needed to achieve correct PWM output polarity)
	PORTF.PIN1CTRL = B8(01000000); //Invert the pin (needed to achieve correct PWM output polarity)
	PORTF.PIN3CTRL = B8(01000000); //Invert the pin (needed to achieve correct PWM output polarity)
	TCF0.CTRLA = 0x07; //enable; div1024
	TCF0.CTRLB = B8(11110011); //All output channels enabled (A through D); Single-slope PWM
	TCF0.PER = 0x00FF; //Set the top of the counter to basically force 8 bit operation; we do this for speed when calling dimming functions in the future
	audio_volume(0x00);
	led_off(LED_0);
	led_off(LED_1);
	led_off(LED_3);			
}

//#############################################################
//## LEDs
//#############################################################

void inline led_on(uint8_t which){led_dim(which, 0xff);}

void inline led_off(uint8_t which){led_dim(which, 0x00);}

void inline led_dim(uint8_t which, uint8_t brightness){
	switch(which){
	case LED_0:
		TCF0.CCABUF = (uint16_t)brightness;		
		break;
	case LED_1:
		TCF0.CCBBUF = (uint16_t)brightness;		
		break;
	case LED_3:
		TCF0.CCDBUF = (uint16_t)brightness;
		break;
	}
}

void inline led_toggle(uint8_t which){
	switch(which){
	case LED_0:
		TCF0.CCAL = ~TCF0.CCAL;		
		break;
	case LED_1:
		TCF0.CCBL = ~TCF0.CCBL;		
		break;
	case LED_3:
		TCF0.CCDL = ~TCF0.CCDL;
		break;
	}
}

//#############################################################
//## AUDIO BEEPER
//#############################################################

uint8_t _audio_num_beeps;
uint8_t _audio_volume;
uint8_t _audio_counter;
uint8_t _audio_state = AUDIO_IDLE;

void audio_volume(uint8_t volume){
	TCF0.CCCBUF = (uint16_t)volume;	
}

//Internal function to actually start making noise
void _audio_beep(){
	_audio_num_beeps--;
	_audio_state = AUDIO_BEEPING;
	_audio_counter = AUDIO_BEEP_LENGTH;
	audio_volume(_audio_volume); //actually start making noise
}	

//[INTERFACE] This is the function to call from outside to make beepy noises
void audio_beep(uint8_t num_beeps, uint16_t volume){
	if (num_beeps < 1) return; //ignore request if no beeps requested
	_audio_num_beeps = num_beeps;
	if (volume == 0) return; //ignore request if volume level would be inaudible anyway
	_audio_volume = volume;
	_audio_beep();
}

//2ms Service Loop -- Call once every 2ms
void service_audio(){
	switch(_audio_state){
		case AUDIO_BEEPING:
			_audio_counter--;
			if (_audio_counter <= 0) {
				//End of this beep!
				audio_volume(0x00); //silence!
				if (_audio_num_beeps == 0) {
					//End of this beep sequence -- ALL DONE! =)					
					_audio_state = AUDIO_IDLE;
				}
				else {
					//start quiet period between beeps
					_audio_state = AUDIO_BETWEEN;
					_audio_counter = AUDIO_GAP_LENGTH;
				}
			}
			break;
		case AUDIO_BETWEEN:
			_audio_counter--;
			if (_audio_counter <= 0) {
				//End of gap
				_audio_beep();
			}
			break;
		case AUDIO_IDLE:
		default:
			audio_volume(0x00); //Silence!
			return;
	}
}
