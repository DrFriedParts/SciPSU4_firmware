#ifndef __quad_h
#define __quad_h

	//DEFINITIONS
		#define RISING_EDGE 	B8(10011001) //Slew rate limiter on; Internal pull-up on; Sense on rising input edges
		#define FALLING_EDGE 	B8(10011010) //Slew rate limiter on; Internal pull-up on; Sense on falling input edges
		
		#define QUAD_IDLE 44
		#define QUAD_UP 45
		#define QUAD_DOWN 46
	//STATE
		uint16_t quad_count;
		uint8_t quad_state;
	
	//FUNCTIONS
		void init_quadrature();
		boolean quad_up();
		boolean quad_down();
		void service_quadrature();
#endif
