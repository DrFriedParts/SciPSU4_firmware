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
		
		boolean lcd_end_macro();
		
	//TOUCH COMMANDS
		#define LCD_TOUCH_BUFFER_LEN 6
		volatile uint8_t lcd_touch_buffer[LCD_TOUCH_BUFFER_LEN];
		volatile uint8_t lcd_last_touch_command;
		#define LCD_TOUCH_NONE 21
		#define LCD_TOUCH_ROW_A 22
		#define LCD_TOUCH_ROW_B 23
		#define LCD_TOUCH_ROW_C 24
		#define LCD_TOUCH_ROW_D 25
		
		
		uint8_t lcd_get_touch();
		void lcd_set_touch(uint8_t latest);
		
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
		void service_lcd();
#endif
