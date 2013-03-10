#ifndef __lcdc_h
#define __lcdc_h

	//DEFINITIONS
		#define LCD_CONSOLE_NUM_ROWS 8
		#define LCD_CONSOLE_NUM_COLS 100
		
		uint8_t lcd_console_head;
		char lcd_console[LCD_CONSOLE_NUM_ROWS][LCD_CONSOLE_NUM_COLS+7];
		boolean lcd_buffer_dirty[LCD_CONSOLE_NUM_ROWS];
		
		uint8_t STATE_lcd_console;
		uint8_t lcd_console_channel;
		uint8_t lcd_console_command;
		uint8_t lcd_console_value_1;
		
		char lcd_console_meter_value[12];
		char lcd_console_set_value[5];
		char lcd_console_incoming_byte[4];
		
		#define LCD_CONSOLE_STATE_IDLE 22
		#define LCD_CONSOLE_STATE_COMMAND 23
		#define LCD_CONSOLE_STATE_VALUE_1 24
		#define LCD_CONSOLE_STATE_VALUE_N 25
		#define LCD_CONSOLE_STATE_VALUE_N2 26
		#define LCD_CONSOLE_STATE_VALUE_N3 27
		#define LCD_CONSOLE_STATE_VALUE_N4 28
		#define LCD_CONSOLE_STATE_TERMINATOR_1 29
		#define LCD_CONSOLE_STATE_TERMINATOR_N 30
		
		#define LCD_CONSOLE_CMD_RELAY 83
		#define LCD_CONSOLE_CMD_MAIN_POS 84
		#define LCD_CONSOLE_CMD_MAIN_NEG 85
		#define LCD_CONSOLE_CMD_ADJUST_MAX 86
		#define LCD_CONSOLE_CMD_ADJUST_SET 87
							
	//FUNCTIONS
		void init_lcd_console();
		void lcd_console_write(char* theString);
		void service_lcd_console();
		void lcd_console_pwr_adj_set();
		uint8_t lcd_console_pwr_adj(uint8_t channel, uint8_t command);
		void lcd_console_meter(uint8_t channel, uint8_t side);
#endif
