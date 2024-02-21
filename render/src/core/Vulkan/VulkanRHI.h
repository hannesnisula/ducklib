#pragma once

#include "Lib/vulkan.h"
#include "../IRHI.h"
#include "Core/Memory/Containers/TArray.h"

namespace ducklib::Render
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
	bool IsPhysicalDeviceUsable(
		const VkPhysicalDeviceProperties& deviceProperties,
		const VkPhysicalDeviceFeatures& deviceFeatures);
	TArray<const char*> GetInstanceExtensions();

	static constexpr uint32_t DEVICE_DESCRIPTION_BUFFER_SIZE = 256;

	IAllocator* alloc;

	VkInstance instance;

	TArray<IAdapter*> adapters;
};
}
