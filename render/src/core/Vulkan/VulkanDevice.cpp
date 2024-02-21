#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"
#include "VulkanCommon.h"
#include "VulkanFrameBuffer.h"
#include "VulkanPass.h"
#include "VulkanSwapChain.h"
#include "Core/Memory/Containers/TArray.h"
#include "Lib/vulkan_win32.h"
#include "Lib/glfw3.h"

namespace ducklib::Render
{
VulkanDevice::~VulkanDevice()
{
	vkDestroyDevice(vkDevice, nullptr);
	vkDestroyCommandPool(vkDevice, vkCommandPool, nullptr);

	commandQueueFinishedSemaphore->~VulkanSemaphore();
	alloc->Free(commandQueueFinishedSemaphore);
}

ICommandBuffer* VulkanDevice::CreateCommandBuffer()
{
	VkCommandBuffer vkCommandBuffer;
	VkCommandBufferAllocateInfo allocInfo{};

	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = vkCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	DL_VK_CHECK(vkAllocateCommandBuffers(vkDevice, &allocInfo, &vkCommandBuffer), "Failed to create Vulkan command buffer");

	VulkanCommandBuffer* cmdBuffer = alloc->Allocate<VulkanCommandBuffer>();
	new(cmdBuffer) VulkanCommandBuffer(vkCommandBuffer);

	return cmdBuffer;
}

IPass* VulkanDevice::CreatePass(const PassDescription& passDesc)
{
	// Setup attachments
	TArray<VkAttachmentDescription> vkAttachmentDescs(nullptr, passDesc.frameBufferDescCount);

	for (uint32 i = 0; i < passDesc.frameBufferDescCount; ++i)
	{
		vkAttachmentDescs[i] = {};
		vkAttachmentDescs[i].loadOp = MapToVulkanLoadOp(passDesc.frameBufferDescs[i].loadOp);
		vkAttachmentDescs[i].storeOp = MapToVulkanStoreOp(passDesc.frameBufferDescs[i].storeOp);
		vkAttachmentDescs[i].stencilStoreOp = MapToVulkanStoreOp(passDesc.frameBufferDescs[i].stencilStoreOp);
		vkAttachmentDescs[i].stencilLoadOp = MapToVulkanLoadOp(passDesc.frameBufferDescs[i].stencilLoadOp);
		vkAttachmentDescs[i].initialLayout = MapToVulkanImageLayout(passDesc.frameBufferDescs[i].initialLayout);
		vkAttachmentDescs[i].finalLayout = MapToVulkanImageLayout(passDesc.frameBufferDescs[i].finalLayout);
		vkAttachmentDescs[i].format = MapToVulkanFormat(passDesc.frameBufferDescs[i].format);
		vkAttachmentDescs[i].samples = VK_SAMPLE_COUNT_1_BIT;
	}

	// Setup subpasses
	uint32 totalAttachmentRefCount = 0;
	uint32 attachmentRefOffset = 0;

	for (uint32 i = 0; i < passDesc.subPassDescCount; ++i)
		totalAttachmentRefCount += passDesc.subPassDescs[i].frameBufferDescRefCount;

	TArray<VkAttachmentReference> vkAttachmentRefs(totalAttachmentRefCount);
	TArray<VkAttachmentReference*> vkAttachmentRefPtrs(passDesc.subPassDescCount);
	TArray<VkSubpassDescription> vkSubPassDescs(nullptr, passDesc.subPassDescCount);

	for (uint32 i = 0; i < passDesc.subPassDescCount; ++i)
	{
		uint32 attachmentRefCount = passDesc.subPassDescs[i].frameBufferDescRefCount;

		for (uint32 u = 0; u < attachmentRefCount; ++u)
		{
			vkAttachmentRefs[attachmentRefOffset + u].attachment = passDesc.subPassDescs[i].frameBufferDescRefs[u].frameBufferDescIndex;
			vkAttachmentRefs[attachmentRefOffset + u].layout = MapToVulkanImageLayout(
				passDesc.subPassDescs[i].frameBufferDescRefs[u].imageBufferLayout);
		}

		vkSubPassDescs[i] = {};
		vkSubPassDescs[i].pipelineBindPoint = MapToVulkanBindPoint(passDesc.subPassDescs[i].pipelineBindPoint);
		vkSubPassDescs[i].colorAttachmentCount = attachmentRefCount;
		vkSubPassDescs[i].pColorAttachments = vkAttachmentRefs.Data();
		// TODO: Add the other members, too

		attachmentRefOffset += attachmentRefCount;
	}

	// Setup the pass
	VkRenderPassCreateInfo passCreateInfo{};
	VkRenderPass vkPass;

	passCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	passCreateInfo.attachmentCount = vkAttachmentDescs.Length();
	passCreateInfo.pAttachments = vkAttachmentDescs.Data();
	passCreateInfo.subpassCount = passDesc.subPassDescCount;
	passCreateInfo.pSubpasses = vkSubPassDescs.Data();

	DL_VK_CHECK(vkCreateRenderPass(vkDevice, &passCreateInfo, nullptr, &vkPass), "Failed to create Vulkan render pass");

	// Cleanup of Vulkan descs
	for (uint32 i = 0; i < vkAttachmentRefPtrs.Length(); ++i)
		alloc->Free(vkAttachmentRefPtrs[i]);

	// Create VulkanPass
	VulkanPass* pass = alloc->Allocate<VulkanPass>();

	new(pass) VulkanPass(vkPass, vkDevice);

	return pass;
}

IFrameBuffer* VulkanDevice::CreateFrameBuffer(ImageBuffer** imageBuffers, uint32 imageBufferCount, IPass* pass)
{
	VkFramebuffer vkFrameBuffer;
	VkFramebufferCreateInfo frameBufferCreateInfo{};
	TArray<VkImageView> vkFrameBufferImageViews(nullptr, imageBufferCount);

	// TODO: Loop images
	for (uint32 i = 0; i < imageBufferCount; ++i)
		vkFrameBufferImageViews[i] = (VkImageView)imageBuffers[i]->apiDescriptor;

	frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	frameBufferCreateInfo.renderPass = ((VulkanPass*)pass)->vkPass;
	frameBufferCreateInfo.pAttachments = vkFrameBufferImageViews.Data();
	frameBufferCreateInfo.attachmentCount = vkFrameBufferImageViews.Length();
	frameBufferCreateInfo.layers = 1;
	frameBufferCreateInfo.width = imageBuffers[0]->width;
	frameBufferCreateInfo.height = imageBuffers[0]->height;

	DL_VK_CHECK(vkCreateFramebuffer(vkDevice, &frameBufferCreateInfo, nullptr, &vkFrameBuffer), "Failed to create Vulkan frame buffer");

	VulkanFrameBuffer* frameBuffer = alloc->Allocate<VulkanFrameBuffer>();

	new(frameBuffer) VulkanFrameBuffer(vkFrameBuffer, VkExtent2D{ imageBuffers[0]->width, imageBuffers[0]->height });

	return frameBuffer;
}

void VulkanDevice::DestroyCommandBuffer(ICommandBuffer* commandBuffer)
{
	// TODO: Implement
}

void VulkanDevice::ExecuteCommandBuffers(
	ICommandBuffer** commandBuffers,
	uint32_t commandBufferCount,
	void* waitSemaphore,
	IFence* signalFence)
{
	TArray<VkCommandBuffer> vkCommandBuffers(nullptr, commandBufferCount);
	VkSubmitInfo submitInfo{};
	VkSemaphore vkWaitSemaphore = (VkSemaphore)waitSemaphore;
	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	for (uint32 i = 0; i < commandBufferCount; ++i)
		vkCommandBuffers[i] = ((VulkanCommandBuffer*)commandBuffers[i])->vkCommandBuffer;

	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = commandBufferCount;
	submitInfo.pCommandBuffers = vkCommandBuffers.Data();
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &vkWaitSemaphore;
	submitInfo.pWaitDstStageMask = &waitStage;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &commandQueueFinishedSemaphore->vkSemaphore;

	DL_VK_CHECK(
		vkQueueSubmit(vkCommandQueue, 1, &submitInfo, ((VulkanFence*)signalFence)->vkFence),
		"Failed to submit command buffer to Vulkan queue");
}

ISwapChain* VulkanDevice::CreateSwapChain(uint32_t width, uint32_t height, Format format, uint32_t bufferCount, HWND windowHandle)
{
	// Create swap chain
	VkFormat vkFormat = MapToVulkanFormat(format);
	VkSurfaceKHR surface = CreateWindowSurface(windowHandle);
	SwapChainSupport swapChainSupport = QuerySwapChainSupport(surface);
	uint32 surfaceFormatIndex = SelectSurfaceFormatIndex(swapChainSupport.surfaceFormats, vkFormat);
	uint32 presentModeIndex = SelectPresentModeIndex(swapChainSupport.presentModes);
	VkExtent2D extent = GetSurfaceExtent(swapChainSupport.surfaceCapabilities, width, height);

	VkSwapchainCreateInfoKHR createInfo{};

	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = 2;
	createInfo.imageFormat = swapChainSupport.surfaceFormats[surfaceFormatIndex].format;
	createInfo.imageColorSpace = swapChainSupport.surfaceFormats[surfaceFormatIndex].colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.queueFamilyIndexCount = 0;
	createInfo.preTransform = swapChainSupport.surfaceCapabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = swapChainSupport.presentModes[presentModeIndex];
	createInfo.clipped = VK_TRUE;

	VkSwapchainKHR vkSwapChain;

	DL_VK_CHECK(
		vkCreateSwapchainKHR(vkDevice, &createInfo, nullptr, &vkSwapChain),
		"Failed to create Vulkan swap chain");

	// Get image buffers
	uint32 imageCount = 0;
	TArray<VkImage> vkImages;

	DL_VK_CHECK(vkGetSwapchainImagesKHR(vkDevice, vkSwapChain, &imageCount, nullptr), "Failed to get Vulkan swapchain image count");

	if (imageCount > ISwapChain::MAX_BUFFERS)
		throw std::runtime_error("Vulkan swap chain contained too many images");

	vkImages.Resize(imageCount);
	DL_VK_CHECK(vkGetSwapchainImagesKHR(vkDevice, vkSwapChain, &imageCount, vkImages.Data()), "Failed to get Vulkan swapchain images");

	ImageBuffer imageBuffers[ISwapChain::MAX_BUFFERS];

	for (uint32 i = 0; i < imageCount; ++i)
	{
		VkImageView vkImageView;
		VkImageViewCreateInfo createInfo{};

		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.format = vkFormat;
		createInfo.image = vkImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		DL_VK_CHECK(vkCreateImageView(vkDevice, &createInfo, nullptr, &vkImageView), "Failed to create Vulkan image view for swap chain");

		imageBuffers[i].width = width;
		imageBuffers[i].height = height;
		imageBuffers[i].format = format;
		imageBuffers[i].apiResource = vkImages[i];
		imageBuffers[i].apiDescriptor = vkImageView;
	}

	// Create swap chain wrapper
	VulkanSwapChain* swapChain = alloc->Allocate<VulkanSwapChain>();

	new(swapChain) VulkanSwapChain(width,
		height,
		format,
		vkSwapChain,
		imageCount,
		imageBuffers,
		vkDevice,
		vkCommandQueue,
		commandQueueFinishedSemaphore);

	return swapChain;
}

void VulkanDevice::DestroySwapChain(ISwapChain* swapChain) {}

IFence* VulkanDevice::CreateFence()
{
	VkFence vkFence;
	VkFenceCreateInfo fenceCreateInfo{};

	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	DL_VK_CHECK(
		vkCreateFence(vkDevice, &fenceCreateInfo, nullptr, &vkFence),
		"Failed to create Vulkan fence for swap chain");

	VulkanFence* fence = alloc->Allocate<VulkanFence>();
	return new(fence) VulkanFence(vkDevice, vkFence);
}

VulkanDevice::VulkanDevice(
	VkDevice vkDevice,
	VkQueue commandQueue,
	uint32 graphicsQueueFamilyIndex,
	VkPhysicalDevice physicalDevice,
	VkInstance vkInstance)
	: alloc(DefAlloc())
	, vkInstance(vkInstance)
	, physicalDevice(physicalDevice)
	, vkDevice(vkDevice)
	, vkCommandQueue(commandQueue)
	, graphicsQueueFamilyIndex(graphicsQueueFamilyIndex)
{
	// TODO: Maybe be consistent on where things are created?

	VkCommandPoolCreateInfo cmdPoolCreateInfo{};

	cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
	cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	DL_VK_CHECK(vkCreateCommandPool(vkDevice, &cmdPoolCreateInfo, nullptr, &vkCommandPool), "Failed to create Vulkan command pool");

	VkSemaphore vkCommandQueueFinishedSemaphore;
	VkSemaphoreCreateInfo semaphoreCreateInfo{};

	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	DL_VK_CHECK(
		vkCreateSemaphore(vkDevice, &semaphoreCreateInfo, nullptr, &vkCommandQueueFinishedSemaphore),
		"Failed to create Vulkan command-queue-finished semaphore");

	commandQueueFinishedSemaphore = alloc->New<VulkanSemaphore>(vkCommandQueueFinishedSemaphore, vkDevice);
}

VkSurfaceKHR VulkanDevice::CreateWindowSurface(HWND windowHandle)
{
	VkSurfaceKHR surface;
	VkWin32SurfaceCreateInfoKHR createInfo{};
	HINSTANCE instance = GetModuleHandle(NULL);

	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hinstance = instance;
	createInfo.hwnd = windowHandle;

	DL_VK_CHECK(vkCreateWin32SurfaceKHR(vkInstance, &createInfo, nullptr, &surface), "Failed to create Vulkan window surface");

	return surface;
}

VulkanDevice::SwapChainSupport VulkanDevice::QuerySwapChainSupport(VkSurfaceKHR surface)
{
	SwapChainSupport support;
	uint32_t surfaceFormatCount;
	uint32_t presentModeCount;

	DL_VK_CHECK(
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr),
		"Failed to get Vulkan adapter surface format count");
	support.surfaceFormats.Resize(surfaceFormatCount);
	DL_VK_CHECK(
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, support.surfaceFormats.Data()),
		"Failed to get Vulkan adapter surface formats");

	DL_VK_CHECK(
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr),
		"Failed to get Vulkan adapter surface present mode count");
	support.presentModes.Resize(presentModeCount);
	DL_VK_CHECK(
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, support.presentModes.Data()),
		"Failed to get Vulkan adapter surface present modes");

	DL_VK_CHECK(
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &support.surfaceCapabilities),
		"Failed to get Vulkan adapter surface capabilities");

	return support;
}

uint32 VulkanDevice::SelectSurfaceFormatIndex(const TArray<VkSurfaceFormatKHR>& surfaceFormats, VkFormat desiredFormat)
{
	for (uint32 i = 0; i < surfaceFormats.Length(); ++i)
		if (surfaceFormats[i].format == desiredFormat)
			return i;

	throw std::runtime_error("Failed to select Vulkan surface format");
}

// Currently hardcoded to select immediate mode (no v-sync)
uint32 VulkanDevice::SelectPresentModeIndex(const TArray<VkPresentModeKHR>& presentModes)
{
	for (uint32 i = 0; i < presentModes.Length(); ++i)
		if (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
			return i;

	throw std::runtime_error("Failed to find Vulkan immediate present mode");
}

VkExtent2D VulkanDevice::GetSurfaceExtent(
	const VkSurfaceCapabilitiesKHR& surfaceCapabilities,
	uint32 width,
	uint32 height) const
{
	if (width >= surfaceCapabilities.minImageExtent.width
		&& width <= surfaceCapabilities.maxImageExtent.width
		&& height >= surfaceCapabilities.minImageExtent.height
		&& height <= surfaceCapabilities.maxImageExtent.height)
		return surfaceCapabilities.currentExtent;

	throw std::runtime_error("Failed to validate swap chain resolution with Vulkan surface");
}
}
