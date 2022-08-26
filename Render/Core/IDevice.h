#pragma once

#include "ICommandBuffer.h"
#include "ISwapChain.h"

namespace DuckLib::Render
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

	virtual IPass* CreatePass(PassDescription passDesc) = 0;

	virtual void DestroySwapChain(ISwapChain* swapChain) = 0;
	// TODO: Remove? At least for now until there is some good structure on the allocation handling
	virtual void DestroyCommandBuffer(ICommandBuffer* commandBuffer) = 0;

	virtual void ExecuteCommandBuffers(
		ICommandBuffer** commandBuffers,
		uint32_t numCommandBuffers) = 0;

	virtual void SignalCompletion(ISwapChain* swapChain) = 0;
};
}
