#pragma once
#include "Lib/vulkan.h"
#include "Render/Core/ISemaphore.h"

namespace ducklib::Render
{
class VulkanSemaphore : public ISemaphore
{
	friend class VulkanSwapChain;
	friend class VulkanDevice;

public:
	VulkanSemaphore(VkSemaphore vkSemaphore, VkDevice vkDevice);
	~VulkanSemaphore() override;

protected:
	VkSemaphore vkSemaphore;
	VkDevice vkDevice;
};
}
