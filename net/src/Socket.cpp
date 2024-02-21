#include "net/Socket.h"
#include "net/Net.h"

#include <cassert>
#include <exception>
#include <WS2tcpip.h>

namespace ducklib
{
Socket::Socket(uint16_t bindPort)
	: socketHandle(INVALID_SOCKET)
{
	// Create socket and set options
	socketHandle = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (socketHandle == INVALID_SOCKET)
		DL_NET_FAIL("Failed to create socket");

	// Bind socket
	sockaddr_in socketAddress;

	socketAddress.sin_addr.s_addr = INADDR_ANY;
	socketAddress.sin_port = htons(bindPort);
	socketAddress.sin_family = AF_INET;

	if (bind(socketHandle, (sockaddr*)&socketAddress, sizeof(socketAddress)) < 0)
		DL_NET_FAIL("Failed to bind socket");

	// Get which port was bound
	sockaddr_in boundSocketAddress;
	int boundSocketAddressSize = sizeof boundSocketAddress;
	int boundNameResult = getsockname(socketHandle, (sockaddr*)&boundSocketAddress, &boundSocketAddressSize);

	if (boundNameResult != 0)
		DL_NET_FAIL("Failed to get bound address of socket (%d)", WSAGetLastError());

	this->address = Address(boundSocketAddress);

	// Set non-blocking mode
	DWORD nonBlockFlag = 1;
	if (ioctlsocket(socketHandle, FIONBIO, &nonBlockFlag) != 0)
		DL_NET_FAIL("Failed to set non-blocking mode on socket");
}

Socket::~Socket()
{
	assert(socketHandle);

	closesocket(socketHandle);
}

int Socket::GetPort() const
{
	assert(socketHandle);

	return address.get_port();
}

int Socket::Send(const Address* dest, const uint8_t* data, uint32 dataSize)
{
	if (socketHandle == INVALID_SOCKET)
		DL_NET_FAIL("Trying to send data over uninitialized socket");

	sockaddr_in socketAddress = dest->as_sockaddr_in();
	HRESULT result = sendto(socketHandle, (const char*)data, (int)dataSize, 0, (sockaddr*)&socketAddress, sizeof(socketAddress));

	if (result == SOCKET_ERROR)
		DL_NET_FAIL("Failed to send data over socket");

	return result;
}

int Socket::Receive(Address* fromAddress, uint8_t* buffer, uint32 bufferSize)
{
	assert(socketHandle != INVALID_SOCKET);
	assert(fromAddress);
	assert(buffer);
	assert(bufferSize > 0);

	sockaddr_in socketAddress;
	int socketAddressSize = sizeof(socketAddress);
	HRESULT result = recvfrom(socketHandle, (char*)buffer, (int)bufferSize, 0, (sockaddr*)&socketAddress, &socketAddressSize);

	// TODO: Check socket address size value?

	// TODO: Propagate this out to the caller
	if (result == SOCKET_ERROR)
	{
		int errorCode = WSAGetLastError();

		if (errorCode == WSAEWOULDBLOCK)
			return 0;

		DL_NET_FAIL("Failed to receive data over socket");
	}

	new(fromAddress) Address(socketAddress);

	return result;
}
}
