#include <avr/io.h>
#include <string.h>
#include "bluetooth.hpp"
#include "uart.hpp"
#include "interface.hpp"

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
		//Error, command is too large
		clearCommandBuffer();
	}
	
	char c = uart::receiveByte();
	bluetooth::commandBuffer[currentCommandIndex] = c;
	bluetooth::commandBuffer[currentCommandIndex + 1] = '\0';
	
	if (checkAndExecuteCommand())
	{
		//Command executed, reset
		clearCommandBuffer();
	}
	
	currentCommandIndex++;
}

void bluetooth::registerCommandCallback(char* cmd, commandCallback callback)
{
	commandMapping* mapping = new commandMapping(cmd, callback);
	commandMappings[currentMappingIndex] = mapping;
	currentMappingIndex++;
}

int bluetooth::checkAndExecuteCommand()
{
	for (int i = 0; i < currentMappingIndex; i++)
	{
		commandMapping cmap = *commandMappings[currentMappingIndex];
		if(strcmp(cmap.command, commandBuffer) == 0)
		{
			cmap.callback();
			return 1;
		}
	}
	
	return 0;
}

void bluetooth::clearCommandBuffer()
{
	memset(bluetooth::commandBuffer, 0, sizeof(bluetooth::commandBuffer));
	currentCommandIndex = 0;
}