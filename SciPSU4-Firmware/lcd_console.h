#ifndef __lcdc_h
#define __lcdc_h

	//DEFINITIONS
		#define LCD_CONSOLE_NUM_ROWS 8
		#define LCD_CONSOLE_NUM_COLS 100
		
		uint8_t lcd_console_head;
		char lcd_console[LCD_CONSOLE_NUM_ROWS][LCD_CONSOLE_NUM_COLS+7];
		boolean lcd_buffer_dirty[LCD_CONSOLE_NUM_ROWS];
	
	//FUNCTIONS
		void init_lcd_console();
		void lcd_console_write(char* theString);
		void service_lcd_console();
#endif
