#include "../../Net.h"
#include <iostream>
#include "../../NetClient.h"

int main()
{
	ducklib::InitializeNet();

	uint16_t port;

	std::cout << "Enter which port to listen to: ";
	std::cin >> port;

	ducklib::NetClient netClient(port);

	while (true)
	{
		netClient.ReceivePacket();

		Sleep(400);
	}

	ducklib::ShutdownNet();

	return 0;
}
