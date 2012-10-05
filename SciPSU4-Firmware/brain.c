#include <avr/io.h>
#include <avr/interrupt.h>
#include "utilities.h"
#include "brain.h"
#include "fp.h"
#include "ui.h"
#include "pwr_main.h"
#include "uart.h"
#include "uart_buffer.h"
#include "string.h"
#include "stdlib.h" //itoa -- integer to ascii conversion
#include "lcd.h"
#include "lcd_console.h"
#include "lcd_touch.h"
#include "quadrature.h"
#include "adc.h"

// Integration Controller -- This is where all the services are wired together

//#############################################################
//## INITIALIZATION ROUTINE
//#############################################################

void init_brain(){
	brain_power_reset();
	STATE_menu = MENU_STARTUP;
}
	
//#############################################################
//## API
//#############################################################

//Used for testing stuff -- usually via a button so runs on demand
void brain_debug(){
	static uint8_t c = 0;
	char volts[12];
	//uart_enqueue_string(&uctrl, "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789----100---012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789---<200---012345678901234567890123456789---250>---ABCDEFGHIJ");

	//Channel values (from ADC)
	if (STATE_menu == MENU_CONSOLE){
		c++;
		if(c%2){lcd_console_write("Hello World...");}
		else{lcd_console_write("No! I refuse!");}
		adc_data(8, VOLTAGE, volts);
		lcd_console_write(volts);
	}
	else {		
		lcd_command("75 1 1234");
		lcd_command("75 11 5678");
		lcd_command("75 2 8765");
		lcd_command("75 21 4321");
	}		
	
}


//Safe and reset all channels (all disabled, master off)
void brain_power_reset(){
	STATE_power_channels = 0;
	STATE_power_output = DISABLE;
	pwr_main_off(CHANNEL_ALL); //Actually shutoff power
	fp_led_disable(CHANNEL_M); //Update LED display to indicate this
}

//Toggle power state and effect the new behavior
void brain_power(uint8_t which){
	//Toggle channel state
		STATE_power_channels ^= _BV(which); 
	//Channel disabled
		if ((STATE_power_channels & _BV(which))==0){		
			pwr_main_off(which);
			fp_led_disable(which);
		}	
	//Channel enabled
		else {
			fp_led_enable(which);
			if (STATE_power_output == ENABLE) {pwr_main_on(which);}
			if (STATE_power_output == DISABLE) {}		
		}	
	//Update LCD
		brain_menu_update();
}

void brain_power_master(){
	//Go to STANDBY (DISABLE)
	if (STATE_power_output == ENABLE){
			STATE_power_output = DISABLE;
			pwr_main_off(CHANNEL_ALL);
			fp_led_disable(CHANNEL_M);			
	}
	
	//Enable OUTPUT! (ENABLE)
	else {	
		if (STATE_power_output == DISABLE){
			STATE_power_output = ENABLE;
			pwr_main_on(CHANNEL_RESTORE);
			fp_led_enable(CHANNEL_M);
		}			
	}
	
	//Update LCD
	brain_menu_update();
}	

void brain_button_pressed(){
	if (STATE_menu == MENU_STARTUP){brain_menu_change(0);}
}

void brain_menu_load(uint8_t which_menu){
	//Announce!
	audio_beep(1, 100);
	
	//Reset LCD state for menu change (no need to update screen with old stuff)
	init_uart_buffer(&ulcd); //flush outgoing buffer
	lcd_flow_control = LCD_READY; //clear any current transmissions
	lcd_last_touch_command = LCD_TOUCH_NONE; //clear out any latent touch actions
	lcd_command(""); //send \r to flush any existing partially transmitted commands
	
	switch (which_menu){
		case MENU_STARTUP:
			brain_menu_output();
			break;
		case MENU_OUTPUT:
			brain_menu_output();
			break;
		case MENU_CONTROL:
			brain_menu_control();
			break;
		case MENU_CONSOLE:
			brain_menu_console();
			break;
	}
	brain_menu_update(); //Update channel indicators
}

void brain_menu_change(uint8_t which_way){
	//Change menu
	switch (STATE_menu){
		case MENU_STARTUP:
			brain_menu_load(MENU_OUTPUT);
			break;
		case MENU_OUTPUT:
			if (which_way == QUAD_DOWN){brain_menu_load(MENU_CONTROL);}
			if (which_way == QUAD_UP){brain_menu_load(MENU_CONSOLE);}
			break;
		case MENU_CONTROL:
			if (which_way == QUAD_DOWN){brain_menu_load(MENU_CONSOLE);}
			if (which_way == QUAD_UP){brain_menu_load(MENU_OUTPUT);}
			break;
		case MENU_CONSOLE:
			if (which_way == QUAD_DOWN){brain_menu_load(MENU_OUTPUT);}
			if (which_way == QUAD_UP){brain_menu_load(MENU_CONTROL);}
			break;
	}	
}

//=================
//== OUTPUT Menu
void brain_menu_output(){
	STATE_menu = MENU_OUTPUT;
	lcd_macro("RUN M_OUT");
}

void brain_menu_output_detail(uint8_t which_detail){
	switch(which_detail){
		case LCD_TOUCH_OPEN_DETAIL_AB:
			STATE_menu = MENU_DETAIL_AB;
			lcd_macro("RUN M_DETAIL");
			lcd_command("88 58 A");
			lcd_command("88 59 B");
			break;
		case LCD_TOUCH_OPEN_DETAIL_CD:
			STATE_menu = MENU_DETAIL_CD;
			lcd_macro("RUN M_DETAIL");
			lcd_command("88 58 C");
			lcd_command("88 59 D");
			break;
	}
}


//=================
//== CONTROL Menu

void brain_menu_control(){
	STATE_menu = MENU_CONTROL;
	lcd_macro("RUN M_CTRL");
}

void brain_menu_control_dial(uint8_t which_channel){
	lcd_macro("RUN M_DIAL");
	switch(which_channel){
		case LCD_TOUCH_ROW_A:
			STATE_menu = MENU_DIAL_A;
			lcd_command("88 90 A");
			break;
		case LCD_TOUCH_ROW_B:
			STATE_menu = MENU_DIAL_B;
			lcd_command("88 90 B");
			break;
		case LCD_TOUCH_ROW_C:
			STATE_menu = MENU_DIAL_C;
			lcd_command("88 90 C");
			break;
		case LCD_TOUCH_ROW_D:
			STATE_menu = MENU_DIAL_D;
			lcd_command("88 90 D");
			break;
	}
}

//=================
//== CONSOLE Menu

void brain_menu_console(){
	STATE_menu = MENU_CONSOLE;
	lcd_macro("RUN M_CON");
}

//Update channel enabled indicators
void brain_menu_update(){
	if ((STATE_menu == MENU_OUTPUT) || (STATE_menu == MENU_CONTROL)) {
		if ((STATE_power_channels & _BV(0)) == 0){lcd_a(LCD_DISABLED);}
		else {
			if (STATE_power_output == DISABLE){lcd_a(LCD_STANDBY);}
			else if (STATE_power_output == ENABLE){lcd_a(LCD_ENABLED);}
		}	
				
		if ((STATE_power_channels & _BV(1)) == 0){lcd_b(LCD_DISABLED);}
		else {
			if (STATE_power_output == DISABLE){lcd_b(LCD_STANDBY);}
			else if (STATE_power_output == ENABLE){lcd_b(LCD_ENABLED);}
		}	

		if ((STATE_power_channels & _BV(2)) == 0){lcd_c(LCD_DISABLED);}
		else {
			if (STATE_power_output == DISABLE){lcd_c(LCD_STANDBY);}
			else if (STATE_power_output == ENABLE){lcd_c(LCD_ENABLED);}
		}

		if ((STATE_power_channels & _BV(3)) == 0){lcd_d(LCD_DISABLED);}
		else {
			if (STATE_power_output == DISABLE){lcd_d(LCD_STANDBY);}
			else if (STATE_power_output == ENABLE){lcd_d(LCD_ENABLED);}
		}
	}	
}

//#############################################################
//## SERVICE ROUTINE
//#############################################################

void service_brain(){
	static uint16_t menu_update_counter = 0;
	char meter_value[12];
	//Decide when to update Power output measurements
	menu_update_counter++;
	if (menu_update_counter >= 500){
		menu_update_counter = 0;
		switch(STATE_menu){
			case MENU_OUTPUT:
				adc_data(0, VOLTAGE, meter_value); //A V+
				lcd_update("75 1", meter_value);
				adc_data(1, CURRENT_HI_RES, meter_value); //A I+
				lcd_update("75 11", meter_value);
				adc_data(4, VOLTAGE, meter_value); //B V+
				lcd_update("75 2", meter_value);
				adc_data(5, CURRENT_HI_RES, meter_value); //B I+
				lcd_update("75 21", meter_value);
				adc_data(8, VOLTAGE, meter_value); //C V+
				lcd_update("75 3", meter_value);
				adc_data(9, CURRENT_HI_RES, meter_value); //C I+
				lcd_update("75 31", meter_value);
				adc_data(12, VOLTAGE, meter_value); //D V+
				lcd_update("75 4", meter_value);
				adc_data(13, CURRENT_HI_RES, meter_value); //D I+
				lcd_update("75 41", meter_value);
				break;
			case MENU_CONTROL:
				break;
		}
		
	}
	
}
