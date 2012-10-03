#ifndef __ui_h
#define __ui_h

	//DEFINITIONS
		#define LED_0	0
		#define LED_1	1
		#define LED_3    2
		//AUDIO - CONFIGURATION
		#define AUDIO_BEEP_LENGTH 5
		#define AUDIO_GAP_LENGTH 50
		//AUDIO - STATES
		#define AUDIO_IDLE 36
		#define AUDIO_BEEPING 37
		#define AUDIO_BETWEEN 38
				
	//FUNCTIONS
		void init_ui();
		void led_on(uint8_t which);
		void led_off(uint8_t which);
		void led_toggle(uint8_t which);
		void led_dim(uint8_t which, uint8_t brightness);		
		void audio_beep(uint8_t num_beeps, uint16_t volume);
		void audio_volume(uint8_t volume);
		void service_audio();
#endif
