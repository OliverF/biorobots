#include <avr/io.h>
#include <stdio.h>
#include "bluetooth.hpp"

void driveUp()
{
	
}

void driveDown()
{
	
}

void driveLeft()
{
	
}

void driveRight()
{
	
}

void gripperUp()
{
	
}

void gripperDown()
{
	
}

void gripperLeft()
{
	
}

void gripperRight()
{
	
}

int main(void)
{
	bluetooth::registerCommandCallback("ctl-ru", driveUp);
	bluetooth::registerCommandCallback("ctl-rd", driveDown);
	bluetooth::registerCommandCallback("ctl-rl", driveLeft);
	bluetooth::registerCommandCallback("ctl-rr", driveRight);
	bluetooth::registerCommandCallback("ctl-gu", gripperUp);
	bluetooth::registerCommandCallback("ctl-gd", gripperDown);
	bluetooth::registerCommandCallback("ctl-gl", gripperLeft);
	bluetooth::registerCommandCallback("ctl-gr", gripperRight);
	
	while(true){ }
}