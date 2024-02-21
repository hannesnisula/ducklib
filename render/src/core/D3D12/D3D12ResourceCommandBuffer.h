#pragma once
#include <d3d12.h>

#include "../IResourceCommandBuffer.h"

namespace ducklib::Render
{
class D3D12ResourceCommandBuffer : public IResourceCommandBuffer
{
public:
	friend class D3D12Device;

	Buffer* CreateBuffer() override;

protected:
	D3D12ResourceCommandBuffer(
		ID3D12Device* apiDevice,
		ID3D12CommandList* apiCommandList,
		ID3D12CommandAllocator* apiCommandAllocator);
	~D3D12ResourceCommandBuffer();

	ID3D12Device* apiDevice;
	ID3D12CommandList* apiCommandList;
	ID3D12CommandAllocator* apiCommandAllocator;
};
}
