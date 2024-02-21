#pragma once

#include "../../Core/Types.h"
#include "IDevice.h"

namespace ducklib::Render
{
class IAdapter
{
public:
	virtual ~IAdapter();

	// TODO: Perhaps object to change
	virtual void* GetApiHandle() const = 0;

	uint32_t GetDeviceId() const;
	const char* GetDescription() const;
	bool IsHardware() const;

	virtual IDevice* CreateDevice() = 0;

protected:
	IAdapter(const char* description, bool isHardware);

	char* description;
	uint32 deviceId;
	bool isHardware;
};
}
