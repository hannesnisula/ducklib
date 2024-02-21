#include "Core/Memory/IAllocator.h"
#include <cstring>
#include "IAdapter.h"

namespace ducklib::Render
{
IAdapter::~IAdapter() = default;

uint32_t IAdapter::GetDeviceId() const
{
	return deviceId;
}

const char* IAdapter::GetDescription() const
{
	return description;
}

bool IAdapter::IsHardware() const
{
	return isHardware;
}

IAdapter::IAdapter(const char* description, bool isHardware)
{
	this->isHardware = isHardware;

	if (description)
	{
		uint32 descriptionLength = (uint32)strlen(description) + 1;

		this->description = DefAlloc()->Allocate<char>(descriptionLength + 1);
		strcpy_s(this->description, descriptionLength, description);
	}
}
}
