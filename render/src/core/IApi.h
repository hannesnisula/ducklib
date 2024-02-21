#pragma once

#include <vector>
#include "IAdapter.h"
#include "ICommandBuffer.h"

namespace DuckLib
{
namespace Render
{
class IDevice
{
public:

	virtual ~IDevice() {}
	
	virtual const std::vector<IAdapter*>& GetAdapters() const = 0;
	virtual ISwapChain* CreateSwapChain(
		uint32 width,
		uint32 height,
		Format format,
		uint32 bufferCount,
		HWND windowHandle) = 0;
	virtual ICommandBuffer* CreateCommandBuffer() = 0;

	virtual void DestroySwapChain(ISwapChain* swapChain) = 0;
	virtual void DestroyCommandBuffer(ICommandBuffer* commandBuffer) = 0;

	virtual void ExecuteCommandBuffers(
		ICommandBuffer** commandBuffers,
		uint32 numCommandBuffers) = 0;

	virtual void SignalCompletion(ISwapChain* swapChain) = 0;
};
}
}
