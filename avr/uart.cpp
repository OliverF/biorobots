#define F_CPU 8000000UL

#include "uart.hpp"
#include <avr/io.h>
#include <avr/interrupt.h>

void uart::init(unsigned int baud)
{
	int ubrr = F_CPU/16/baud-1;
	// set baud rate
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)(ubrr);

	// enable receiver and transmitter
	UCSR0B = (1<<RXEN0) | (1<<TXEN0);

	// set frame format to: 8 data bits, 1 stop bit
	UCSR0C = (3<<UCSZ00);
}

void uart::transmitByte(unsigned char data)
{
	// wait for empty buffer
	while(!(UCSR0A & (1<<UDRE0))){ }

	// place data in buffer
	UDR0 = data;
}

void uart::transmitPacket(unsigned char data[], unsigned int size)
{
	for(int i = 0; i < size; i++)
		uart::transmitByte(data[i]);
}

void uart::transmitString(unsigned char* data)
{
	uint32_t i = 0;
	
	while(data[i] != '\0')
	{
		uart::transmitByte(data[i++]);
	}
}

void uart::transmitString(const char* data)
{
	uart::transmitString((unsigned char*)data);
}

unsigned char uart::receiveByte(void)
{
	// wait for data to be received
	while(!(UCSR0A & (1<<RXC0))){ }

	// return data
	return UDR0;
}
