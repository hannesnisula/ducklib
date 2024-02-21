#include "VulkanSemaphore.h"

namespace ducklib::Render
{
VulkanSemaphore::VulkanSemaphore(VkSemaphore vkSemaphore, VkDevice vkDevice)
	: vkSemaphore(vkSemaphore)
	, vkDevice(vkDevice) {}

VulkanSemaphore::~VulkanSemaphore()
{
	vkDestroySemaphore(vkDevice, vkSemaphore, nullptr);
}
}
