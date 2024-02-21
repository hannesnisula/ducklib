#include "VulkanSwapChain.h"
#include "VulkanCommon.h"
#include "VulkanSemaphore.h"

namespace ducklib::Render
{
VulkanSwapChain::~VulkanSwapChain()
{
	for (uint32 i = 0; i < numBuffers; ++i)
		vkDestroyImageView(vkDevice, (VkImageView)buffers[i].apiDescriptor, nullptr);

	vkDestroySemaphore(vkDevice, vkImageAvailableSemaphore, nullptr);
}

void* VulkanSwapChain::GetImageAvailabilitySemaphore()
{
	return vkImageAvailableSemaphore;
}

// TODO: Add semaphore for finished rendering as argument, to decouple swap chain (properly, if possible in both APIs)
void VulkanSwapChain::Present()
{
	VkPresentInfoKHR presentInfo{};
	const VkSemaphore vkWaitSemaphore = ((VulkanSemaphore*)vkFrameRenderFinishSemaphore)->vkSemaphore;

	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = & vkWaitSemaphore;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &vkSwapChain;
	presentInfo.pImageIndices = &currentFrameIndex;

	DL_VK_CHECK(vkQueuePresentKHR(vkPresentQueue, &presentInfo), "Failed to present Vulkan swap chain image");
}

void VulkanSwapChain::PrepareFrame()
{
	vkAcquireNextImageKHR(vkDevice, vkSwapChain, UINT64_MAX, vkImageAvailableSemaphore, VK_NULL_HANDLE, &currentFrameIndex);
}

VulkanSwapChain::VulkanSwapChain(
	uint32 width,
	uint32 height,
	Format format,
	VkSwapchainKHR vkSwapChain,
	uint32 bufferCount,
	const ImageBuffer* images,
	VkDevice vkDevice,
	VkQueue vkPresentQueue,
	ISemaphore* frameRenderFinishedSemaphore)
	: ISwapChain()
{
	this->width = width;
	this->height = height;
	this->format = format;
	this->vkSwapChain = vkSwapChain;
	this->numBuffers = bufferCount;
	this->vkDevice = vkDevice;
	this->vkPresentQueue = vkPresentQueue;
	this->vkFrameRenderFinishSemaphore = frameRenderFinishedSemaphore;

	for (uint32 i = 0; i < bufferCount; ++i)
		this->buffers[i] = images[i];

	// Create sync primites
	VkSemaphoreCreateInfo semaphoreCreateInfo{};

	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	DL_VK_CHECK(
		vkCreateSemaphore(vkDevice, &semaphoreCreateInfo, nullptr, &vkImageAvailableSemaphore),
		"Failed to create Vulkan semaphore");
}
}
