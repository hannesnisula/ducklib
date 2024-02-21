#include "net/Net.h"

#include <winsock2.h>
#include <exception>

namespace ducklib
{
void net_initialize()
{
	WSAData data;

	if (WSAStartup(MAKEWORD(2, 2), &data) == SOCKET_ERROR)
	{
		int errorCode = WSAGetLastError();
		throw std::exception();
	}
}

void net_shutdown()
{
	if (WSACleanup() == SOCKET_ERROR)
	{
		int errorCode = WSAGetLastError();
		throw std::exception();
	}
}
}
