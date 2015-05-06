#ifndef BLUETOOTH_H_
#define BLUETOOTH_H_

#include "interface.hpp"

struct commandMapping
{
	char* command;
	commandCallback callback;
	
	commandMapping(char* command, commandCallback callback)
	{
		this->command = command;
		this->callback = callback;
	}
};

class bluetooth
{
	public:
		static void init();
		static void process();
		static void registerCommandCallback(char* cmd, commandCallback callback);
	private:
		static char commandBuffer[20];
		static uint32_t currentCommandIndex;
		
		static commandMapping* commandMappings[20];
		static uint32_t currentMappingIndex;
		
		static void clearCommandBuffer();
		static int checkAndExecuteCommand();
};

#endif /* BLUETOOTH_H_ */