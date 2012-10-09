#ifndef __pwr_adj_h
#define __pwr_adj_h

	#define PWR_ADJ_NUM_CHANNELS 8
	uint16_t pwr_adj_values[PWR_ADJ_NUM_CHANNELS];
	
	uint8_t pwr_adj_send_channels; //flag register -- signals which channels to transmit
	uint8_t pwr_adj_channel_num_in_progress;
	uint8_t pwr_adj_spi_state;
	uint8_t pwr_adj_channel_dirty; //has this channel been updated since last screen update?
	uint8_t pwr_adj_left_right; //which adjustment knob is selected in the dial window
	
	#define PWR_ADJ_SPI_IDLE 0
	#define PWR_ADJ_SPI_COMMAND 1
	#define PWR_ADJ_SPI_DATAH 2
	#define PWR_ADJ_SPI_DATAL 3
 	
	#define MAX5494_SET1 0x01
	#define MAX5494_SET2 0x02
	
	#define PWR_ADJ_SUPPLY_VOLTAGE 5.0
	 
	void init_pwr_adjust();
	void pwr_adj_on(uint8_t channel);
	void pwr_adj_off(uint8_t channel);
	void pwr_adj_spi_send(uint8_t channel_num);
	void pwr_adj_change(uint8_t channel_num, uint16_t new_value);
	void pwr_adj_change_increment(uint8_t channel_num, uint8_t which_way);
	void pwr_adj_data(uint8_t channel_num, char* result);
	void pwr_adj_control(uint8_t channel_num, char* result);
	void pwr_adj_touch_dial(uint8_t which_one);
	void service_pwr_adjust();
#endif
