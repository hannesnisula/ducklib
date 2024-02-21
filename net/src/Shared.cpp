#include <winsock2.h>
#include <string>
#include <WS2tcpip.h>
#include <cassert>

#include "net/Shared.h"

#include <stdexcept>


namespace ducklib
{
Address::Address(const std::string& address, uint16_t port)
{
    this->port = port;
    IN_ADDR inAddr;

    if (inet_pton(AF_INET, address.data(), &inAddr) != 1)
        throw std::runtime_error("Failed to parse address");

    addrV4 = inAddr.S_un.S_addr;
}

Address::Address(const sockaddr_in& sockAddr)
{
    addrV4 = sockAddr.sin_addr.s_addr;
    port = ntohs(sockAddr.sin_port);
}

auto Address::as_sockaddr_in() const -> sockaddr_in
{
    assert(addrV4 != 0);
    assert(port != 0);

    sockaddr_in sockAddr{};

    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(port);
    sockAddr.sin_addr.s_addr = addrV4;

    return sockAddr;
}
}
