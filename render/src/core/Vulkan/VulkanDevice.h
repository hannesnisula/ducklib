#pragma once
#include "VulkanSemaphore.h"
#include "Lib/vulkan.h"
#include "../IDevice.h"
#include "Core/Memory/IAllocator.h"
#include "Core/Memory/Containers/TArray.h"

namespace ducklib::Render
{
class VulkanDevice : IDevice
{
public:
	friend class VulkanAdapter;

	~VulkanDevice() override;

	ICommandBuffer* CreateCommandBuffer() override;

	IPass* CreatePass(const PassDescription& passDesc) override;
	IFrameBuffer* CreateFrameBuffer(ImageBuffer** imageBuffers, uint32 imageBufferCount, IPass* pass) override;

	void DestroyCommandBuffer(ICommandBuffer* commandBuffer) override;

	void ExecuteCommandBuffers(
		ICommandBuffer** commandBuffers,
		uint32_t commandBufferCount,
		void* waitSemaphore,
		IFence*signalFence) override;

	ISwapChain* CreateSwapChain(uint32_t width, uint32_t height, Format format, uint32_t bufferCount,HWND windowHandle) override;
	void DestroySwapChain(ISwapChain* swapChain) override;

	IFence* CreateFence() override;

protected:
	VulkanDevice(
		VkDevice vkDevice,
		VkQueue commandQueue,
		uint32 graphicsQueueFamilyIndex,
		VkPhysicalDevice physicalDevice,
		VkInstance vkInstance);

	VkSurfaceKHR CreateWindowSurface(HWND windowHandle);

	struct SwapChainSupport
	{
		TArray<VkSurfaceFormatKHR> surfaceFormats;
		TArray<VkPresentModeKHR> presentModes;
		VkSurfaceCapabilitiesKHR surfaceCapabilities;
	};

	SwapChainSupport QuerySwapChainSupport(VkSurfaceKHR surface);
	uint32 SelectSurfaceFormatIndex(const TArray<VkSurfaceFormatKHR>& surfaceFormats, VkFormat desiredFormat);
	uint32 SelectPresentModeIndex(const TArray<VkPresentModeKHR>& presentModes);
	VkExtent2D GetSurfaceExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities, uint32 width, uint32 height) const;

	IAllocator* alloc;

	VkInstance vkInstance;
	VkPhysicalDevice physicalDevice;
	VkDevice vkDevice;
	// Currently assumed to also be the present queue. Unsure if it is even being tested.
	// TODO: Make sure this also is the present queue or add a possible other queue for presentation.
	VkCommandPool vkCommandPool;
	VkQueue vkCommandQueue;
	VulkanSemaphore* commandQueueFinishedSemaphore; // TODO: Change back to VkSemaphore? Sync primitive triage

	uint32 graphicsQueueFamilyIndex;
};
}
