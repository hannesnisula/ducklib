#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
#include "D3D12Fence.h"
#include "../ISwapChain.h"

namespace ducklib::Render
{
class D3D12SwapChain : public ISwapChain
{
public:
	friend class D3D12Device;

	// TODO: -> protected
	D3D12SwapChain(
		uint32 width,
		uint32 height,
		Format format,
		IDXGISwapChain3* d3dSwapChain,
		uint32 bufferCount,
		const ImageBuffer* images,
		ID3D12DescriptorHeap* descriptorHeap,
		uint32 descriptorSize);
	~D3D12SwapChain() override;

	void* GetImageAvailabilitySemaphore() override;

	void Present() override;
	void PrepareFrame() override;

protected:
	IDXGISwapChain3* d3dSwapChain;
	ID3D12DescriptorHeap* rtDescriptorHeap;
	HANDLE fenceEventHandle;
	uint32 descriptorSize;
};
}
