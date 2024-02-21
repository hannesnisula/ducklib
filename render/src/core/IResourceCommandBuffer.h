#pragma once
#include "Resources/Buffer.h"

namespace ducklib::Render
{
class IResourceCommandBuffer
{
public:
	virtual ~IResourceCommandBuffer() {}

	virtual Buffer* CreateBuffer() = 0;

private:
};
}
