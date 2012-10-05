#ifndef __lcd_h
#define __lcd_h

	//DEFINITIONS
		#define LCD_BUSY 0x4E
		#define LCD_DONE_COMMAND 0x4F
		#define LCD_DONE_MACRO 0x50
		#define LCD_READY 0x52
	
		#define LCD_ENABLED 31
		#define LCD_STANDBY 32
		#define LCD_DISABLED 33
		
		#define LCD_COMMAND 0xFA
		#define LCD_MACRO 0xFE
		
		
		
	
		
	//UART BUFFER STATE		
		volatile uint8_t lcd_flow_control;
		volatile uint8_t lcd_flow_type;  

	//OUTPUT & CONTROL SCREENS
		void lcd_a(uint8_t mode);
		void lcd_b(uint8_t mode);
		void lcd_c(uint8_t mode);
		void lcd_d(uint8_t mode);
	
	//FUNCTIONS
		void init_lcd();
		void lcd_command(char* theCommand);
		void lcd_macro(char* theCommand);
		void lcd_update(char* theCommand, char* theValue);
		void service_lcd();
#endif
