#ifndef __fp_h
#define __fp_h

	//DEFINITIONS
		#define FP_ON_LENGTH 1
		#define FP_OFF_LENGTH 10
		
		#define FP_SWITCH_A 0
		#define FP_SWITCH_B 1
		#define FP_SWITCH_C 2
		#define FP_SWITCH_D 3
		#define FP_SWITCH_M 4
		#define FP_SWITCH_R 5

	//LED STATE
		uint8_t fp_channel_mask;
		uint8_t fp_counter; //used to implement standby dimming
		uint8_t fp_counter2; //used to implement pulsing master LED
		uint8_t fp_updown; //used to implement pulsing master LED
		uint16_t fp_press_counter; //used to time the rotary switch's button pressed length
		uint8_t fp_master_status; //master switch status (PRESSED, RELEASED)
		uint8_t fp_rot_status; //rotary switch status (PRESSED, RELEASED)
	
	//BUTTON STATE
		uint8_t fp_button_time0;
		uint8_t fp_button_time1;
		uint8_t fp_button_time2;
		
	//FUNCTIONS
		void init_fp();
		void fp_led_enable(uint8_t which);
		void fp_led_disable(uint8_t which);		
		void service_fp();
#endif
