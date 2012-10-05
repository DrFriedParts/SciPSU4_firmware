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
#include "lcd_console.h"

//Handles EarthLCD ezLCD301 hardware

//#############################################################
//## INITIALIZATION ROUTINE
//#############################################################

void init_lcd_console(){
	//ensure that string termination is present in the beginning for blank lines
	for(uint8_t i=0;i<LCD_CONSOLE_NUM_ROWS;i++){
		lcd_console[i][0] = 0;
		lcd_buffer_dirty[i] = false;
	}
	lcd_console_head = 0;
}

//#############################################################
//## SCREENS: CONSOLE
//#############################################################

void lcd_console_write(char* theString){
	uint8_t head = lcd_console_head;
	char* line_buffer = lcd_console[lcd_console_head];
	//Command
	line_buffer[0] = '8'; line_buffer[1]='8'; line_buffer[2]=' '; line_buffer[4]=' '; line_buffer[5]='"';
	//Destination
	for(uint8_t i=0;i<LCD_CONSOLE_NUM_ROWS;i++){
		lcd_console[head][3] = 0x31 + i; //id of static control (Row) to write to 
		head++;
		if(head >= LCD_CONSOLE_NUM_ROWS){head = 0;}
	}	
	//Content
	for (uint8_t i=0; i<LCD_CONSOLE_NUM_COLS; i++){
		if (theString[i] == 0){
			line_buffer[6+i] = '"';
			line_buffer[6+i+1] = 0;
			break;
		}
		line_buffer[6+i] = theString[i];
	}
	lcd_buffer_dirty[lcd_console_head] = true;
	//Move row pointer
	lcd_console_head++;
	if (lcd_console_head >= LCD_CONSOLE_NUM_ROWS){lcd_console_head = 0;}
}

 

//#############################################################
//## SERVICE ROUTINE
//#############################################################

//Used to refresh the console
void service_lcd_console(){
	static uint8_t next_row_to_output = 0;
	uint8_t num_rows_output = 0;
	if (STATE_menu != MENU_CONSOLE){return;}
	while((uart_count(&ulcd) < 512)&&(num_rows_output<LCD_CONSOLE_NUM_ROWS)){
		if(lcd_buffer_dirty[next_row_to_output]){
			lcd_command(lcd_console[next_row_to_output]);
			lcd_buffer_dirty[next_row_to_output] = false;
		}
		next_row_to_output++; num_rows_output++;
		if(next_row_to_output>=LCD_CONSOLE_NUM_ROWS){next_row_to_output=0;}
	}
}