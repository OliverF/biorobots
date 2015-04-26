#include <avr/io.h>
#include <string.h>
#include "bluetooth.hpp"
#include "uart.hpp"
#include "interface.hpp"

#define DEBUGBT

char bluetooth::commandBuffer[20];
commandMapping* bluetooth::commandMappings[20];
uint32_t bluetooth::currentCommandIndex = 0;
uint32_t bluetooth::currentMappingIndex = 0;


void bluetooth::init()
{
	uart::init(9600);
}

void bluetooth::process()
{
	if(currentCommandIndex + 1 >= sizeof(commandBuffer))
	{
		uart::transmitString("Command buffer full, clearing\r\n");
		//Error, command is too large
		clearCommandBuffer();
	}
	
	char c = uart::receiveByte();
	bluetooth::commandBuffer[currentCommandIndex] = c;
	bluetooth::commandBuffer[currentCommandIndex + 1] = '\0';
	
	if (checkAndExecuteCommand())
	{
		uart::transmitString("Command executed, clearing buffer\r\n");
		//Command executed, reset
		clearCommandBuffer();
	}
	else
	{
		//Only increment if we did not reset the buffer, otherwise we'll be starting at index 1
		currentCommandIndex++;
	}
}

void bluetooth::registerCommandCallback(char* cmd, commandCallback callback)
{
	commandMapping* mapping = new commandMapping(cmd, callback);
	
	uart::transmitString("Callback registered: ");
	uart::transmitString(mapping->command);
	uart::transmitString("\r\n");
	
	commandMappings[currentMappingIndex] = mapping;
	currentMappingIndex++;
}

int bluetooth::checkAndExecuteCommand()
{
	for (int i = 0; i < currentMappingIndex; i++)
	{
		commandMapping cmap = *commandMappings[i];
		if(strcmp(cmap.command, commandBuffer) == 0)
		{
			uart::transmitString("Executing callback\r\n");
			cmap.callback();
			return 1;
		}
		#ifdef DEBUGBT
			else
			{
				uart::transmitString(cmap.command);
				uart::transmitString(" != ");
				uart::transmitString(commandBuffer);
				uart::transmitString("\r\n");
			}
		#endif
	}
	
	return 0;
}

void bluetooth::clearCommandBuffer()
{
	memset(bluetooth::commandBuffer, 0, sizeof(bluetooth::commandBuffer));
	currentCommandIndex = 0;
}