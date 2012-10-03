#ifndef __uartbuff_h
#define __uartbuff_h

	//DATA BUFFERS
	#define NUM_PORTS_TO_BUFFER 3
	#define MAX_BUFFER_LEN 1024
	#define MAX_IBUFFER_LEN 1024

	//FUNCTIONS
	void init_uart_buffers();
	void service_uart_buffer();
	
	void init_uart_buffer(USART_t* which);
	void uart_transmit(USART_t* port);
	void uart_receive(USART_t* port);
	
	void uart_transmit_lcd(USART_t* port);
	void uart_receive_lcd(USART_t* port);
	

	void init_uart_obuffer(USART_t* port);
	uint16_t uart_count(USART_t* port);
	void uart_enqueue(USART_t* port, uint8_t datain);
	void uart_enqueue_string(USART_t* port, char* string_in);
	uint8_t uart_dequeue(USART_t* port);

	void init_uart_ibuffer(USART_t* port);
	uint16_t uart_icount(USART_t* port);
	void uart_ienqueue(USART_t* port, uint8_t datain);
	uint8_t uart_idequeue(USART_t* port);

	void uart_txbuffer_enable(USART_t* port);
	void uart_txbuffer_disable(USART_t* port);	
	void uart_rxbuffer_enable(USART_t* port);
	void uart_rxbuffer_disable(USART_t* port);
#endif
