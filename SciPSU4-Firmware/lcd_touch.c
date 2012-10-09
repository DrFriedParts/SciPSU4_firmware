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

//Handles EarthLCD ezLCD301 touchscreen behavior logic

//#############################################################
//## INITIALIZATION ROUTINE
//#############################################################

void init_lcd_touch(){
	lcd_last_touch_command = LCD_TOUCH_NONE;
	lcd_touch_buffer[LCD_TOUCH_BUFFER_LEN] = 0; //string termination to help with debug printing of the buffer	
}

//#############################################################
//## API
//#############################################################

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
	for (uint8_t i=0;i<LCD_TOUCH_BUFFER_LEN-1;i++){lcd_touch_buffer[LCD_TOUCH_BUFFER_LEN-1-i] = lcd_touch_buffer[LCD_TOUCH_BUFFER_LEN-2-i];}
	//Add to front (0-index)
	lcd_touch_buffer[0] = latest;

	//Analyze (remember reverse order)

	//Top Menu Navigation
		if ((lcd_touch_buffer[0]==0x0D)&&(lcd_touch_buffer[1]=='2')&&(lcd_touch_buffer[2]=='1')&&(lcd_touch_buffer[3]=='P')&&(lcd_touch_buffer[4]=='Z')&&(lcd_touch_buffer[5]=='T')){lcd_last_touch_command=LCD_TOUCH_OUTPUT;return;}
		if ((lcd_touch_buffer[0]==0x0D)&&(lcd_touch_buffer[1]=='3')&&(lcd_touch_buffer[2]=='1')&&(lcd_touch_buffer[3]=='P')&&(lcd_touch_buffer[4]=='Z')&&(lcd_touch_buffer[5]=='T')){lcd_last_touch_command=LCD_TOUCH_CONTROL;return;}
		if ((lcd_touch_buffer[0]==0x0D)&&(lcd_touch_buffer[1]=='4')&&(lcd_touch_buffer[2]=='1')&&(lcd_touch_buffer[3]=='P')&&(lcd_touch_buffer[4]=='Z')&&(lcd_touch_buffer[5]=='T')){lcd_last_touch_command=LCD_TOUCH_CONSOLE;return;}
	//Control Screen		
		if ((lcd_touch_buffer[0]==0x0D)&&(lcd_touch_buffer[1]=='7')&&(lcd_touch_buffer[2]=='1')&&(lcd_touch_buffer[3]=='P')&&(lcd_touch_buffer[4]=='Z')&&(lcd_touch_buffer[5]=='T')){lcd_last_touch_command=LCD_TOUCH_ROW_A;return;}
		if ((lcd_touch_buffer[0]==0x0D)&&(lcd_touch_buffer[1]=='8')&&(lcd_touch_buffer[2]=='1')&&(lcd_touch_buffer[3]=='P')&&(lcd_touch_buffer[4]=='Z')&&(lcd_touch_buffer[5]=='T')){lcd_last_touch_command=LCD_TOUCH_ROW_B;return;}
		if ((lcd_touch_buffer[0]==0x0D)&&(lcd_touch_buffer[1]=='9')&&(lcd_touch_buffer[2]=='1')&&(lcd_touch_buffer[3]=='P')&&(lcd_touch_buffer[4]=='Z')&&(lcd_touch_buffer[5]=='T')){lcd_last_touch_command=LCD_TOUCH_ROW_C;return;}
		if ((lcd_touch_buffer[0]==0x0D)&&(lcd_touch_buffer[1]=='0')&&(lcd_touch_buffer[2]=='2')&&(lcd_touch_buffer[3]=='P')&&(lcd_touch_buffer[4]=='Z')&&(lcd_touch_buffer[5]=='T')){lcd_last_touch_command=LCD_TOUCH_ROW_D;return;}
	//Control Dialog Window
		if ((lcd_touch_buffer[0]==0x0D)&&(lcd_touch_buffer[1]=='6')&&(lcd_touch_buffer[2]=='9')&&(lcd_touch_buffer[3]=='P')&&(lcd_touch_buffer[4]=='Z')&&(lcd_touch_buffer[5]=='T')){lcd_last_touch_command=LCD_TOUCH_CLOSE_DIAL;return;}
		if ((lcd_touch_buffer[0]==0x0D)&&(lcd_touch_buffer[1]=='7')&&(lcd_touch_buffer[2]=='9')&&(lcd_touch_buffer[3]=='P')&&(lcd_touch_buffer[4]=='Z')&&(lcd_touch_buffer[5]=='T')){lcd_last_touch_command=LCD_TOUCH_LEFT;return;}
		if ((lcd_touch_buffer[0]==0x0D)&&(lcd_touch_buffer[1]=='8')&&(lcd_touch_buffer[2]=='9')&&(lcd_touch_buffer[3]=='P')&&(lcd_touch_buffer[4]=='Z')&&(lcd_touch_buffer[5]=='T')){lcd_last_touch_command=LCD_TOUCH_RIGHT;return;}
	//Output Screen
		if ((lcd_touch_buffer[0]==0x0D)&&(lcd_touch_buffer[1]=='9')&&(lcd_touch_buffer[2]=='9')&&(lcd_touch_buffer[3]=='P')&&(lcd_touch_buffer[4]=='Z')&&(lcd_touch_buffer[5]=='T')){lcd_last_touch_command=LCD_TOUCH_CLOSE_DETAIL;return;}
		if ((lcd_touch_buffer[0]==0x0D)&&(lcd_touch_buffer[1]=='5')&&(lcd_touch_buffer[2]=='1')&&(lcd_touch_buffer[3]=='P')&&(lcd_touch_buffer[4]=='Z')&&(lcd_touch_buffer[5]=='T')){lcd_last_touch_command=LCD_TOUCH_OPEN_DETAIL_AB;return;}
		if ((lcd_touch_buffer[0]==0x0D)&&(lcd_touch_buffer[1]=='6')&&(lcd_touch_buffer[2]=='1')&&(lcd_touch_buffer[3]=='P')&&(lcd_touch_buffer[4]=='Z')&&(lcd_touch_buffer[5]=='T')){lcd_last_touch_command=LCD_TOUCH_OPEN_DETAIL_CD;return;}
}

//#############################################################
//## SERVICE ROUTINE
//#############################################################

//Used to refresh the console
void service_lcd_touch(){
	if (lcd_last_touch_command==LCD_TOUCH_NONE) return; //exit if nothing has been pressed
	switch(lcd_get_touch()){
		
		//MENU NAVIGATION
		case LCD_TOUCH_OUTPUT:
			brain_menu_load(MENU_OUTPUT);
			break;
		case LCD_TOUCH_CONTROL:
			brain_menu_load(MENU_CONTROL);
			break;
		case LCD_TOUCH_CONSOLE:
			brain_menu_load(MENU_CONSOLE);
			break;
			
		//CONTROL MENU
		case LCD_TOUCH_ROW_A:
			brain_menu_control_dial(LCD_TOUCH_ROW_A);
			break;
		case LCD_TOUCH_ROW_B:
			brain_menu_control_dial(LCD_TOUCH_ROW_B);
			break;
		case LCD_TOUCH_ROW_C:
			brain_menu_control_dial(LCD_TOUCH_ROW_C);
			break;
		case LCD_TOUCH_ROW_D:
			brain_menu_control_dial(LCD_TOUCH_ROW_D);
			break;
		
		//CONTROL DIALOG WINDOW
		case LCD_TOUCH_LEFT:
			brain_menu_control_dial_select(LCD_TOUCH_LEFT);
			break;
		case LCD_TOUCH_RIGHT:
			brain_menu_control_dial_select(LCD_TOUCH_RIGHT);
			break;
		case LCD_TOUCH_CLOSE_DIAL:
			brain_menu_load(MENU_CONTROL);
			break;
		
		//OUTPUT MENU
		case LCD_TOUCH_CLOSE_DETAIL:
			brain_menu_load(MENU_OUTPUT);
			break;
		case LCD_TOUCH_OPEN_DETAIL_AB:
			brain_menu_output_detail(LCD_TOUCH_OPEN_DETAIL_AB);
			break;
		case LCD_TOUCH_OPEN_DETAIL_CD:
			brain_menu_output_detail(LCD_TOUCH_OPEN_DETAIL_CD);
			break;
	}	
}