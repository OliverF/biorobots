#define F_CPU 8000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <util/delay.h>
#include <stdlib.h>

#include "bluetooth.hpp"
#include "uart.hpp"
#include "pinmaniputation.hpp"

#define DIR_BW 2
#define DIR_FW 1
#define DIR_NO 0

#define VMAX 3.3f
#define V_GRIP_THRESHOLD_CLOSE 0.9f
#define V_GRIP_THRESHOLD_OPEN 0.9f

#define TIMEOUT_OVERFLOW_LIMIT 15

volatile uint16_t timeoutOverflowCount = 0;

void resetTimeout()
{
	timeoutOverflowCount = 0;
}

uint16_t readADC(uint8_t channel)
{
	//constrain channel to 0-7
	channel &= 0b00000111;
	
	//clear lower 3 bits and set channel
	ADMUX = (ADMUX & 0xF8) | channel;
	
	//start single conversion
	ADCSRA |= (1<<ADSC);
	
	//wait for conversion
	while(ADCSRA & (1 << ADSC));
	
	return ADC;
}

bool isGripperGripping(float threshold)
{
	float result = VMAX*((float)readADC(PA0)/1024);
	return (result > threshold);
}

void setGripperMotor(int direction)
{
	switch (direction)
	{
		case DIR_NO:
			LOW(PORTC, PINC0);
			LOW(PORTC, PINC1);
			break;
		case DIR_FW:
			LOW(PORTC, PINC0);
			HIGH(PORTC, PINC1);
			break;
		case DIR_BW:
			HIGH(PORTC, PINC0);
			LOW(PORTC, PINC1);
			break;
	}
}

void setTankDrive(double powerL, double powerR)
{
	resetTimeout();
	
	int dirL = 0;
	int dirR = 0;
	
	if (powerL != 0)
		dirL  = (powerL > 0) ? 1 : -1;
	
	if (powerR != 0)
		dirR  = (powerR > 0) ? 1 : -1;
	
	//account for negative power levels
	powerL *= dirL;
	powerR *= dirR;
	
	//scale to 0-255
	uint8_t npowerA = (uint8_t)floorf(255*powerL);
	uint8_t npowerB = (uint8_t)floorf(255*powerR);
	
	if (dirL == 0)
	{
		INPUT(DDRB, PB3);
		INPUT(DDRB, PB4);
		
		OCR0A = 0;
		OCR0B = 0;
	}
	else if (dirL >= 0)
	{
		INPUT(DDRB, PB3);
		OUTPUT(DDRB, PB4);
		
		OCR0A = 0;
		OCR0B = npowerA;
	}
	else
	{
		OUTPUT(DDRB, PB3);
		INPUT(DDRB, PB4);
		
		OCR0A = npowerA;
		OCR0B = 0;
	}
	
	if (dirR == 0)
	{
		INPUT(DDRD, PIND7);
		INPUT(DDRD, PIND6);
		
		OCR2A = 0;
		OCR2B = 0;
	}
	else if (dirR >= 0)
	{
		OUTPUT(DDRD, PIND7);
		INPUT(DDRD, PIND6);
		
		OCR2A = npowerB;
		OCR2B = 0;
	}
	else
	{
		INPUT(DDRD, PIND7);
		OUTPUT(DDRD, PIND6);
		
		OCR2A = 0;
		OCR2B = npowerB;
	}
}

ISR(TIMER1_OVF_vect)
{
	timeoutOverflowCount++;
	
	if (timeoutOverflowCount >= TIMEOUT_OVERFLOW_LIMIT)
	{
		//timed out, stop driving
		setTankDrive(0, 0);
		
		//reset overflow timer
		timeoutOverflowCount = 0;
	}
}

void driveUp()
{
	setTankDrive(1, 1);
}

void driveDown()
{
	setTankDrive(-1, -1);
}

void driveLeft()
{
	setTankDrive(-1, 1);
}

void driveRight()
{
	setTankDrive(1, -1);
}

void driveUpRight()
{
	setTankDrive(1, 0.7);
}

void driveUpLeft()
{
	setTankDrive(0.7, 1);
}

void driveDownRight()
{
	setTankDrive(-1, -0.5);
}

void driveDownLeft()
{
	setTankDrive(-0.5, -1);
}

void driveStop()
{
	setTankDrive(0, 0);
}

void gripperClose()
{
	setTankDrive(0, 0);
	
	setGripperMotor(DIR_FW);
	
	_delay_ms(500);
	
	while(!isGripperGripping(V_GRIP_THRESHOLD_CLOSE));
	
	setGripperMotor(DIR_NO);
}

void gripperOpen()
{
	setTankDrive(0, 0);
	
	setGripperMotor(DIR_BW);
	
	_delay_ms(500);
	
	while(!isGripperGripping(V_GRIP_THRESHOLD_OPEN)){}
	
	setGripperMotor(DIR_NO);
}

void initADC()
{
    //AREF = AVcc
    ADMUX = (1<<REFS0);
    
    //enable ADC and set prescaler of 128
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

void initPWM()
{
	//Left motor - timer 0
	//0% duty cycle @8bit
	OCR0A = 0x00;
	
	//0% duty cycle @8bit
	OCR0B = 0x00;
	
	//set non-inverting mode
	TCCR0A |= (1 << COM0A1);
	TCCR0A |= (1 << COM0B1); // Workaround: http://electronics.stackexchange.com/questions/97596/attiny85-pwm-why-does-com1a0-need-to-be-set-before-pwm-b-will-work
	
	//fast PWM mode
	TCCR0A |= (1 << WGM01) | (1 << WGM00);
	
	//prescaler = 8, start PWM
	TCCR0B |= (1 << CS01);
	
	//Right motor - timer 2
	//0% duty cycle @8bit
	OCR2A = 0x00;
		
	//0% duty cycle @8bit
	OCR2B = 0x00;
		
	//set non-inverting mode
	TCCR2A |= (1 << COM2A1);
	TCCR2A |= (1 << COM2B1);
		
	//fast PWM mode
	TCCR2A |= (1 << WGM21) | (1 << WGM20);
		
	//prescaler = 8, start PWM
	TCCR2B |= (1 << CS21);
}

void initTimeout()
{
	//enable overflow interrupt timer 0 (8 bit)
	TIMSK1 = (1 << TOIE1);
	//reset timer
	TCNT1 = 0x00;
	//1024 prescaler:
	//8000000/(8*65536) = ~15 ticks per second
	TCCR1B = (1 << CS11);
	//enable global interrupts
	sei();
}

void init()
{
	//Gripper motor
	OUTPUT(DDRC, PINC0);
	OUTPUT(DDRC, PINC1);
	
	//Left motor - PWM0
	OUTPUT(DDRB, PINB3);
	OUTPUT(DDRB, PINB4);
	
	//Right motor - PWM2
	OUTPUT(DDRD, PIND6);
	OUTPUT(DDRD, PIND7);
	
	initPWM();
	initADC();
	initTimeout();
	
	bluetooth::init();
}

int main(void)
{
	init();
	
	uart::transmitString("Initialized\r\n");
	
	bluetooth::registerCommandCallback("ctl-du\r\n", driveUp);
	bluetooth::registerCommandCallback("ctl-dd\r\n", driveDown);
	bluetooth::registerCommandCallback("ctl-dl\r\n", driveLeft);
	bluetooth::registerCommandCallback("ctl-dr\r\n", driveRight);
	
	bluetooth::registerCommandCallback("ctl-dur\r\n", driveUpRight);
	bluetooth::registerCommandCallback("ctl-dul\r\n", driveUpLeft);
	bluetooth::registerCommandCallback("ctl-ddr\r\n", driveDownRight);
	bluetooth::registerCommandCallback("ctl-ddl\r\n", driveDownLeft);
	
	bluetooth::registerCommandCallback("ctl-ds\r\n", driveStop);
	
	bluetooth::registerCommandCallback("ctl-cl\r\n", gripperClose);
	bluetooth::registerCommandCallback("ctl-op\r\n", gripperOpen);
	
	uart::transmitString("Closing gripper...\r\n");
	
	uart::transmitString("Gripper closed\r\n");
	
	while(true)
	{
		bluetooth::process();
	}
}