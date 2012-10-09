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
	lcd_flow_reboot = LCD_ENABLED;
}

//Reboot the LCD because it f's up a lot due to its lack of correctly implemented flow-control.
void lcd_reboot(){
	//flush command buffer
	uart_rxbuffer_disable(&ulcd);
	uart_txbuffer_disable(&ulcd);
	init_uart_obuffer(&ulcd);
	init_uart_ibuffer(&ulcd);
	uart_rxbuffer_enable(&ulcd);
	uart_txbuffer_enable(&ulcd);
	lcd_flow_control = LCD_READY; //unlock if stuck
	//state recovery
	lcd_flow_reboot = LCD_REBOOT; //suppress normal output from OS (dropped silently)
	//transmit flush to LCD
	uart_enqueue(&ulcd, LCD_COMMAND);
	uart_enqueue_string(&ulcd, "\r"); //transmit \r to terminate anything currently in the buffer
	//send reboot commands
	uart_enqueue(&ulcd, LCD_COMMAND);
	uart_enqueue_string(&ulcd, "RESET\r");
	//state recovery
	lcd_flow_reboot = LCD_REBOOT;
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
	if ((lcd_flow_reboot == LCD_ENABLED)&&(uart_count(&ulcd)<MAX_BUFFER_LEN-100)){
		uart_enqueue(&ulcd, LCD_COMMAND);
		uart_enqueue_string(&ulcd, theCommand);
		uart_enqueue(&ulcd, 0x0D); //command terminator
	}		
}

void lcd_macro(char* theCommand){
	if ((lcd_flow_reboot == LCD_ENABLED)&&(uart_count(&ulcd)<MAX_BUFFER_LEN-100)){
		//Command Header
		uart_enqueue(&ulcd, LCD_MACRO);
		//Command String
		uart_enqueue_string(&ulcd, theCommand);
		//Command Footer (terminator)
		uart_enqueue(&ulcd, 0x0D);
	}		
}	

//Don't forget to end theCommand with a SPACE!
//--it's that way to support negation
//--Positive example: "75 1 "
//--Negative example: "75 1 -" 
void lcd_update(char* theCommand, char* theValue){
	if ((lcd_flow_reboot == LCD_ENABLED)&&(uart_count(&ulcd)<MAX_BUFFER_LEN-100)){
		uart_enqueue(&ulcd, LCD_COMMAND);
		uart_enqueue_string(&ulcd, theCommand);
		uart_enqueue_string(&ulcd, theValue);
		uart_enqueue(&ulcd, 0x0D); //command terminator
	}	
}




//#############################################################
//## SERVICE ROUTINE
//#############################################################

void service_lcd(){	
	static uint16_t decimator = 0;

	//Flow control indicator
	if (lcd_flow_control != LCD_READY){led_on(LED_1);}
	else {led_off(LED_1);}
	
	//Reboot logic
	switch(lcd_flow_reboot){
		case LCD_ENABLED:
			//for efficiency test this case first (since it is normal case)
			break;
		case LCD_REBOOT:
			decimator = 0;
			lcd_flow_reboot = LCD_BOOTING;
			break;
		case LCD_BOOTING:
			decimator++;
			if (decimator >= 4999){
				decimator = 0;
				lcd_flow_reboot = LCD_ENABLED;
				STATE_menu = MENU_STARTUP;
			}
			break;
	}
	
	//Command processing logic
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
