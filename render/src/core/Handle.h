#pragma once
#include <cstdint>

namespace ducklib::Render
{
struct Handle
{
	uint32 type : 6;
	uint32 index : 25;
	// TODO: Consider adding a generation value to detect outdated handles being reused
};
}
