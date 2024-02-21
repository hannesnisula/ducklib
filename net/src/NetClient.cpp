#include <winsock2.h>
#include <cassert>
#include "net/NetClient.h"
#include "net/Net.h"

namespace ducklib
{
NetClient::NetClient(uint16_t bindPort)
	: socket(bindPort)
{
	// Init sequence numbers to never match possible packet sequence numbers (16 bits)
	for (int i = 0; i < MAX_FRAGMENT_PACKET_BUFFER_SIZE; ++i)
		receivedFragmentPackets[i].sequence = -1;
}

bool NetClient::send_packet(const Address* to, const uint8_t* data, int dataSize)
{
	assert(to);
	assert(data);
	assert(dataSize > 0);

	uint8_t packetSendBuffer[MAX_PACKET_SIZE];

	if (dataSize > MAX_PAYLOAD_SIZE)
	{
		// Ceiling function from Number Conversion (2001) by Roland Backhouse
		const int fragmentCount = (dataSize + MAX_FRAGMENT_PAYLOAD_SIZE - 1) / MAX_FRAGMENT_PAYLOAD_SIZE;

		for (int i = 0; i < fragmentCount; ++i)
		{
			// Construct fragment packet
			const int remainingDataSize = dataSize - i * MAX_FRAGMENT_PAYLOAD_SIZE;
			const int fragmentPayloadSize = remainingDataSize > MAX_FRAGMENT_PAYLOAD_SIZE ? MAX_FRAGMENT_PAYLOAD_SIZE : remainingDataSize;
			const int packetSize = fragmentPayloadSize + BASE_HEADER_SIZE + FRAGMENT_SUB_HEADER_SIZE;

			assert(packetSize <= MAX_PACKET_SIZE);

			const BasePacketHeader header{ nextSequence, PacketType::FRAGMENT };
			const FragmentPacketSubHeader fragmentHeader{ (uint8_t)fragmentCount, (uint8_t)i };

			WriteBasePacketHeader(packetSendBuffer, &header);
			WriteFragmentPacketHeader(&packetSendBuffer[BASE_HEADER_SIZE], &fragmentHeader);
			memcpy(
				&packetSendBuffer[BASE_HEADER_SIZE + FRAGMENT_SUB_HEADER_SIZE],
				&data[(uint64_t)i * MAX_FRAGMENT_PAYLOAD_SIZE],
				fragmentPayloadSize);

			// Send packet
			const int sizeSent = socket.Send(to, packetSendBuffer, packetSize);

			if (sizeSent != packetSize)
			{
				DL_NET_LOG_ERROR("Failed to send fragment (#%d) packet of size %d", i, packetSize);
				return false;
			}
		}
	}
	else
	{
		// Construct regular packet
		const int packetSize = BASE_HEADER_SIZE + dataSize;
		const BasePacketHeader header{ nextSequence, PacketType::REGULAR };
		WriteBasePacketHeader(packetSendBuffer, &header);
		memcpy(&packetSendBuffer[BASE_HEADER_SIZE], data, dataSize);

		// Send packet
		const int sizeSent = socket.Send(to, packetSendBuffer, packetSize);

		if (sizeSent != dataSize)
			DL_NET_LOG_ERROR("Failed to send packet of size %d", packetSize);
	}

	// TODO: Store sequence and timestamp of when this packet was sent to track RTT

	++nextSequence;

	return true;
}

void NetClient::receive_packet()
{
	// TODO: Add asserts on socket?
	// assert(from);
	// assert(dataPtr);

	Address from;
	uint8_t packetReceiveBuffer[MAX_PACKET_SIZE];
	const int packetSize = socket.Receive(&from, packetReceiveBuffer, sizeof(packetReceiveBuffer));

	if (packetSize == 0)
		return; // TODO: Come up with how to handle all of these exit cases

	const auto [sequence, packetType] = ReadBasePacketHeader(packetReceiveBuffer);

	switch (packetType)
	{
	case PacketType::REGULAR:
		HandlePacket(packetReceiveBuffer, packetSize);
		break;
	case PacketType::FRAGMENT:
		const auto [fragmentCount, fragmentIndex] = ReadFragmentPacketSubHeader(&packetReceiveBuffer[BASE_HEADER_SIZE]);
		ReceivedFragmentPacketData* fragmentPacketData = &receivedFragmentPackets[sequence % MAX_FRAGMENT_PACKET_BUFFER_SIZE];

		if ((int)fragmentCount > MAX_FRAGMENTS || fragmentCount == 0 || fragmentIndex >= fragmentCount)
		{
			// TODO: Log?
			// TODO: Metrics?
			return;
		}

		if (fragmentPacketData->sequence != sequence)
			InitNewFragmentPacketData(sequence, fragmentCount);

		if (!fragmentPacketData->fragmentReceived[fragmentIndex])
		{
			memcpy(
				&fragmentPacketData->packetData[(uint64_t)fragmentIndex * MAX_FRAGMENT_PAYLOAD_SIZE],
				&packetReceiveBuffer[BASE_HEADER_SIZE + FRAGMENT_SUB_HEADER_SIZE],
				packetSize - BASE_HEADER_SIZE - FRAGMENT_SUB_HEADER_SIZE);
			fragmentPacketData->fragmentReceived[fragmentIndex] = true;
			++fragmentPacketData->fragmentReceivedCount;

			if (fragmentIndex == fragmentCount - 1)
			{
				const int lastFragmentSize = packetSize - BASE_HEADER_SIZE - FRAGMENT_SUB_HEADER_SIZE;
				fragmentPacketData->totalSize = (fragmentCount - 1) * MAX_FRAGMENT_PAYLOAD_SIZE + lastFragmentSize;
			}
		}

		if (fragmentPacketData->fragmentReceivedCount == fragmentPacketData->fragmentCount)
			HandlePacket(fragmentPacketData->packetData, fragmentPacketData->totalSize);

		break;
	}
}

void NetClient::WriteBasePacketHeader(uint8_t* packetBuffer, const BasePacketHeader* header)
{
	assert(packetBuffer);
	assert(header);

	const uint16_t netSequence = htons(header->sequence);
	packetBuffer[0] = (netSequence >> 8) & 0xFF;
	packetBuffer[1] = netSequence & 0xFF;

	packetBuffer[2] = (uint8_t)header->packetType;
}

void NetClient::WriteFragmentPacketHeader(uint8_t* packetBuffer, const FragmentPacketSubHeader* header)
{
	assert(packetBuffer);
	assert(header);

	packetBuffer[0] = header->fragmentCount;
	packetBuffer[1] = header->fragmentIndex;
}

NetClient::BasePacketHeader NetClient::ReadBasePacketHeader(const uint8_t* packetBuffer)
{
	BasePacketHeader baseHeader;
	const auto packetType = (PacketType)packetBuffer[2];
	const uint16_t hostSequence = (uint16_t)(packetBuffer[0] << 8) | packetBuffer[1];

	baseHeader.sequence = ntohs(hostSequence);
	baseHeader.packetType = packetType;

	return baseHeader;
}

NetClient::FragmentPacketSubHeader NetClient::ReadFragmentPacketSubHeader(const uint8_t* packetBuffer)
{
	FragmentPacketSubHeader fragmentHeader;

	fragmentHeader.fragmentCount = packetBuffer[0];
	fragmentHeader.fragmentIndex = packetBuffer[1];

	return fragmentHeader;
}

void NetClient::HandlePacket(uint8_t* packet, int packetSize)
{
	// TEST
	for (uint32_t i = 0; (uint64_t)i < packetSize / sizeof(int); ++i)
		if (i != ((uint32_t*)packet)[i])
		{
			DL_NET_LOG_ERROR("Error in packet data");
			return;
		}

	net_log_error("Success!");
}

void NetClient::InitNewFragmentPacketData(uint16_t sequence, uint8_t fragmentCount)
{
	const int index = sequence % MAX_FRAGMENT_PACKET_BUFFER_SIZE;
	ReceivedFragmentPacketData* data = &receivedFragmentPackets[index];

	data->sequence = sequence;
	data->fragmentCount = fragmentCount;
	data->fragmentReceivedCount = 0;

	for (int i = 0; i < MAX_FRAGMENTS; ++i)
		data->fragmentReceived[i] = false;

	// Will be adjusted when last fragment is received as it may be smaller than MAX_FRAGMENT_PAYLOAD_SIZE
	data->totalSize = fragmentCount * MAX_FRAGMENT_PAYLOAD_SIZE;
	data->packetData = (uint8_t*)malloc(data->totalSize);
}
}
