#ifndef __lcdt_h
#define __lcdt_h

	//TOUCH COMMANDS
	#define LCD_TOUCH_BUFFER_LEN 6
	volatile uint8_t lcd_touch_buffer[LCD_TOUCH_BUFFER_LEN+1];
	

	volatile uint8_t lcd_last_touch_command;
	#define LCD_TOUCH_NONE 21
	
	//Control Screen
	#define LCD_TOUCH_ROW_A 17
	#define LCD_TOUCH_ROW_B 18
	#define LCD_TOUCH_ROW_C 19
	#define LCD_TOUCH_ROW_D 20
	
	//Control Dialog Window
	#define LCD_TOUCH_CLOSE_DIAL 96
	#define LCD_TOUCH_LEFT 97
	#define LCD_TOUCH_RIGHT 98
	
	//Output Detail Dialog Window
	#define LCD_TOUCH_CLOSE_DETAIL 99
	#define LCD_TOUCH_OPEN_DETAIL_AB 15
	#define LCD_TOUCH_OPEN_DETAIL_CD 16
	
	//Top Menu
	#define LCD_TOUCH_OUTPUT 12
	#define LCD_TOUCH_CONTROL 13
	#define LCD_TOUCH_CONSOLE 14
	
	//FUNCTIONS
		void init_lcd_touch();
		uint8_t lcd_get_touch();
		void lcd_set_touch(uint8_t latest);
		boolean lcd_end_macro();
		void service_lcd_touch();
#endif
