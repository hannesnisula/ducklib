#pragma once

#include <stdexcept>
#include "Render/Core/PassDescription.h"

namespace ducklib::Render
{
#define DL_VK_CHECK(statement, errorText) \
	if ((statement) != VK_SUCCESS) \
		throw std::runtime_error((errorText))

inline VkAttachmentLoadOp MapToVulkanLoadOp(FrameBufferLoadOp loadOp)
{
	switch (loadOp)
	{
	case FrameBufferLoadOp::LOAD: return VK_ATTACHMENT_LOAD_OP_LOAD;
	case FrameBufferLoadOp::CLEAR: return VK_ATTACHMENT_LOAD_OP_CLEAR;
	case FrameBufferLoadOp::DONT_CARE: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	default: throw std::runtime_error("Invalid frame buffer load op when mapping to Vulkan load op");
	}
}

inline VkAttachmentStoreOp MapToVulkanStoreOp(FrameBufferStoreOp storeOp)
{
	switch (storeOp)
	{
	case FrameBufferStoreOp::STORE: return VK_ATTACHMENT_STORE_OP_STORE;
	case FrameBufferStoreOp::DONT_CARE: return VK_ATTACHMENT_STORE_OP_DONT_CARE;
	default: throw std::runtime_error("Invalid frame buffer store op when mapping to Vulkan store op");
	}
}

inline VkImageLayout MapToVulkanImageLayout(ImageBufferLayout layout)
{
	switch (layout)
	{
	case ImageBufferLayout::UNDEFINED: return VK_IMAGE_LAYOUT_UNDEFINED;
	case ImageBufferLayout::COLOR: return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	case ImageBufferLayout::PRESENT: return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	case ImageBufferLayout::TRANSFER: return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	default: throw std::runtime_error("Invalid image buffer layout when mapping to Vulkan image buffer layout");
	}
}

inline VkPipelineBindPoint MapToVulkanBindPoint(PipelineBindPoint bindPoint)
{
	switch (bindPoint)
	{
	case PipelineBindPoint::COMPUTE: return VK_PIPELINE_BIND_POINT_COMPUTE;
	case PipelineBindPoint::GRAPHICS: return VK_PIPELINE_BIND_POINT_GRAPHICS;
	default: throw std::runtime_error("Invalid pipeline bind point when mapping to Vulkan pipeline bind point");
	}
}

// NOTE: There are several "_USCALED"/"_SSCALED" values in Vulkan that are currently not supported
static VkFormat vulkanFormatMap[] = {
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_UNDEFINED, // Typeless not existing in Vulkan
	VK_FORMAT_R32G32B32A32_SFLOAT,
	VK_FORMAT_R32G32B32A32_UINT,
	VK_FORMAT_R32G32B32A32_SINT,
	VK_FORMAT_UNDEFINED, // Typeless not existing in Vulkan
	VK_FORMAT_R32G32B32_SFLOAT,
	VK_FORMAT_R32G32B32_UINT,
	VK_FORMAT_R32G32B32_SINT,
	VK_FORMAT_UNDEFINED, // Typeless not existing in Vulkan
	VK_FORMAT_R16G16B16A16_SFLOAT,
	VK_FORMAT_R16G16B16A16_UINT,
	VK_FORMAT_R16G16B16A16_SINT,
	VK_FORMAT_R16G16B16A16_UNORM,
	VK_FORMAT_R16G16B16A16_SNORM,
	VK_FORMAT_UNDEFINED, // Typeless not existing in Vulkan
	VK_FORMAT_R32G32_SFLOAT,
	VK_FORMAT_R32G32_UINT,
	VK_FORMAT_R32G32_SINT,
	VK_FORMAT_UNDEFINED, // Typeless not existing in Vulkan
	VK_FORMAT_UNDEFINED, // Ignored because of time
	VK_FORMAT_UNDEFINED, // Ignored because of time
	VK_FORMAT_UNDEFINED, // Ignored because of time
	VK_FORMAT_UNDEFINED, // Ignored because of time
	VK_FORMAT_UNDEFINED, // Ignored because of time
	VK_FORMAT_UNDEFINED, // Ignored because of time
	VK_FORMAT_UNDEFINED, // Ignored because of time
	VK_FORMAT_UNDEFINED, // Ignored because of time
	VK_FORMAT_R8G8B8A8_UNORM,
	VK_FORMAT_UNDEFINED, // Ignored because of time
	VK_FORMAT_R8G8B8A8_UINT,
	VK_FORMAT_R8G8B8A8_SNORM,
	VK_FORMAT_R8G8B8A8_SINT,
	VK_FORMAT_UNDEFINED, // Ignored because of time
	VK_FORMAT_R16G16_SFLOAT,
	VK_FORMAT_R16G16_UNORM,
	VK_FORMAT_R16G16_UINT,
	VK_FORMAT_R16G16_SNORM,
	VK_FORMAT_R16G16_SINT,
	VK_FORMAT_UNDEFINED, // Ignored because of time
	VK_FORMAT_D32_SFLOAT,
	VK_FORMAT_R32_SFLOAT,
	VK_FORMAT_R32_UINT,
	VK_FORMAT_R32_SINT,
	VK_FORMAT_UNDEFINED, // Ignored because of time
	VK_FORMAT_D24_UNORM_S8_UINT,
	VK_FORMAT_UNDEFINED, // Ignored because of time
	VK_FORMAT_UNDEFINED, // Ignored because of time
	VK_FORMAT_UNDEFINED, // Ignored because of time
	VK_FORMAT_B8G8R8A8_UNORM,
	VK_FORMAT_B8G8R8A8_SRGB,
	VK_FORMAT_UNDEFINED, // Ignored because of time
	VK_FORMAT_UNDEFINED, // Ignored because of time
	VK_FORMAT_UNDEFINED, // Ignored because of time
	VK_FORMAT_UNDEFINED, // Ignored because of time
	VK_FORMAT_R8G8_UNORM,
	VK_FORMAT_R8G8_UINT,
	VK_FORMAT_R8G8_SNORM,
	VK_FORMAT_R8G8_SINT,
	VK_FORMAT_UNDEFINED, // Ignored because of time
	VK_FORMAT_R16_SFLOAT,
	VK_FORMAT_D16_UNORM,
	VK_FORMAT_R16_UNORM,
	VK_FORMAT_R16_UINT,
	VK_FORMAT_R16_SNORM,
	VK_FORMAT_R16_SINT,
	VK_FORMAT_UNDEFINED, // Ignored because of time
	VK_FORMAT_R8_UNORM,
	VK_FORMAT_R8_UINT,
	VK_FORMAT_R8_SNORM,
	VK_FORMAT_R8_SINT,
	VK_FORMAT_UNDEFINED, // Ignored because of time
	//R1_UNORM // Ignored in Format
	VK_FORMAT_UNDEFINED, // Ignored because of time
	VK_FORMAT_UNDEFINED, // Ignored because of time
	VK_FORMAT_UNDEFINED, // Ignored because of time
	VK_FORMAT_UNDEFINED, // Ignored because of time
	VK_FORMAT_BC1_RGBA_UNORM_BLOCK,
	VK_FORMAT_BC1_RGBA_SRGB_BLOCK,
	VK_FORMAT_UNDEFINED, // Ignored because of time
	VK_FORMAT_BC2_UNORM_BLOCK,
	VK_FORMAT_BC2_SRGB_BLOCK,
	VK_FORMAT_UNDEFINED, // Ignored because of time
	VK_FORMAT_BC3_UNORM_BLOCK,
	VK_FORMAT_BC3_SRGB_BLOCK,
	VK_FORMAT_UNDEFINED, // Ignored because of time
	VK_FORMAT_BC4_UNORM_BLOCK,
	VK_FORMAT_BC4_SNORM_BLOCK,
	VK_FORMAT_UNDEFINED, // Ignored because of time
	VK_FORMAT_BC5_UNORM_BLOCK,
	VK_FORMAT_BC5_SNORM_BLOCK,
	VK_FORMAT_UNDEFINED, // Ignored because of time
	VK_FORMAT_BC6H_UFLOAT_BLOCK,
	VK_FORMAT_BC6H_SFLOAT_BLOCK,
	VK_FORMAT_UNDEFINED, // Ignored because of time
	VK_FORMAT_BC7_UNORM_BLOCK,
	VK_FORMAT_BC7_SRGB_BLOCK,
	VK_FORMAT_B5G6R5_UNORM_PACK16,
	VK_FORMAT_B5G5R5A1_UNORM_PACK16,
	VK_FORMAT_UNDEFINED, // Ignored because of time
	VK_FORMAT_UNDEFINED
};

inline VkFormat MapToVulkanFormat(Format format)
{
	return vulkanFormatMap[(uint32)format];
}
}
