#include <avr/io.h>

#ifndef PINMANIPUTATION_H_
#define PINMANIPUTATION_H_
	#define HIGH(port, pin) port |= (1 << pin)
	#define LOW(port, pin) port &= ~(1 << pin)
	
	#define OUTPUT(port, pin) HIGH(port, pin)
	#define INPUT(port, pin) LOW(port, pin)

#endif /* PINMANIPUTATION_H_ */