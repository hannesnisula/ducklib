#pragma once
#include <cstdint>
#include "Format.h"

namespace DuckLib
{
namespace Render
{
struct Buffer
{
	uint32_t size;
	Format format;
	void* apiResources;
	void* apiDescriptor;
};
}
}
