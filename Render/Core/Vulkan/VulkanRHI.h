#pragma once

#include "Lib/vulkan.h"
#include "../IRHI.h"
#include "Core/Memory/Containers/TArray.h"

namespace DuckLib::Render
{
class VulkanRHI : IRHI
{
public:
	static IRHI* GetInstance();

	~VulkanRHI() override;

	const TArray<IAdapter*>& GetAdapters() const override;

protected:
	VulkanRHI();

	void CreateInstance();
	void EnumerateAdapters();
	bool PhysicalDeviceIsUsable(
		const VkPhysicalDeviceProperties& deviceProperties,
		const VkPhysicalDeviceFeatures& deviceFeatures);
	TArray<const char*> GetInstanceExtensions();

	static constexpr uint32_t DEVICE_DESCRIPTION_BUFFER_SIZE = 256;

	IAlloc* alloc;

	VkInstance instance;

	TArray<IAdapter*> adapters;
};
}
