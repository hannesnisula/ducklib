#pragma once

#include "ICommandBuffer.h"
#include "IFence.h"
#include "IFrameBuffer.h"
#include "ISwapChain.h"
#include "PassDescription.h"

namespace ducklib::Render
{
class IDevice
{
public:

	virtual ~IDevice() {}
	
	virtual ISwapChain* CreateSwapChain(
		uint32_t width,
		uint32_t height,
		Format format,
		uint32_t bufferCount,
		HWND windowHandle) = 0;
	virtual ICommandBuffer* CreateCommandBuffer() = 0;

	virtual IPass* CreatePass(const PassDescription& passDesc) = 0;
	virtual IFrameBuffer* CreateFrameBuffer(ImageBuffer** imageBuffers, uint32 imageBufferCount, IPass* pass) = 0;

	virtual void DestroySwapChain(ISwapChain* swapChain) = 0;
	// TODO: Remove? At least for now until there is some good structure on the allocation handling
	virtual void DestroyCommandBuffer(ICommandBuffer* commandBuffer) = 0;

	virtual void ExecuteCommandBuffers(
		ICommandBuffer** commandBuffers,
		uint32_t numCommandBuffers,
		void* waitSemaphore,
		IFence* signalFence) = 0;

	virtual IFence* CreateFence() = 0;
};
}
