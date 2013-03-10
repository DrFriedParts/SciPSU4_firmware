#ifndef __brain_h
#define __brain_h

	//DEFINITIONS
		//these numbers matter -- don't change unless you are sure
		#define CHANNEL_A 0
		#define CHANNEL_B 1
		#define CHANNEL_C 2
		#define CHANNEL_D 3
		#define CHANNEL_M 5
		#define CHANNEL_ALL 58
		#define CHANNEL_RESTORE 60

		#define MENU_STARTUP 31
		#define MENU_OUTPUT 32		
		#define MENU_CONTROL 33
		#define MENU_CONSOLE 34
		#define MENU_DETAIL_AB 35
		#define MENU_DETAIL_CD 36
		#define MENU_DIAL_A 37
		#define MENU_DIAL_B 38
		#define MENU_DIAL_C 39
		#define MENU_DIAL_D 40
		
		#define BRAIN_BEEPS 1
		#define BRAIN_VOLUME 100
		
	//GLOBAL STATE
		uint8_t STATE_power_output; //Master Output Mute Status: ENABLE or DISABLE
		uint8_t STATE_power_channels; //Channel map: B8(0000DCBA), ex. bit B=0 channel B disabled, B=1 channel B output (possibly, gated by master) enabled
		uint8_t STATE_menu; //Which menu are we currently displaying?

	//FUNCTIONS
		void init_brain();
		void brain_debug();
		
		void brain_power(uint8_t which);
		void brain_power_on(uint8_t which);
		void brain_power_off(uint8_t which);
		void brain_power_reset();
		void brain_power_master();
		
		void brain_button_pressed();
		
		void brain_menu_load(uint8_t which_menu);
		void brain_rotary_change(uint8_t which_way);
		
		void brain_menu_output();
		void brain_menu_output_detail(uint8_t which_detail);
		
		void brain_menu_control();
		void brain_menu_control_dial(uint8_t which_channel);
		void brain_menu_control_dial_select(uint8_t which_one);
		
		void brain_menu_console();
		void brain_menu_update();
		
		void service_brain();
#endif
