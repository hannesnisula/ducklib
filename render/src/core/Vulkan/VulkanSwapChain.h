#pragma once
#include "VulkanFence.h"
#include "Lib/vulkan.h"
#include "Render/Core/ISwapChain.h"

namespace ducklib::Render
{
class VulkanSwapChain : public ISwapChain
{
public:
	friend class VulkanDevice;

	~VulkanSwapChain() override;

	void* GetImageAvailabilitySemaphore() override;

	void Present() override;
	void PrepareFrame() override;

protected:
	VulkanSwapChain(
		uint32 width,
		uint32 height,
		Format format,
		VkSwapchainKHR vkSwapChain,
		uint32 bufferCount,
		const ImageBuffer* images,
		VkDevice vkDevice,
		VkQueue vkPresentQueue,
		ISemaphore* frameRenderFinishedSemaphore);

	VkSwapchainKHR vkSwapChain;
	VkDevice vkDevice;
	VkQueue vkPresentQueue;
	VkSemaphore vkImageAvailableSemaphore;
	ISemaphore* vkFrameRenderFinishSemaphore;
};
}
