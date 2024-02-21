#pragma once
#include "Lib/vulkan.h"
#include "Render/Core/IFence.h"

namespace ducklib::Render
{
class VulkanFence : public IFence
{
public:
	~VulkanFence() override;

	void Wait() override;

protected:
	friend class VulkanDevice;

	VulkanFence(VkDevice vkDevice, VkFence vkFence);

	VkDevice vkDevice;
	VkFence vkFence;
};
}
