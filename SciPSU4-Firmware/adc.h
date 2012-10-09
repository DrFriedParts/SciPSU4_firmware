#ifndef __adc_h
#define __adc_h

	//DEFINITIONS
		
	//GLOBAL STATE
		#define ADC_NUM_CHANNELS 16
		#define ADC_NUM_CONVERSIONS 8
		uint16_t adc_results[ADC_NUM_CHANNELS][ADC_NUM_CONVERSIONS];
		uint8_t adc_bank;
		uint8_t adc_head;
	
	//VOLTAGE CONVERSION
		//5% of Vref, nominally (0.1031V); Vref = 62.5% of VCC (2.0625V); VCC = 3.3V
		//Covert ADC code to voltage (as seen by ADC)
		#define CODE_TO_VOLTS 0.00048763
		#define CODE_ZERO 161
	
	//VOLTAGE MEASUREMENT
		//Undo the input divider (prescaler)
		#define VOLTAGE_DESCALE_FACTOR 13.12
		
	//VOLTAGE (NEGATIVE) MEASUREMENT
		#define VOLTAGE_NEG_OFFSET 180
		//#define VOLTAGE_NEG_OFFSET 512
	
	//CURRENT MEASUREMENT
		//0.020 ohm sense resistor
		#define CODE_TO_AMPS 0.000592
		uint16_t adc_current_offset[ADC_NUM_CHANNELS/2];
		
	//MEASUREMENT TYPES
		#define VOLTAGE_POS 103
		#define VOLTAGE_NEG 104
		#define CURRENT_LO_RES 105
		#define CURRENT_HI_RES 106
	
	//FUNCTIONS
		void init_adc();
		uint8_t adc_read_cal_byte( uint8_t index );
		void adc_bank_select(uint8_t which);
		void adc_convert();
		void adc_ftoa(float x, uint16_t scaler, char* result);
		void adc_data(uint8_t channel_num, uint8_t measurement_type, char* result);
		void service_adc();
#endif
