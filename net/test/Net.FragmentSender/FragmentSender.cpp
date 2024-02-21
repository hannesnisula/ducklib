#include "../../Net.h"
#include <iostream>
#include "../../NetClient.h"
#include "../../Shared.h"

constexpr int TEST_PACKET_SIZE = 1600;
constexpr int TEST_PACKET_INT_COUNT = TEST_PACKET_SIZE / sizeof(int);

int main()
{
    ducklib::InitializeNet();

    char ipAddress[32];

    std::cout << "Enter address to send fragmented packets to: ";
    std::cin >> ipAddress;

    ducklib::Address destAddress(ipAddress);
    ducklib::NetClient netClient;
    char message[1042];
    uint8_t testPacket[1600];

    for (int i = 0; i < TEST_PACKET_INT_COUNT; ++i)
        ((int*)&testPacket)[i] = i;

    while (true)
    {
        message[0] = '\0';

        std::cout << "Enter \"send\" to send fragmented packet (1600 bytes) and enter \"quit\" to exit.\n";
        std::cin >> message;

        if (strcmp(message, "send") == 0)
        {
            netClient.SendPacket(&destAddress, testPacket, sizeof testPacket);
        }
        else if (strcmp(message, "quit") == 0)
            break;
    }

    ducklib::ShutdownNet();
    
    return 0;
}
