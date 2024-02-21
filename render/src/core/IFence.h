#pragma once

namespace ducklib::Render
{
class IFence
{
public:
	virtual ~IFence() {}

	virtual void Wait() = 0;
};
}
