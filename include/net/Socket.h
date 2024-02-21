#pragma once

#include <winsock2.h>
#include "Shared.h"

namespace ducklib
{
class Socket
{
public:
	/**
	 *\param bindPort Port to bind socket to. 0 = OS chooses.
	 */
	explicit Socket(uint16_t bindPort = 0);
	~Socket();

	int GetPort() const;

	int Send(const Address* dest, const uint8_t* data, uint32_t dataSize);
	int Receive(Address* fromAddress, uint8_t* buffer, uint32_t bufferSize);

	// TODO: Consider adding a Close() function

protected:
	SOCKET socketHandle;
	Address address;
};
}
