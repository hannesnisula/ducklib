#include "VulkanFence.h"
#include "VulkanCommon.h"

namespace ducklib::Render
{
VulkanFence::~VulkanFence()
{
	vkDestroyFence(vkDevice, vkFence, nullptr);
}

void VulkanFence::Wait()
{
	DL_VK_CHECK(
		vkWaitForFences(vkDevice, 1, &vkFence, VK_TRUE, UINT64_MAX),
		"Failed to wait for Vulkan render fence when waiting for frame");

	DL_VK_CHECK(vkResetFences(vkDevice, 1, &vkFence), "Failed to reset Vulkan render fence when waiting for frame");
}

VulkanFence::VulkanFence(VkDevice vkDevice, VkFence vkFence)
	: vkDevice(vkDevice), vkFence(vkFence) {}
}
