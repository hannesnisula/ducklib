#pragma once
#include "Lib/vulkan.h"
#include "Render/Core/IFrameBuffer.h"

namespace ducklib::Render
{
class VulkanFrameBuffer : public IFrameBuffer
{
protected:
	friend class VulkanDevice;
	friend class VulkanCommandBuffer;

	VulkanFrameBuffer(VkFramebuffer vkFrameBuffer, VkExtent2D vkExtent)
		: vkFrameBuffer(vkFrameBuffer), vkExtent(vkExtent) { }

	VkFramebuffer vkFrameBuffer;
	VkExtent2D vkExtent;
};
}
