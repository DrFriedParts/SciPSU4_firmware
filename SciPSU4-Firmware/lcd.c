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
#include <stddef.h>

//Handles EarthLCD ezLCD301 hardware

//#############################################################
//## INITIALIZATION ROUTINE
//#############################################################

void init_lcd(){
	lcd_flow_control = LCD_BUSY; //Wait for LCD to bootup -- queue all commands prior to start
	lcd_flow_type = LCD_COMMAND;
	lcd_last_touch_command = LCD_TOUCH_NONE;
}

//#############################################################
//## SCREENS: OUTPUT STATUS
//#############################################################

void _lcd_no_bubble(){
	lcd_command("COLOR BLUE");
	lcd_command("BOX 30 30 F");
}

void _lcd_bubble(uint8_t mode){
	switch(mode){
		case LCD_ENABLED:
		lcd_command("COLOR 69");
		break;
		case LCD_STANDBY:
		lcd_command("COLOR 16");
		break;
	}
	lcd_command("CIRCLE 10 F");
	lcd_command("COLOR WHITE");
	lcd_command("CIRCLE 10");
	lcd_command("ARC 8 125 145");
}

void lcd_a(uint8_t mode){
	if (mode == LCD_DISABLED){
		lcd_command("XY 365 55");
		_lcd_no_bubble();		
	}
	else {
		lcd_command("XY 380 65");
		_lcd_bubble(mode);
	}		
}

void lcd_b(uint8_t mode){
	if (mode == LCD_DISABLED){
		lcd_command("XY 365 103");
		_lcd_no_bubble();
	}
	else {
		lcd_command("XY 380 113");
		_lcd_bubble(mode);
	}
}

void lcd_c(uint8_t mode){
	if (mode == LCD_DISABLED){
		lcd_command("XY 365 151");
		_lcd_no_bubble();
	}
	else {
		lcd_command("XY 380 161");
		_lcd_bubble(mode);
	}
}

void lcd_d(uint8_t mode){
	if (mode == LCD_DISABLED){
		lcd_command("XY 365 199");
		_lcd_no_bubble();
	}
	else {
		lcd_command("XY 380 209");
		_lcd_bubble(mode);
	}
}
 
//#############################################################
//## API
//#############################################################

//Only supports RUN (capital) and 31 (PLAY command as number) for detecting macros
void lcd_command(char* theCommand){
	uart_enqueue(&ulcd, LCD_COMMAND);
	uart_enqueue_string(&ulcd, theCommand);
	uart_enqueue(&ulcd, 0x0D); //command terminator
}

void lcd_macro(char* theCommand){
	//Command Header
	uart_enqueue(&ulcd, LCD_MACRO);
	//Command String
	uart_enqueue_string(&ulcd, theCommand);
	//Command Footer (terminator)
	uart_enqueue(&ulcd, 0x0D);
}	



//#############################################################
//## TOUCHSCREEN COMMAND BUFFER
//#############################################################

boolean lcd_end_macro(){
	if ((lcd_touch_buffer[0]==0x7E)&&(lcd_touch_buffer[1]==0x27)){return true;}
	else {return false;}
}

/// Returns the code for the last thing touched by user
/** Clears the touch history on read*/
uint8_t lcd_get_touch(){
	uint8_t last = lcd_last_touch_command;
	lcd_last_touch_command = LCD_TOUCH_NONE;
	return last;
}
	
void lcd_set_touch(uint8_t latest){
	//Rotate buffer
	for (uint8_t i=0;i<LCD_TOUCH_BUFFER_LEN-1;i++){lcd_touch_buffer[i+1] = lcd_touch_buffer[i];}
	//Add to front (0-index)
	lcd_touch_buffer[0] = latest;
	//Analyze (remember reverse order)
	if ((lcd_touch_buffer[0]==' ')&&(lcd_touch_buffer[1]==' ')&&(lcd_touch_buffer[2]==' ')&&(lcd_touch_buffer[3]==' ')){lcd_last_touch_command=LCD_TOUCH_NONE;return;}
}

//#############################################################
//## SERVICE ROUTINE
//#############################################################

void service_lcd(){	
	static uint16_t decimator = 0;

	//Flow control indicator
	if (lcd_flow_control != LCD_READY){led_on(LED_1);}
	else {led_off(LED_1);}
	
	switch(lcd_flow_control){
		case LCD_DONE_COMMAND:
			if (lcd_flow_type == LCD_COMMAND){lcd_flow_control = LCD_READY;}
			break;
		case LCD_DONE_MACRO:
			lcd_flow_control = LCD_READY;
			break;
	}
	
	//Boot up logic (show start screen and let LCD bootup so commands are understood)
	if (decimator < 5000) {decimator++;}
	else if (decimator == 5000) {lcd_flow_control = LCD_READY; decimator = 9000;}
}
