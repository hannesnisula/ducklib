#pragma once
#include <cstdint>

namespace ducklib
{
class Address
{
public:
	Address() = default;
	Address(const std::string& address, uint16_t port);
	explicit Address(const sockaddr_in& sockAddr);

	[[nodiscard]] auto get_port() const -> uint16_t;
	[[nodiscard]] auto as_sockaddr_in() const -> sockaddr_in; // TODO: Consider moving these to NetClient

private:
	uint32_t addrV4;
	uint16_t port;
};

inline auto Address::get_port() const -> uint16_t { return port; }
}
