#include <avr/io.h>
#include <avr/interrupt.h>
#include "utilities.h"
#include "uart.h"
#include "uart_buffer.h"
#include "ui.h" //for debugging
#include "string.h"
#include "lcd.h" //TODO: remove this dependency
#include "lcd_touch.h" //TODO: remove this dependency

//XMEGA uart DATA BUFFER
//--Due to the XMEGA architecture, it is not as easy to write a port-agnostic
//		data buffer, so I implemented in this separate file for now.

/*
	Limitations -- 
		- only works for one hardcoded UART
		- for performance reasons string format is tailored to ezLCD301 application (CR = ACK on RX, and CR = EXECUTE on TX)	
*/
//TODO -- make use of DMA transfers

//==================================
//= State and Storage Variables
//==================================

//TX Queue (outgoing)
volatile uint8_t uart_buffer[NUM_PORTS_TO_BUFFER][MAX_BUFFER_LEN];
volatile uint16_t uart_head[NUM_PORTS_TO_BUFFER];
volatile uint16_t uart_tail[NUM_PORTS_TO_BUFFER];

//RX Queue (incoming)
volatile uint8_t uart_ibuffer[NUM_PORTS_TO_BUFFER][MAX_IBUFFER_LEN];
volatile uint16_t uart_ihead[NUM_PORTS_TO_BUFFER];
volatile uint16_t uart_itail[NUM_PORTS_TO_BUFFER];

inline uint8_t port_map(USART_t* port){
	if (port == &ulcd)  {return 0;}
	if (port == &uctrl) {return 1;}
	if (port == &udata) {return 2;}
	return 0; //should never reach here!
}

//************************************************************************
//************************************************************************
//** [PORT SPECIFIC CODE]
//************************************************************************
//************************************************************************

//Must correspond to definitions in uart.h
SIGNAL(USARTC0_DRE_vect) {uart_transmit(&uctrl);}//TX Interrupt
SIGNAL(USARTC0_RXC_vect) {uart_receive(&uctrl);} //RX Interrupt
SIGNAL(USARTC1_DRE_vect) {uart_transmit(&udata);}//TX Interrupt
SIGNAL(USARTC1_RXC_vect) {uart_receive(&udata);} //RX Interrupt
SIGNAL(USARTF1_DRE_vect) {uart_transmit_lcd(&ulcd);}//TX Interrupt
SIGNAL(USARTF1_RXC_vect) {uart_receive_lcd(&ulcd);} //RX Interrupt
	
//MAKE SURE TO INIT UART FIRST
void init_uart_buffers(){
	init_uart_buffer(&uctrl);
	init_uart_buffer(&udata);
	init_uart_buffer(&ulcd);
}	
	
//************************************************************************
//************************************************************************
//** [PORT AGNOSTIC CODE]
//************************************************************************
//************************************************************************

//MAKE SURE TO INIT UART FIRST
void init_uart_buffer(USART_t* port){
	//Setup data buffers
		init_uart_obuffer(port);
		init_uart_ibuffer(port);
	//Enable Receive and Transmit interrupts
		uart_rxbuffer_enable(port);
		uart_txbuffer_enable(port);
}

//Enable Receive Complete (high priority) -- so incoming is always handled first
void uart_rxbuffer_enable(USART_t* port){
	port->CTRLA = (port->CTRLA | B8(00110000));	//Set the Data Register Empty Interrupt to Medium Priority (timer needs to be higher!)
}

void uart_rxbuffer_disable(USART_t* port){
	port->CTRLA = (port->CTRLA & B8(11001111));	//Disable the Data Register Empty Interrupt
}

//Enable Transmit Ready (med priority) 
void uart_txbuffer_enable(USART_t* port){
	port->CTRLA = (port->CTRLA | B8(00000010));	//Set the Data Register Empty Interrupt to Medium Priority (timer needs to be higher!)
}

void uart_txbuffer_disable(USART_t* port){
	port->CTRLA = (port->CTRLA & B8(11111100));	//Disable the Data Register Empty Interrupt
}

//==================================
//= TRANSMISSION ENGINE (ISR BASED)
//==================================

//Starts a transmission out of the UART if the UART is ready to receive data
//and we have data to send. (helper function to the ISR so that we can initiate
//the first transfer
void inline uart_transmit(USART_t* port){
	//keep loading until data register is full or outgoing queue is empty
	while (((port->STATUS & _BV(5)) == B8(00100000)) && (uart_count(port) > 0)){
		port->DATA = uart_dequeue(port);
	}
	//disable the tx outgoing hardware buffer ready interrupt if we have nothing more to put in it.
	if(uart_count(port)>0) uart_txbuffer_enable(port);
	else uart_txbuffer_disable(port);
}

void inline uart_transmit_lcd(USART_t* port){
	uint8_t toSend;
	//keep loading until data register is full or outgoing queue is empty
	while (((port->STATUS & _BV(5)) == B8(00100000)) && (uart_count(port) > 0) && (lcd_flow_control == LCD_READY)){
		toSend = uart_dequeue(port);
		switch(toSend){
			case LCD_COMMAND:
			case LCD_MACRO:
				//Header byte just describes payload -- do not send to LCD
				lcd_flow_type = toSend;
				break;
			default:
				//Payload bytes -- send to LCD
				if (toSend == 0x0D) {lcd_flow_control = LCD_BUSY;}
				port->DATA = toSend;
				break;
		}				
	}
	uart_txbuffer_disable(port); //implemented this way to prevent periodic stalls that happen when uart_txbuffer isn't disabled quickly enough
	if ((uart_count(port)>0) && (lcd_flow_control == LCD_READY)){ //...something is waiting to go out
		uart_txbuffer_enable(port);	
	}	
}


//==================================
//= RECEPTION ENGINE (ISR BASED)
//==================================

void inline uart_receive(USART_t* port){
	//keep receiving until data register is empty or incoming queue is full
	while (((port->STATUS & _BV(7)) == B8(10000000)) && (uart_icount(port) < MAX_IBUFFER_LEN)){		
		uart_ienqueue(port, port->DATA);
	}	
}

void inline uart_receive_lcd(USART_t* port){
	uint8_t incomingByte;
	//keep receiving until data register is empty or incoming queue is full
	while (((port->STATUS & _BV(7)) == B8(10000000)) && (uart_icount(port) < MAX_IBUFFER_LEN)){
		incomingByte = port->DATA;
		uart_enqueue(&udata, incomingByte); //echo to data port
		if (incomingByte == 0x0D){
			switch(lcd_flow_type){				
				case LCD_MACRO:
					uart_enqueue(&udata,'+');uart_enqueue(&udata,lcd_touch_buffer[0]);uart_enqueue(&udata,lcd_touch_buffer[1]);
					if (lcd_end_macro()){ //look for '~ macro terminator sequence
						lcd_flow_control = LCD_DONE_MACRO;
					}
					break;
				default:
				case LCD_COMMAND:
					lcd_flow_control = LCD_DONE_COMMAND;
					break;
			}					
		}
		lcd_set_touch(incomingByte); //write to touch-command listener
		uart_ienqueue(port, incomingByte);
	}
}

//***************************************************
/** @defgroup uart_oq Serial Outgoing Data Queue */
/** @{ */
/** Insert from head. Read from tail. The goal is to be fast (very fast) and light.
	No protection is provided for buffer overflow! Be careful! */
//***************************************************

void init_uart_obuffer(USART_t* port){
	uint8_t idx = port_map(port);
	uart_head[idx] = 0;
	uart_tail[idx] = 0;
}

inline uint16_t uart_count(USART_t* port){
	uint8_t idx = port_map(port);
	if (uart_head[idx] >= uart_tail[idx]){	
		return (uart_head[idx] - uart_tail[idx]);
	}
	else {
		return ((MAX_BUFFER_LEN-uart_tail[idx])+uart_head[idx]);
	}
}

///Enqueue a string into the outgoing serial queue. Adds CR terminator to string.
inline void uart_enqueue_string(USART_t* port, char* string_in){
	uint16_t length = (uint16_t)strlen(string_in);
	for (uint16_t i=0; i<length; i++) {uart_enqueue(port, (uint8_t)string_in[i]);}
}

///Enqueue date into the outgoing serial queue. This data is sent via USB to the PC's first virtual Comm Port associated with the EEICM. 
/**This is the queue used to send data back to the command and control GUI. The #define UART_DEBUG can be used to disable normal serial activity through this queue
	The blue LED is used in this routine to signal buffer overflow, which, due to the real-time scheduled nature of the EEICM firmware architecture, should not happen.
	This function is inactive when in UART DEBUG mode. Calls to this function have no effect during this period.*/
inline void uart_enqueue(USART_t* port, uint8_t datain){
#ifndef UART_DEBUG
	uint8_t idx = port_map(port);
	
	//if (idx==0){uart_enqueue(&uctrl, datain);} //xxx
	
	uart_buffer[idx][uart_head[idx]] = datain;
	uart_head[idx]++;
	if (uart_head[idx] >= MAX_BUFFER_LEN){
		uart_head[idx] = 0;
	}
	if (idx == 0){uart_transmit_lcd(port);} //start the transmission process.
	else {uart_transmit(port);} //start the transmission process.}	
	
#endif
}

inline uint8_t uart_dequeue(USART_t* port){
	uint8_t idx = port_map(port);
	uint16_t oldtail;
	oldtail = uart_tail[idx];
	uart_tail[idx]++;
	if (uart_tail[idx] >= MAX_BUFFER_LEN){
		uart_tail[idx] = 0;
	}
	
	//if (idx==0){uart_enqueue(&udata, uart_buffer[idx][oldtail]);} //xxx
	
	return uart_buffer[idx][oldtail];
}

/** @} */
//****************************************************

//***************************************************
/** @defgroup uart_iq Serial Incoming Data Queue */
/** @{ */
/** Insert from head. Read from tail. The goal is to be fast (very fast) and light.
	No protection is provided for buffer overflow! Be careful! */
//***************************************************

void init_uart_ibuffer(USART_t* port){
	uint8_t idx = port_map(port);
	uart_ihead[idx] = 0;
	uart_itail[idx] = 0;
}

inline uint16_t uart_icount(USART_t* port){
	uint8_t idx = port_map(port);
	if (uart_ihead[idx] >= uart_itail[idx]){	
		return (uart_ihead[idx] - uart_itail[idx]);
	}
	else {
		return ((MAX_IBUFFER_LEN-uart_itail[idx])+uart_ihead[idx]);
	}
}

inline void uart_ienqueue(USART_t* port, uint8_t datain){
	uint8_t idx = port_map(port);
	uart_ibuffer[idx][uart_ihead[idx]] = datain;
	uart_ihead[idx]++;
	if (uart_ihead[idx] >= MAX_IBUFFER_LEN){
		uart_ihead[idx] = 0;
	}
}

//internal implementation of peek and dequeue
inline uint8_t _uart_idequeue(USART_t* port, uint8_t peek){
	uint8_t idx = port_map(port);
	uint16_t oldtail;
	oldtail = uart_itail[idx];
	uart_itail[idx]++;
	if (uart_itail[idx] >= MAX_IBUFFER_LEN){
		uart_itail[idx] = 0;
	}
	if (peek){uart_itail[idx] = oldtail;}
	return uart_ibuffer[idx][oldtail];
}

inline uint8_t uart_idequeue(USART_t* port){
	return _uart_idequeue(port, false);
}

inline void service_uart_buffer(){
	//Resume transmission attempt if outgoing data still pending -- we do this so we can don't block the CPU for too long waiting for flow-control
	if (uart_count(&ulcd)>0){uart_transmit_lcd(&ulcd);}
}



/** @} */
//****************************************************



