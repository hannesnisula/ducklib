#pragma once
#include <cstdint>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <vector>
#include "D3D12Common.h"
#include "D3D12Adapter.h"
#include "D3D12SwapChain.h"
#include "../ICommandBuffer.h"
#include "../IResourceCommandBuffer.h"
#include "../IApi.h"
#include "../Resources/Format.h"

namespace DuckLib
{
namespace Render
{
const D3D_FEATURE_LEVEL DL_D3D_FEATURE_LEVEL = D3D_FEATURE_LEVEL_12_1;

class D3D12Api : public IApi
{
public:

	D3D12Api();
	~D3D12Api() override;

	const std::vector<IAdapter*>& GetAdapters() const override;

	ISwapChain* CreateSwapChain(
		uint32 width,
		uint32 height,
		Format format,
		uint32 bufferCount,
		HWND windowHandle) override;
	ICommandBuffer* CreateCommandBuffer() override;
	IResourceCommandBuffer* CreateResourceCommandBuffer() override;

	void DestroySwapChain(ISwapChain* swapChain) override;
	void DestroyCommandBuffer(ICommandBuffer* commandBuffer) override;
	void DestroyResourceCommandBuffer(IResourceCommandBuffer* resourceCommandBuffer) override;

	void ExecuteCommandBuffers(ICommandBuffer** commandBuffers, uint32 numCommandBuffers) override;

	void SignalCompletion(ISwapChain* swapChain) override;

private:

	void EnumAndCreateAdapters();
	ID3D12CommandQueue* CreateQueue(D3D12_COMMAND_LIST_TYPE type, D3D12_COMMAND_QUEUE_FLAGS flags);
	ID3D12DescriptorHeap* CreateDescriptorHeap(uint32 numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type);
	ImageBuffer* CreateImageBuffer(
		uint32 width,
		uint32 height,
		uint32 depth,
		Format format,
		ID3D12Resource* apiResource,
		D3D12_CPU_DESCRIPTOR_HANDLE apiDescriptor);

	void DestroyQueue(ID3D12CommandQueue* queue);
	void DestroyDescriptorHeap(ID3D12DescriptorHeap* descriptorHeap);

	void DestroyAdapters();

	static const uint32 NUM_FRAME_BUFFERS = 4;

#ifdef _DEBUG
	ID3D12Debug* debugInterface;
#endif
	IDXGIFactory4* factory;
	ID3D12Device* device;

	ID3D12CommandQueue* commandQueue;

	ID3D12DescriptorHeap* descriptorHeap;

	// TODO: Replace this temporary store with an allocation object
	std::vector<IAdapter*> adapters;
	std::vector<ISwapChain*> swapChains;
};
}
}
