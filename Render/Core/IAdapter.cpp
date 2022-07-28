#include "Core/Memory/IAlloc.h"
#include <cstring>
#include "IAdapter.h"

namespace DuckLib::Render
{
IAdapter::~IAdapter() { }

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
		uint32_t descriptionLength = (uint32_t)strlen(description) + 1;

		this->description = DefAlloc()->Allocate<char>(descriptionLength + 1);
		strcpy_s(this->description, descriptionLength, description);
	}
}
}
