#ifndef __eeprom_h
#define __eeprom_h
	
	#include "pwr_adjust.h"
	#include <avr/eeprom.h>
	
	//DEFINITIONS (APPEND, DO NOT CHANGE ORDER OR SIZES!)
		//Hold center knob approximately 2 seconds to save
		#define EEPROM_HOLD_TO_SAVE_TIME 1000
	
	//FUNCTIONS
		void init_eeprom();
		void eeprom_save();
		void eeprom_restore();
		uint8_t eeprom_read();
		void service_eeprom();
#endif
