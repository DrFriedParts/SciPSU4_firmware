#include <avr/io.h>
#include <avr/interrupt.h>
#include "utilities.h"
#include <avr/eeprom.h>
#include "eeprom.h"
#include "pwr_adjust.h"
#include "brain.h"

//Handles non-volatile memory (store/load config data through power-off)
//Blocks on read/write (non-ISR based)
//http://www.google.com/url?sa=t&rct=j&q=&esrc=s&source=web&cd=1&cad=rja&ved=0CB8QFjAA&url=http%3A%2F%2Fwww.teslabs.com%2Fopenplayer%2Fdocs%2Fdocs%2Fprognotes%2FEEPROM%2520Tutorial.pdf&ei=OmlyUJ-aH6bAiwLnsIGwDA&usg=AFQjCNEaFVSAXVm66KdG1BUxwXPCzv24ZA&sig2=WBp_OfnVAHFZUW_48YAU7Q


//#############################################################
//## EEPROM PAGE STRUCTURE
//#############################################################

uint16_t EEMEM nv_pwr_adj_values[PWR_ADJ_NUM_CHANNELS]; //the values on the control channel
uint8_t EEMEM nv_STATE_power_channels; //Which channels are enabled; Channel map: B8(0000DCBA), ex. bit B=0 channel B disabled, B=1 channel B output (possibly, gated by master) enabled



//#############################################################
//## Initialization
//#############################################################

void init_eeprom(){
	eeprom_restore();
}



//#############################################################
//## API
//#############################################################

void eeprom_save(){
	eeprom_write_byte(&nv_STATE_power_channels, STATE_power_channels);
	eeprom_write_block((void*)&pwr_adj_values, (void*)&nv_pwr_adj_values, PWR_ADJ_NUM_CHANNELS*2); //multiple size by 2 since uint16_t = 2 bytes
}

void eeprom_restore(){
	STATE_power_channels = eeprom_read_byte(&nv_STATE_power_channels);
	eeprom_read_block((void*)&pwr_adj_values, (void*)&nv_pwr_adj_values, PWR_ADJ_NUM_CHANNELS*2);
}

//mostly for debugging
uint8_t eeprom_read(){
	return eeprom_read_byte(&nv_STATE_power_channels);
}

//#############################################################
//## Service
//#############################################################

//2ms Service Loop -- Call once every 2ms
void service_eeprom(){
	
}
