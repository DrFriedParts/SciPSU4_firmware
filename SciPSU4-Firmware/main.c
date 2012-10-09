/*
 * SciPSU4_Firmware.c
 * 
 * Created: 9/23/2012 10:18:42 PM
 *  Author: Jonathan Friedman, PhD -- CircuitHub, Inc.
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "utilities.h"
#include "uart.h"
#include "ui.h"
#include "fp.h"
#include "adc.h"
#include "main.h"
#include "brain.h"
#include "uart_buffer.h"
#include "pwr_main.h"
#include "lcd.h"
#include "lcd_console.h"
#include "lcd_touch.h"
#include "quadrature.h"
#include "eeprom.h"

int init_rtos_clock_external(void){
	//Boot up and configure oscillator
	OSC.XOSCCTRL = B8(00100010); //enable external 32kHz Xtal using low-power (e.g. low-swing) mode
	OSC.CTRL = B8(00001011); //enable 32M-RC & External Xtal -- also "enable" 2M-RC since its already running b/c we booted from it and can't actually disable it until we switch sources
	//Wait for stability
	led_on(LED_0);
	//This is actually tricky sequencing because we boot from the 2MHz internal RC so previous write to OSC.CTRL was ineffective at shutting down the 2M-RC so OSC.STATUS will still reflect that it is running
	while(OSC.STATUS != B8(00001011)); //stall for external xtal and 32M-RC stability
	led_off(LED_0);
	//Configure
	OSC.DFLLCTRL = B8(00000010); //use external xtal for 32M-RC calibration
	DFLLRC32M.CTRL = B8(00000001); //enable Xtal calibration of internal 32MHz RC oscillator
	//Switch system clock over to stable RC oscillator
	//Switch to 32M-RC as system clock source and disable the 2M-RC that we booted from.
	//----REQUIRES CONFIGURATION PROTECTION REGISTER
	CCP = CCP_IOREG_gc; //disable change protection for IO register
	CLK.CTRL = CLK_SCLKSEL_RC32M_gc;
	OSC.CTRL = B8(00001010); //re-execute this write -- this will shutdown the 2M-RC since we are no longer running from it.
	//Now running live at 32MHz
	return 0;
}

int init_rtos_clock_internal(void){
	//Boot up and configure oscillator
	OSC.CTRL = B8(00000111); //enable 32M-RC & INTERNAL 32kHz -- also "enable" 2M-RC since its already running b/c we booted from it and can't actually disable it until we switch sources
	//Wait for stability
	led_on(LED_0);
	//This is actually tricky sequencing because we boot from the 2MHz internal RC so previous write to OSC.CTRL was ineffective at shutting down the 2M-RC so OSC.STATUS will still reflect that it is running
	while(OSC.STATUS != B8(00000111)); //stall for INTERNAL 32k-RC and 32MHz-RC stability
	led_off(LED_0);
	//Configure
	OSC.DFLLCTRL = B8(00000010); //use external xtal for 32M-RC calibration
	DFLLRC32M.CTRL = B8(00000001); //enable Xtal calibration of internal 32MHz RC oscillator
	//Switch system clock over to stable RC oscillator
	//Switch to 32M-RC as system clock source and disable the 2M-RC that we booted from.
	//----REQUIRES CONFIGURATION PROTECTION REGISTER
	CCP = CCP_IOREG_gc; //disable change protection for IO register
	CLK.CTRL = CLK_SCLKSEL_RC32M_gc;
	OSC.CTRL = B8(00001010); //re-execute this write -- this will shutdown the 2M-RC since we are no longer running from it.
	//Now running live at 32MHz
	return 0;
}



int main(void){
	//[BLINK ALIVE]
	uint8_t blah;
	int8_t updown;

	//[LED's, Button, & Switches]
	init_ui(); //init LED's first so that they are available for debugging
	init_fp(); //init Front Panel LED's and Switches so that they are available for debugging
	init_quadrature(); //init quadrature decoder for front panel rotary encoder
	
	//[CPU CLOCK]
	init_rtos_clock_internal();

	//[LCD]
	init_lcd();
	init_lcd_console();
	init_lcd_touch();

	//[UARTs]
	init_uart(&uctrl, BAUD_115200);
	init_uart(&udata, BAUD_115200);
	init_uart(&ulcd, BAUD_115200);
	init_uart_buffers();
	
	//[ADC]
	init_adc();		

	//[POWER PATH]
	init_pwr_main();
	init_pwr_adjust();

	//[BRAIN]
	init_brain();
	
	//[EEPROM] -- do this last
	init_eeprom();
		
	//[Realtime Loop Timer]
	//Use PortC's T/C0
	TCC0.CTRLA = 0x07; //Start the timer; Div1024 operation = 32M/1024 = 31250
	TCC0.PER = 62; //2ms Loop Time

	//[PMIC (Interrupt Controller)]
	PMIC.CTRL = B8(10000111); //enable all three interrupt levels (lowest one with round-robin)
	sei(); //ENABLE INTERRUPTS AND GO LIVE!

	//[RTOS START!]
	blah = 1;
	updown = 1;
	led_off(LED_0);
	led_off(LED_1);
	audio_beep(2, 100);
	PORTE.DIRSET = B8(00000011); //PE0, PE1 to output pin for loop timer

	while(1){
		PORTE.OUTSET = 0x02; //Set PE1 on start of loop and lower after work is done.
		//Blink alive
		led_dim(LED_0, blah);	
		blah += updown;
		if ((blah == 255) || (blah == 0)) {
			updown = -1 * updown;
		}
		PORTE.OUTTGL = 0x01; //wiggle pin to indicate loop timing
		
		//Call services
		service_audio();
		service_fp();
		service_adc();
		service_brain();	
		service_uart_buffer();
		service_lcd();
		service_lcd_console();
		service_lcd_touch();
		service_pwr_adjust();
		
		//Wait out RTOS loop
		PORTE.OUTCLR = 0x02; //Indicate work for this cycle has finished on PE1
		while((TCC0.INTFLAGS & _BV(0)) != 0x01); //Wait for the loop time to expire
		TCC0.INTFLAGS = 0x01; //Clear the interrupt flag
	}//while
}//int main(void)
