#include "VulkanRHI.h"
#include "Lib/glfw3.h"
#include <stdexcept>
#include "VulkanAdapter.h"

namespace ducklib::Render
{
VulkanRHI::~VulkanRHI() {}

const TArray<IAdapter*>& VulkanRHI::GetAdapters() const
{
	return adapters;
}

IRHI* VulkanRHI::GetInstance()
{
	static VulkanRHI rhi;

	return &rhi;
}

VulkanRHI::VulkanRHI()
	: alloc(nullptr)
	, instance(nullptr)
{
	alloc = DefAlloc();

	CreateInstance();
	EnumerateAdapters();
}

void VulkanRHI::CreateInstance()
{
	VkApplicationInfo appInfo{};
	VkInstanceCreateInfo createInfo{};
	TArray<const char*> extensions = GetInstanceExtensions();

	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.apiVersion = VK_API_VERSION_1_3;

	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = extensions.Length();
	createInfo.ppEnabledExtensionNames = extensions.Data();
#if _DEBUG
	constexpr const char* const validationLayers[] = {
		"VK_LAYER_KHRONOS_validation"
	};
	createInfo.enabledLayerCount = 1;
	createInfo.ppEnabledLayerNames = validationLayers;
#else
	createInfo.enabledLayerCount = 0;
#endif

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
		throw std::runtime_error("Failed to create Vulkan instance");
}

void VulkanRHI::EnumerateAdapters()
{
	uint32_t deviceCount;

	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0)
		throw std::runtime_error("No Vulkan compatible adapters found");

	VkPhysicalDevice* devices = alloc->Allocate<VkPhysicalDevice>(deviceCount);

	vkEnumeratePhysicalDevices(instance, &deviceCount, devices);

	for (uint32_t i = 0; i < deviceCount; ++i)
	{
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;

		vkGetPhysicalDeviceProperties(devices[i], &deviceProperties);
		vkGetPhysicalDeviceFeatures(devices[i], &deviceFeatures);

		if (IsPhysicalDeviceUsable(deviceProperties, deviceFeatures))
		{
			VulkanAdapter* adapter = alloc->Allocate<VulkanAdapter>();

			new(adapter) VulkanAdapter(
				deviceProperties.deviceName,
				deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
				devices[i],
				instance);

			adapters.Append(adapter);
		}
	}

	alloc->Free(devices);
}

bool VulkanRHI::IsPhysicalDeviceUsable(
	const VkPhysicalDeviceProperties& deviceProperties,
	const VkPhysicalDeviceFeatures& deviceFeatures)
{
	return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
}

TArray<const char*> VulkanRHI::GetInstanceExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	TArray extensions(glfwExtensions, glfwExtensionCount, glfwExtensionCount + 2);

	extensions.Append("VK_KHR_surface");
	extensions.Append("VK_KHR_win32_surface");

	return extensions;
}
}
