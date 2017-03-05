// ATracknophilia.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <iostream>
#include "Net.h"

void eh(Message m)
{
	std::cout << m.data << std::endl;
}

int main()
{
	Net n;
	n.setMessageCallback(eh);

	//run the game loop
	bool quit = false;

	while (!quit)
	{
		
		n.update();
	}

    return 0;
}

