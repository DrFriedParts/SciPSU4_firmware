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
#include "quadrature.h"

// Integration Controller -- This is where all the services are wired together

//#############################################################
//## INITIALIZATION ROUTINE
//#############################################################

void init_brain(){
	brain_power_reset();
	STATE_menu = MENU_OUTPUT;
}
	
//#############################################################
//## API
//#############################################################

//Used for testing stuff -- usually via a button so runs on demand
void brain_debug(){
	static uint8_t c = 0;
	//uart_enqueue_string(&uctrl, "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789----100---012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789---<200---012345678901234567890123456789---250>---ABCDEFGHIJ");

	//Channel values (from ADC)
	if (STATE_menu == MENU_CONSOLE){
		c++;
		if(c%2){lcd_console_write("Hello World...");}
		else{lcd_console_write("No! I refuse!");}
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

void brain_menu_change(uint8_t which_way){
	//Announce!
	audio_beep(1, 100);
	
	//Reset LCD state for menu change (no need to update screen with old stuff)
	init_uart_buffer(&ulcd); //flush outgoing buffer
	lcd_flow_control = LCD_READY; //clear any current transmissions
	lcd_last_touch_command = LCD_TOUCH_NONE; //clear out any latent touch actions
	lcd_command(""); //send \r to flush any existing partially transmitted commands
	
	//Change menu
	switch (STATE_menu){
		case MENU_OUTPUT:
			if (which_way == QUAD_DOWN){brain_menu_control();}
			if (which_way == QUAD_UP){brain_menu_console();}
			break;
		case MENU_CONTROL:
			if (which_way == QUAD_DOWN){brain_menu_console();}
			if (which_way == QUAD_UP){brain_menu_output();}
			break;
		case MENU_CONSOLE:
			if (which_way == QUAD_DOWN){brain_menu_output();}
			if (which_way == QUAD_UP){brain_menu_control();}
			break;
	}
	brain_menu_update(); //Update channel indicators
}

void brain_menu_output(){
	STATE_menu = MENU_OUTPUT;
	lcd_macro("RUN M_OUT");
}

void brain_menu_control(){
	STATE_menu = MENU_CONTROL;
	lcd_macro("RUN M_CTRL");
}

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

}
