#include "Net.h"

#include <winsock2.h>
#include <exception>

namespace ducklib
{
void InitializeNet()
{
	WSAData data;

	if (WSAStartup(MAKEWORD(2, 2), &data) == SOCKET_ERROR)
	{
		int errorCode = WSAGetLastError();
		throw std::exception();
	}
}

void ShutdownNet()
{
	if (WSACleanup() == SOCKET_ERROR)
	{
		int errorCode = WSAGetLastError();
		throw std::exception();
	}
}
}
