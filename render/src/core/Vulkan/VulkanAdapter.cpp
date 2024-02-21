#include "VulkanAdapter.h"

#include "VulkanDevice.h"
#include "Core/Memory/Containers/TArray.h"

namespace ducklib::Render
{
void* VulkanAdapter::GetApiHandle() const
{
	return physicalDevice;
}

IDevice* VulkanAdapter::CreateDevice()
{
	constexpr const char* const enabledExtensions[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	uint32_t graphicsQueueFamilyIndex = GetGraphicsQueueFamilyIndex();
	float dummyQueuePriority = 1.0f;
	VkDeviceQueueCreateInfo queueCreationInfo{};
	VkDeviceCreateInfo creationInfo{};
	VkPhysicalDeviceFeatures physicalDeviceFeatures{};
	VkDevice vkDevice;

	queueCreationInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreationInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
	queueCreationInfo.queueCount = 1;
	queueCreationInfo.pQueuePriorities = &dummyQueuePriority;

	creationInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	creationInfo.pQueueCreateInfos = &queueCreationInfo;
	creationInfo.queueCreateInfoCount = 1;
	creationInfo.pEnabledFeatures = &physicalDeviceFeatures;
	creationInfo.enabledExtensionCount = 1;
	creationInfo.ppEnabledExtensionNames = enabledExtensions;
	creationInfo.enabledLayerCount = 0;

	// TODO: Implement validation layer stuff here

	if (vkCreateDevice(physicalDevice, &creationInfo, nullptr, &vkDevice) != VK_SUCCESS)
		throw std::runtime_error("Failed to create Vulkan device");

	VkQueue commandQueue;
	VulkanDevice* device = alloc->Allocate<VulkanDevice>();

	vkGetDeviceQueue(vkDevice, graphicsQueueFamilyIndex, 0, &commandQueue);

	new(device) VulkanDevice(vkDevice, commandQueue, graphicsQueueFamilyIndex, physicalDevice, vkInstance);

	return device;
}

VulkanAdapter::VulkanAdapter(
	const char* description,
	bool isHardware,
	VkPhysicalDevice apiAdapter,
	VkInstance vkInstance)
	: IAdapter(description, isHardware)
	, alloc(DefAlloc())
	, vkInstance(vkInstance)
	, physicalDevice(apiAdapter) {}

VulkanAdapter::~VulkanAdapter() {}

uint32_t VulkanAdapter::GetGraphicsQueueFamilyIndex()
{
	uint32_t familyCount;

	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, nullptr);

	if (familyCount == 0)
		throw std::runtime_error("No Vulkan queues found");

	TArray<VkQueueFamilyProperties> families(familyCount);

	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, families.Data());

	for (uint32_t i = 0; i < familyCount; ++i)
	{
		// VkSurfaceKHR dummySurface{};
		VkBool32 hasPresent = false;
		// TODO: Check this on creation of swap chain instead? Just for validation
		// vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, dummySurface, &hasPresent);

		bool hasGraphics = families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT;
		bool hasCompute = families[i].queueFlags & VK_QUEUE_COMPUTE_BIT;
		bool hasTransfer = families[i].queueFlags & VK_QUEUE_TRANSFER_BIT;
		bool hasSparseBinding = families[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT;
		bool hasProtected = families[i].queueFlags & VK_QUEUE_PROTECTED_BIT;

		if (hasGraphics && hasTransfer)
			return i;
	}

	throw std::runtime_error("No Vulkan graphics queue family found");
}
}
