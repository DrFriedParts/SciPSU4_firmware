#include <avr/io.h>
#include <avr/interrupt.h>
#include "utilities.h"
#include "brain.h"
#include "fp.h"
#include "ui.h"
#include "string.h" 
#include "uart.h"
#include "uart_buffer.h"
#include "lcd.h"
#include "lcd_console.h"
#include "adc.h"
#include "pwr_adjust.h"
#include <stdlib.h>

//Handles EarthLCD ezLCD301 hardware

//#############################################################
//## INITIALIZATION ROUTINE
//#############################################################

void init_lcd_console(){
	char* line_buffer;
	//Load the command bytes into the console buffer
	for(uint8_t i=0;i<LCD_CONSOLE_NUM_ROWS;i++){
		line_buffer = lcd_console[i];
		line_buffer[0] = '8'; line_buffer[1]='8'; line_buffer[2]=' '; line_buffer[4]=' '; line_buffer[5]='"'; line_buffer[6]='"';		
		lcd_buffer_dirty[i] = false;
	}
	//Init state
	lcd_console_head = 0;
	STATE_lcd_console = LCD_CONSOLE_STATE_IDLE;
	lcd_console_set_value[4] = 0; //add string termination character
}

//#############################################################
//## HOST API SUPPORT FUNCTIONS
//#############################################################

void lcd_console_pwr_adj_set(){
	uint16_t new_value;
	//ASCII to uint10_t (0-1023)
	new_value = (uint16_t)atoi(lcd_console_set_value);
	//Change Dial
	pwr_adj_change(lcd_console_pwr_adj(lcd_console_channel, lcd_console_command), new_value);
}

/*
	Map channel, dial to channel number
*/
uint8_t lcd_console_pwr_adj(uint8_t channel, uint8_t command){
	switch(channel){
		case CHANNEL_A:
			switch(command){
				case 'm':
				case 'M':
					return 0;
				break;
				case 's':
				case 'S':
					return 1;
				break;
			}
			break;
		case CHANNEL_B:
			switch(command){
				case 'm':
				case 'M':
				return 2;
				break;
				case 's':
				case 'S':
				return 3;
				break;
			}
			break;
		case CHANNEL_C:
			switch(command){
				case 'm':
				case 'M':
				return 4;
				break;
				case 's':
				case 'S':
				return 5;
				break;
			}
			break;
		case CHANNEL_D:
			switch(command){
				case 'm':
				case 'M':
				return 6;
				break;
				case 's':
				case 'S':
				return 7;
				break;
			}
			break;
	}
	return 0; //SHOULD NEVER REACH HERE
}

/*
	Retrieve the current meter reading value (voltage or current)
	--returns value via lcd_console_meter_value
*/
void lcd_console_meter(uint8_t channel, uint8_t side){
	switch(channel){
		case CHANNEL_A:
			switch(side){
				case LCD_CONSOLE_CMD_MAIN_POS:
					switch(lcd_console_value_1){
						case 'v':
						case 'V':
							adc_data(0, VOLTAGE_POS, lcd_console_meter_value); //A V+
						break;
						case 'i':
						case 'I':
							adc_data(1, CURRENT_HI_RES, lcd_console_meter_value); //A I+
						break;
					}//Pos: V or I
				break;
				case LCD_CONSOLE_CMD_MAIN_NEG:
					switch(lcd_console_value_1){
						case 'v':
						case 'V':
							adc_data(2, VOLTAGE_NEG, lcd_console_meter_value); //A V-
						break;
						case 'i':
						case 'I':
							adc_data(3, CURRENT_HI_RES, lcd_console_meter_value); //A I-
						break;
					}//Neg: V or I
				break;
			}//side		
		break; //Channel A
		
		case CHANNEL_B:
		switch(side){
			case LCD_CONSOLE_CMD_MAIN_POS:
			switch(lcd_console_value_1){
				case 'v':
				case 'V':
				adc_data(4, VOLTAGE_POS, lcd_console_meter_value); //B V+
				break;
				case 'i':
				case 'I':
					adc_data(5, CURRENT_HI_RES, lcd_console_meter_value); //B I+
				break;
			}//Pos: V or I
			break;
			case LCD_CONSOLE_CMD_MAIN_NEG:
			switch(lcd_console_value_1){
				case 'v':
				case 'V':
					adc_data(6, VOLTAGE_NEG, lcd_console_meter_value); //B V-
				break;
				case 'i':
				case 'I':
					adc_data(7, CURRENT_HI_RES, lcd_console_meter_value); //B I-
				break;
			}//Neg: V or I
			break;
		}//side
		break; //Channel B
		
		case CHANNEL_C:
		switch(side){
			case LCD_CONSOLE_CMD_MAIN_POS:
			switch(lcd_console_value_1){
				case 'v':
				case 'V':
					adc_data(8, VOLTAGE_POS, lcd_console_meter_value); //C V+
				break;
				case 'i':
				case 'I':
					adc_data(9, CURRENT_HI_RES, lcd_console_meter_value); //C I+
				break;
			}//Pos: V or I
			break;
			case LCD_CONSOLE_CMD_MAIN_NEG:
			switch(lcd_console_value_1){
				case 'v':
				case 'V':
					adc_data(10, VOLTAGE_NEG, lcd_console_meter_value); //C V-
				break;
				case 'i':
				case 'I':
					adc_data(11, CURRENT_HI_RES, lcd_console_meter_value); //C I-
				break;
			}//Neg: V or I
			break;
		}//side
		break; //Channel C
		
		case CHANNEL_D:
		switch(side){
			case LCD_CONSOLE_CMD_MAIN_POS:
			switch(lcd_console_value_1){
				case 'v':
				case 'V':
					adc_data(12, VOLTAGE_POS, lcd_console_meter_value); //D V+
				break;
				case 'i':
				case 'I':
					adc_data(13, CURRENT_HI_RES, lcd_console_meter_value); //D I+
				break;
			}//Pos: V or I
			break;
			case LCD_CONSOLE_CMD_MAIN_NEG:
			switch(lcd_console_value_1){
				case 'v':
				case 'V':
					adc_data(14, VOLTAGE_NEG, lcd_console_meter_value); //D V-
				break;
				case 'i':
				case 'I':
					adc_data(15, CURRENT_HI_RES, lcd_console_meter_value); //D I-
				break;
			}//Neg: V or I
			break;
		}//side
		break; //Channel D
	}//channel		
}		


//#############################################################
//## SCREENS: CONSOLE
//#############################################################

void lcd_console_write(char* theString){
	int8_t head = (int8_t)lcd_console_head;
	char* line_buffer = lcd_console[lcd_console_head];
	//Destination
	for(uint8_t i=0;i<LCD_CONSOLE_NUM_ROWS;i++){
		lcd_console[head][3] = 0x31 + i; //id of static control (Row) to write to 
		head--;
		if(head < 0){head = LCD_CONSOLE_NUM_ROWS-1;}
	}	
	//Content
	for (uint8_t i=0; i<LCD_CONSOLE_NUM_COLS; i++){
		if (theString[i] == 0){
			line_buffer[6+i] = '"';
			line_buffer[6+i+1] = 0;
			break;
		}
		line_buffer[6+i] = theString[i];
	}
	//Dirty all rows (so all rows get printed to screen)
	for (uint8_t i=0; i<LCD_CONSOLE_NUM_ROWS; i++){lcd_buffer_dirty[i] = true;}	
	//Move row pointer
	lcd_console_head++;
	if (lcd_console_head >= LCD_CONSOLE_NUM_ROWS){lcd_console_head = 0;}
}

 

//#############################################################
//## SERVICE ROUTINE
//#############################################################

//Used to refresh the console
void service_lcd_console(){
	//Process command buffer
	while (uart_icount(&uctrl) > 0){
		uint8_t incoming = uart_idequeue(&uctrl);
		lcd_console_write("Byte received"); //xxx
		switch(STATE_lcd_console){
			
			case LCD_CONSOLE_STATE_IDLE:
				lcd_console_write("IDLE"); //xxx
				switch(incoming){
					case 'a':
					case 'A':
						STATE_lcd_console = LCD_CONSOLE_STATE_COMMAND;
						lcd_console_channel = CHANNEL_A;
						break;
					case 'b':
					case 'B':
						STATE_lcd_console = LCD_CONSOLE_STATE_COMMAND;
						lcd_console_channel = CHANNEL_B;
						break;
					case 'c':
					case 'C':
						STATE_lcd_console = LCD_CONSOLE_STATE_COMMAND;
						lcd_console_channel = CHANNEL_C;
						break;
					case 'd':
					case 'D':
						STATE_lcd_console = LCD_CONSOLE_STATE_COMMAND;
						lcd_console_channel = CHANNEL_D;
					break;
				}
			break;
			
			case LCD_CONSOLE_STATE_COMMAND:
				lcd_console_write("CMD"); //xxx
				switch(incoming){
					case 'z':
					case 'Z':
						STATE_lcd_console = LCD_CONSOLE_STATE_VALUE_1;
						lcd_console_command = LCD_CONSOLE_CMD_RELAY;
						break;
					case 'p':
					case 'P':
						STATE_lcd_console = LCD_CONSOLE_STATE_VALUE_1;
						lcd_console_command = LCD_CONSOLE_CMD_MAIN_POS;
						break;
					case 'n':
					case 'N':
						STATE_lcd_console = LCD_CONSOLE_STATE_VALUE_1;
						lcd_console_command = LCD_CONSOLE_CMD_MAIN_NEG;
						break;
					case 'm':
					case 'M':
						STATE_lcd_console = LCD_CONSOLE_STATE_VALUE_N;
						lcd_console_command = LCD_CONSOLE_CMD_ADJUST_MAX;
						break;
					case 's':
					case 'S':
						STATE_lcd_console = LCD_CONSOLE_STATE_VALUE_N;
						lcd_console_command = LCD_CONSOLE_CMD_ADJUST_SET;
						break;
					default:
						//RESET ON ERROR
						lcd_console_write("Bad Command");
						STATE_lcd_console = LCD_CONSOLE_STATE_IDLE;
				}
			break;
			
			case LCD_CONSOLE_STATE_VALUE_1:
				lcd_console_write("VAL1"); //xxx
				switch(incoming){
					case '1':
					case '0':
					case 'v':
					case 'V':
					case 'i':
					case 'I':
						STATE_lcd_console = LCD_CONSOLE_STATE_TERMINATOR_1;
						lcd_console_value_1 = incoming;
						break;
					default:
						//RESET ON ERROR
						lcd_console_write("Bad Command");
						STATE_lcd_console = LCD_CONSOLE_STATE_IDLE;
				}
			break;
			
			case LCD_CONSOLE_STATE_VALUE_N:
				lcd_console_write("VALN-1"); //xxx
				switch(incoming){
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
						STATE_lcd_console = LCD_CONSOLE_STATE_VALUE_N2;
						lcd_console_set_value[0] = incoming;
						break;
					default:
						//RESET ON ERROR
						lcd_console_write("Bad Command");
						STATE_lcd_console = LCD_CONSOLE_STATE_IDLE;
				}
			break;
			
			case LCD_CONSOLE_STATE_VALUE_N2:
			lcd_console_write("VALN-2"); //xxx
			switch(incoming){
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					STATE_lcd_console = LCD_CONSOLE_STATE_VALUE_N3;
					lcd_console_set_value[1] = incoming;
					break;
				case 0x0D:
					lcd_console_set_value[1] = 0; //terminate string
					lcd_console_pwr_adj_set();
					STATE_lcd_console = LCD_CONSOLE_STATE_IDLE;
					break;
				default:
					//RESET ON ERROR
					lcd_console_write("Bad Command");
					STATE_lcd_console = LCD_CONSOLE_STATE_IDLE;
			}
			break;
			
			case LCD_CONSOLE_STATE_VALUE_N3:
			lcd_console_write("VALN-3"); //xxx
			switch(incoming){
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					STATE_lcd_console = LCD_CONSOLE_STATE_VALUE_N4;
					lcd_console_set_value[2] = incoming;
					break;
				case 0x0D:
					lcd_console_set_value[2] = 0; //terminate string
					lcd_console_pwr_adj_set();
					STATE_lcd_console = LCD_CONSOLE_STATE_IDLE;
					break;
				default:
					//RESET ON ERROR
					lcd_console_write("Bad Command");
					STATE_lcd_console = LCD_CONSOLE_STATE_IDLE;
			}
			break;
			
			case LCD_CONSOLE_STATE_VALUE_N4:
			lcd_console_write("VALN-4"); //xxx
			switch(incoming){
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					STATE_lcd_console = LCD_CONSOLE_STATE_TERMINATOR_N;
					lcd_console_set_value[3] = incoming;
					break;
				case 0x0D:
					lcd_console_set_value[3] = 0; //terminate string
					lcd_console_pwr_adj_set();
					STATE_lcd_console = LCD_CONSOLE_STATE_IDLE;
				break;
				default:
					//RESET ON ERROR
					lcd_console_write("Bad Command");
					STATE_lcd_console = LCD_CONSOLE_STATE_IDLE;
			}
			break;
			
			case LCD_CONSOLE_STATE_TERMINATOR_N:
				lcd_console_write("TERM_N"); //xxx
				switch(incoming){
					case 0x0D:
						lcd_console_pwr_adj_set();
						break;
					default:
						//RESET ON ERROR
						lcd_console_write("Bad Command");
				}
				STATE_lcd_console = LCD_CONSOLE_STATE_IDLE;
			break;
			
			case LCD_CONSOLE_STATE_TERMINATOR_1:
				lcd_console_write("TERM1"); //xxx
				switch(incoming){
					case 0x0D:
						switch(lcd_console_value_1){
							case '1':
								STATE_lcd_console = LCD_CONSOLE_STATE_IDLE;
								brain_power_on(lcd_console_channel);
								lcd_console_write("POWER ON");
								break;
							case '0':
								STATE_lcd_console = LCD_CONSOLE_STATE_IDLE;
								brain_power_off(lcd_console_channel);
								lcd_console_write("POWER OFF");
								break;
							case 'v':
							case 'V':
							case 'i':
							case 'I':
								STATE_lcd_console = LCD_CONSOLE_STATE_IDLE;
								//Report current or voltage on selected channel
								lcd_console_meter(lcd_console_channel, lcd_console_command);
								uart_enqueue_string(&uctrl, lcd_console_meter_value);
								break;
							default:
								//RESET ON ERROR
								lcd_console_write("Bad Command");
								STATE_lcd_console = LCD_CONSOLE_STATE_IDLE;
						}
						break;
					default:
						//RESET ON ERROR
						lcd_console_write("Bad Command");
						STATE_lcd_console = LCD_CONSOLE_STATE_IDLE;
				}						
			break;
		}
	}
	
	//Update console display if in CONSOLE MENU
	static uint8_t next_row_to_output = 0;
	uint8_t num_rows_output = 0;
	if (STATE_menu != MENU_CONSOLE){return;}
	while((uart_count(&ulcd) < 512)&&(num_rows_output<LCD_CONSOLE_NUM_ROWS)){
		if(lcd_buffer_dirty[next_row_to_output]){
			lcd_command(lcd_console[next_row_to_output]);
			lcd_buffer_dirty[next_row_to_output] = false;
		}
		next_row_to_output++; num_rows_output++;
		if(next_row_to_output>=LCD_CONSOLE_NUM_ROWS){next_row_to_output=0;}
	}
}
