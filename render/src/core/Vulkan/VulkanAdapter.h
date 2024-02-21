#pragma once

#include "Lib/vulkan.h"
#include "Core/Memory/IAllocator.h"
#include "Render/Core/IAdapter.h"

namespace ducklib::Render
{
class VulkanAdapter : public IAdapter
{
public:

	friend class VulkanRHI;

	void* GetApiHandle() const override;
	IDevice* CreateDevice() override;

private:
	VulkanAdapter(const char* description, bool isHardware, VkPhysicalDevice apiAdapter, VkInstance vkInstance);
	~VulkanAdapter() override;

	uint32_t GetGraphicsQueueFamilyIndex();

	IAllocator* alloc;

	VkInstance vkInstance;
	VkPhysicalDevice physicalDevice;
};
}
