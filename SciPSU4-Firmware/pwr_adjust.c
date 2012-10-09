#include <avr/io.h>
#include <avr/interrupt.h>
#include "utilities.h"
#include "brain.h"
#include "fp.h"
#include "ui.h"
#include "string.h" 
#include "uart.h"
#include "uart_buffer.h"
#include "lcd.h"
#include "lcd_touch.h"
#include "pwr_adjust.h"
#include "quadrature.h"
#include "adc.h"
#include <math.h>
#include <float.h>

//Handles Adjustable Variable Voltage Output Channels

//#############################################################
//## INITIALIZATION ROUTINE
//#############################################################

void init_pwr_adjust(){
	//Enable lines
	PORTH.OUTCLR = 0x00;
	PORTH.DIRSET = B8(00011111); //ADJ Outputs & 5V fixed output to out direction
	
	//Chip Select lines
	PORTD.OUTSET = B8(00011111); //set all output pins high prior to enabling output (deselect SPI peripherals)
	PORTD.DIRSET = B8(11111011); //ADJ_CS, TC_CS, SPI_MOSI, SPI_CLOCK to outputs	
	
	//SPI Configuration
	/* 
		MAX5494 requirements: 
		--Master reads on the rising clock edge
		--Clock idles HIGH
		--Shifted MSB-first 
		--Max clock = 7MHz
	*/
	SPID.CTRL = B8(11011101); //2X Clock (with 16x prescale) = 1/8 System Frequency = 4MHz; Master Mode; Clock idle high; Master sample on rising edge
	SPID.INTCTRL = 2; //medium priority interrupt
	
	pwr_adj_spi_state = PWR_ADJ_SPI_IDLE;
	pwr_adj_send_channels = 0x00;
	pwr_adj_channel_dirty = 0x00;
	pwr_adj_left_right = LCD_TOUCH_LEFT;
}

//#############################################################
//## SPI LOW-LEVEL FUNCTIONS
//#############################################################

void _pwr_adj_next(){
	pwr_adj_send_channels &= ~_BV(pwr_adj_channel_num_in_progress); //clear channel we just finished sending
	if (pwr_adj_send_channels > 0){
		//Some other channels need to be sent
		for(uint8_t i=0;i<PWR_ADJ_NUM_CHANNELS;i++){
			if((pwr_adj_send_channels & _BV(i)) > 0){
				pwr_adj_spi_send(i);
				return; //exit for
			}
		}
	}
}	
	
void pwr_adj_spi_send(uint8_t channel_num){
	//STATE
	pwr_adj_spi_state = PWR_ADJ_SPI_COMMAND;
	pwr_adj_channel_num_in_progress = channel_num;
	
	//CHIP SELECT
	switch(channel_num){
	case 0:
	case 1:
		PORTD.OUTCLR = _BV(0); //CS low, start transmitting
		break;
	case 2:
	case 3:
		PORTD.OUTCLR = _BV(1); //CS low, start transmitting
		break;
	case 4:
	case 5:
		PORTD.OUTCLR = _BV(2); //CS low, start transmitting
		break;
	case 6:
	case 7:
		PORTD.OUTCLR = _BV(3); //CS low, start transmitting
		break;
	}
	
	//SEND COMMAND BYTE
	if(channel_num % 2){
		//channel_num = 1,3,5,7 (0 == false in C)
		SPID.DATA = MAX5494_SET2;
	} 
	else {
		SPID.DATA = MAX5494_SET1;
	}		
}



 
//#############################################################
//## API
//#############################################################

void pwr_adj_on(uint8_t channel){
	switch(channel){
		case CHANNEL_A:
			PORTH.OUTSET = _BV(1);
			break;
		case CHANNEL_B:
			PORTH.OUTSET = _BV(2);
			break;
		case CHANNEL_C:
			PORTH.OUTSET = _BV(3);
			break;
		case CHANNEL_D:
			PORTH.OUTSET = _BV(4);
			break;
		case CHANNEL_ALL:
			PORTH.OUTSET = B8(00011110);
			break;
		case CHANNEL_RESTORE:
			PORTH.OUTSET = STATE_power_channels << 1; //[000DCBA0] align to port
			break;
	}
}
void pwr_adj_off(uint8_t channel){
	switch(channel){
		case CHANNEL_A:
			PORTH.OUTCLR = _BV(1);
			break;
		case CHANNEL_B:
			PORTH.OUTCLR = _BV(2);
			break;
		case CHANNEL_C:
			PORTH.OUTCLR = _BV(3);
			break;
		case CHANNEL_D:
			PORTH.OUTCLR = _BV(4);
			break;
		case CHANNEL_ALL:
			PORTH.OUTCLR = B8(00011110);
			break;
	}
}

//Will drop (ignore) this value change request if in the middle of writing a different value to the channel
void pwr_adj_change(uint8_t channel_num, uint16_t new_value){
	if ((pwr_adj_spi_state == PWR_ADJ_SPI_IDLE) || (pwr_adj_channel_num_in_progress != channel_num)){
		pwr_adj_channel_dirty |= _BV(channel_num); //mark this channel for update to screen
		if (new_value >= 1024){new_value = 1023;} //sanity check
		pwr_adj_values[channel_num] = new_value; //buffer value
		pwr_adj_send_channels |= _BV(channel_num); //flag channel for transmission
		if (pwr_adj_spi_state == PWR_ADJ_SPI_IDLE){pwr_adj_spi_send(channel_num);}
	}	
}

//QUAD_DOWN = CLOCKWISE, ergo it is more natural to make this INCREASE the counters
void pwr_adj_change_increment(uint8_t channel_num, uint8_t which_way){
	if ((which_way == QUAD_DOWN) && (pwr_adj_values[channel_num]<1024)) pwr_adj_values[channel_num]++;
	if ((which_way == QUAD_UP) && (pwr_adj_values[channel_num]>0)) pwr_adj_values[channel_num]--;
	pwr_adj_change(channel_num, pwr_adj_values[channel_num]);
}

//#############################################################
//## SELECTION AND TOUCH PANEL
//#############################################################

inline void pwr_adj_touch_dial(uint8_t which_one){
	pwr_adj_left_right = which_one;
}

//#############################################################
//## DISPLAY ROUTINES
//#############################################################

//Calculate the output for the first voltage divider
float _pwr_adj_data(uint8_t channel_num){
	return PWR_ADJ_SUPPLY_VOLTAGE * (float)pwr_adj_values[channel_num]/1023;
}

void pwr_adj_data(uint8_t channel_num, char* result){
	if ((channel_num % 2)==0){
		//channel_num is 0,2,4,etc...
		adc_ftoa(_pwr_adj_data(channel_num), 1000, result);
	}
	else{
		//channel_num is 1,3,5,etc...
		adc_ftoa(_pwr_adj_data(channel_num-1)*(float)pwr_adj_values[channel_num]/1023, 1000, result);		
	}
}	

void pwr_adj_control(uint8_t channel_num, char* result){
	utoa(pwr_adj_values[channel_num], result, 10);
}


//#############################################################
//## SERVICE ROUTINES
//#############################################################

ISR(SPID_INT_vect){
	led_on(LED_1);
	switch(pwr_adj_spi_state){
		case PWR_ADJ_SPI_COMMAND:
			//finished sending first byte... send next
			pwr_adj_spi_state = PWR_ADJ_SPI_DATAH;
			SPID.DATA = (uint8_t)(pwr_adj_values[pwr_adj_channel_num_in_progress] >> 2); //extract the 8 MSb's from the 10-bit number
			break;
		case PWR_ADJ_SPI_DATAH:
			//finished sending second byte... send next
			pwr_adj_spi_state = PWR_ADJ_SPI_DATAL;
			SPID.DATA = (uint8_t)(pwr_adj_values[pwr_adj_channel_num_in_progress]) & B8(00000011); //extract the 2 LSb's from the 10-bit number
			break;
		case PWR_ADJ_SPI_DATAL:
			//finished sending third byte... all done!
			//DESELECT
			switch(pwr_adj_channel_num_in_progress){
				case 0:
				case 1:
				PORTD.OUTSET = _BV(0); //CS high, done transmitting
				break;
				case 2:
				case 3:
				PORTD.OUTSET = _BV(1); //CS high, done transmitting
				break;
				case 4:
				case 5:
				PORTD.OUTSET = _BV(2); //CS high, done transmitting
				break;
				case 6:
				case 7:
				PORTD.OUTSET = _BV(3); //CS high, done transmitting
				break;
			}
			//Select Next Channel
			pwr_adj_spi_state = PWR_ADJ_SPI_IDLE;
			_pwr_adj_next();
			break;	
		case PWR_ADJ_SPI_IDLE:
		default:
			//do nothing
			break;
	}
}

void service_pwr_adjust(){	
	
}
