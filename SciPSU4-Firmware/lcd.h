#ifndef __lcd_h
#define __lcd_h

	//DEFINITIONS
		//Place all Operating states lower than 0x90
		#define LCD_BUSY 0x4E
		#define LCD_DONE_COMMAND 0x4F
		#define LCD_READY 0x52
		//Place all Booting states higher than 0x90
		#define LCD_BOOTING 0x90
		#define LCD_REBOOT 0x91
		#define LCD_REBOOT_MENU 0x92
	
		#define LCD_ENABLED 31
		#define LCD_STANDBY 32
		#define LCD_DISABLED 33
		
	//UART BUFFER STATE		
		volatile uint8_t lcd_flow_control;
		volatile uint8_t lcd_flow_reboot; //recovery mechanism 

	//OUTPUT & CONTROL SCREENS
		void lcd_a(uint8_t mode);
		void lcd_b(uint8_t mode);
		void lcd_c(uint8_t mode);
		void lcd_d(uint8_t mode);
	
	//FUNCTIONS
		void init_lcd();
		void lcd_command(char* theCommand);
		void lcd_update(char* theCommand, char* theValue);
		void lcd_reboot();
		void service_lcd();
#endif
