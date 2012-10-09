#include <avr/io.h>
#include <avr/interrupt.h>
#include "utilities.h"
#include "ui.h"
#include "adc.h"
#include <stddef.h>
#include <avr\pgmspace.h>
#include <math.h>
#include <float.h>
#include <stdlib.h>
#include "lcd_console.h"
#include "uart_buffer.h"
#include "uart.h"
#include "brain.h"

//Handles the ADC


//#############################################################
//## ADCs -- INITIALIZATION
//#############################################################

void init_adc(){	
	//ADC A
	ADCA.CALL = adc_read_cal_byte( offsetof(NVM_PROD_SIGNATURES_t, ADCACAL0) );
	ADCA.CALH = adc_read_cal_byte( offsetof(NVM_PROD_SIGNATURES_t, ADCACAL1) );
	ADCA.PRESCALER = B8(00000111); //ADC clock = Peripheral clock / 512 (maximum resolution/accuracy)
	ADCA.REFCTRL = B8(00010011); //Use AVCC/1.6 = 2.063V as reference (highest allowed); Enable BandGap Reference and Temperature Sensor (internal)
	ADCA.CTRLB = B8(00000110); //12bit-left adjusted; One-shot conversion; unsigned mode;
	ADCA.CTRLA = B8(00000001); //Enable ADC;
	ADCA.CH0.CTRL = B8(00000001); //Single ended input mode; No gain
	ADCA.CH1.CTRL = B8(00000001); //Single ended input mode; No gain
	ADCA.CH2.CTRL = B8(00000001); //Single ended input mode; No gain
	ADCA.CH3.CTRL = B8(00000001); //Single ended input mode; No gain
	
	//ADC B
	ADCB.CALL = adc_read_cal_byte( offsetof(NVM_PROD_SIGNATURES_t, ADCBCAL0) );
	ADCB.CALH = adc_read_cal_byte( offsetof(NVM_PROD_SIGNATURES_t, ADCBCAL1) );
	ADCB.PRESCALER = B8(00000111); //ADC clock = Peripheral clock / 512 (maximum resolution/accuracy)
	ADCB.REFCTRL = B8(00010011); //Use AVCC/1.6 = 2.063V as reference (highest allowed); Enable BandGap Reference and Temperature Sensor (internal)
	ADCB.CTRLB = B8(00000110); //12bit-left adjusted; One-shot conversion; unsigned mode;
	ADCB.CTRLA = B8(00000001); //Enable ADC;
	ADCB.CH0.CTRL = B8(00000001); //Single ended input mode; No gain
	ADCB.CH1.CTRL = B8(00000001); //Single ended input mode; No gain
	ADCB.CH2.CTRL = B8(00000001); //Single ended input mode; No gain
	ADCB.CH3.CTRL = B8(00000001); //Single ended input mode; No gain
	
	//STATE
	adc_bank = 0;
	adc_head = 0;
	adc_bank_select(adc_bank);
	
	//FLUSH
	ADCA.CTRLA |= 0x02;
	ADCB.CTRLB |= 0x02;
	adc_convert();
}

///http://www.bostonandroid.com/manuals/xmega-precision-adc-howto.html
uint8_t adc_read_cal_byte( uint8_t index ){
	uint8_t result;
	/* Load the NVM Command register to read the calibration row. */
	NVM_CMD = NVM_CMD_READ_CALIB_ROW_gc;
	result = pgm_read_byte(index);
	/* Clean up NVM Command register. */
	NVM_CMD = NVM_CMD_NO_OPERATION_gc;
	return( result );
}



//#############################################################
//## ADCs -- OPERATION
//#############################################################

void adc_bank_select(uint8_t which){
	if(which % 2){
		ADCA.CH0.MUXCTRL = B8(00000000); //CH0 converts from PA0
		ADCA.CH1.MUXCTRL = B8(00001000); //CH1 converts from PA1
		ADCA.CH2.MUXCTRL = B8(00010000); //CH2 converts from PA2
		ADCA.CH3.MUXCTRL = B8(00011000); //CH3 converts from PA3
		ADCB.CH0.MUXCTRL = B8(00000000); //CH0 converts from PB0
		ADCB.CH1.MUXCTRL = B8(00001000); //CH1 converts from PB1
		ADCB.CH2.MUXCTRL = B8(00010000); //CH2 converts from PB2
		ADCB.CH3.MUXCTRL = B8(00011000); //CH3 converts from PB3
	}
	else {
		ADCA.CH0.MUXCTRL = B8(00100000); //CH0 converts from PA4
		ADCA.CH1.MUXCTRL = B8(00101000); //CH1 converts from PA5
		ADCA.CH2.MUXCTRL = B8(00110000); //CH2 converts from PA6
		ADCA.CH3.MUXCTRL = B8(00111000); //CH3 converts from PA7
		ADCB.CH0.MUXCTRL = B8(00100000); //CH0 converts from PB4
		ADCB.CH1.MUXCTRL = B8(00101000); //CH1 converts from PB5
		ADCB.CH2.MUXCTRL = B8(00110000); //CH2 converts from PB6
		ADCB.CH3.MUXCTRL = B8(00111000); //CH3 converts from PB7	
	}
}	

void adc_convert(){
	ADCA.CTRLA |= B8(00111100); //Start conversions on all four channels
	ADCB.CTRLA |= B8(00111100); //Start conversions on all four channels
}

//#############################################################
//## ADCs -- DATA PROCESSING
//#############################################################

//Float-to-ASCII: uses scaler to shift position, does not include fractional component, null-terminates result, handles pos & neg numbers
void adc_ftoa(float x, uint16_t scaler, char* result){
	x = x * scaler;
	utoa((int)x, result, 10);
}

///result = char[6] = 15324\0 --> 15.324 V or A (don't forget string terminator byte)
void adc_data(uint8_t channel_num, uint8_t measurement_type, char* result){
	uint16_t current_offset;
	
	//Init
		uint16_t summation = 0;
		float voltage;
		
	//Average
		for (uint8_t i=0;i<ADC_NUM_CONVERSIONS;i++){
			summation += adc_results[channel_num][i];
		}
		summation = summation >> 3; //divide by 8
		
	//Offset
		switch (measurement_type){
			case VOLTAGE_POS:
				if(summation<CODE_ZERO){summation = 0;}
				else{summation -= CODE_ZERO;}
				break;
			case VOLTAGE_NEG:
				if(summation<VOLTAGE_NEG_OFFSET){summation = 0;}
				else{summation -= VOLTAGE_NEG_OFFSET;}
				break;
			case CURRENT_HI_RES:
				current_offset = adc_current_offset[(channel_num-1)>>1];
				if(summation<current_offset){summation=0;}
				else{summation -= current_offset;}
				break;
			case CURRENT_LO_RES:
				//TODO: add scaler for the low-res mode
				break;
		}
		
	//Format & Return
		switch (measurement_type){
			case VOLTAGE_POS:
				voltage = (float)summation * CODE_TO_VOLTS * VOLTAGE_DESCALE_FACTOR;
				adc_ftoa(voltage, 100, result);
				return;
			case VOLTAGE_NEG:
				voltage = (float)summation * CODE_TO_VOLTS * VOLTAGE_DESCALE_FACTOR;
				if (voltage < 2.2) voltage = 0; //blank anything inside the amp offset (can't actually offset this because it adds huge error to the linear fit)
				adc_ftoa(voltage, 100, result);
				return;
			case CURRENT_HI_RES:
				adc_ftoa((float)summation * CODE_TO_AMPS, 1000, result);
				return;
			case CURRENT_LO_RES:
				//TODO: add scaler for the low-res mode
				return;
		}
}

//Returns the maximum value seen in the buffer for the specified channel
uint16_t adc_max(uint8_t channel_num){
	uint16_t maxValue = 0;
	for(uint8_t i=0;i<ADC_NUM_CONVERSIONS;i++){
		if (adc_results[channel_num][i]>maxValue){maxValue=adc_results[channel_num][i];};
	}		
	return maxValue;
}

//#############################################################
//## ADCs -- SERVICE
//#############################################################

uint16_t adjust(uint16_t theValue){
	uint16_t working = theValue;
	working = theValue >> 4;
	if (working < CODE_ZERO){working = CODE_ZERO;}; //clip on overflow
	return working;
}

// Implemented with polling for conversion complete
void service_adc(){	
	//Did all conversions complete?
	if ((ADCA.INTFLAGS == 0x0F) && (ADCB.INTFLAGS == 0x0F)){
		//Write Results
		if(adc_bank % 2){
			//lower bank
			adc_results[0][adc_head] = adjust(ADCA.CH0RES); //PSU Channel A
			adc_results[1][adc_head] = adjust(ADCA.CH1RES);
			adc_results[2][adc_head] = adjust(ADCA.CH2RES);
			adc_results[3][adc_head] = adjust(ADCA.CH3RES);
			adc_results[8][adc_head] = adjust(ADCB.CH0.RES); //PSU Channel C
			adc_results[9][adc_head] = adjust(ADCB.CH1.RES);
			adc_results[10][adc_head] = adjust(ADCB.CH2.RES);
			adc_results[11][adc_head] = adjust(ADCB.CH3.RES);
		}
		else {
			//upper bank
			adc_results[4][adc_head] = adjust(ADCA.CH0.RES); //PSU Channel B
			adc_results[5][adc_head] = adjust(ADCA.CH1.RES);
			adc_results[6][adc_head] = adjust(ADCA.CH2.RES);
			adc_results[7][adc_head] = adjust(ADCA.CH3.RES);
			adc_results[12][adc_head] = adjust(ADCB.CH0.RES); //PSU Channel D
			adc_results[13][adc_head] = adjust(ADCB.CH1RES);
			adc_results[14][adc_head] = adjust(ADCB.CH2RES);
			adc_results[15][adc_head] = adjust(ADCB.CH3RES);
			
			//next column
			adc_head++;
			if (adc_head >= ADC_NUM_CONVERSIONS){adc_head=0;}
		}
		
		//Clear Flags to Reset ADC
		ADCA.INTFLAGS = 0x0F;
		ADCB.INTFLAGS = 0x0F;
		
		//Toggle Channel Bank
		adc_bank++;
		adc_bank_select(adc_bank);
		
		//Start Next Round of Conversions
		adc_convert();
		
		//Auto-calibrate Current Offsets (when output is disabled)
		if(STATE_power_output == DISABLE){
			for(uint8_t i=0;i<(ADC_NUM_CHANNELS>>1);i++){
				adc_current_offset[i] = adc_max((i<<1)+1);
			}			
		}
	}
}
