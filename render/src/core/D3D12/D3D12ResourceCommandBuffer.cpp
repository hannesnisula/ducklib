#include "D3D12ResourceCommandBuffer.h"

namespace ducklib::Render
{
Buffer* D3D12ResourceCommandBuffer::CreateBuffer()
{
	// apiDevice->CreateCommittedResource()

	// TODO: Implement
	return nullptr;
}

D3D12ResourceCommandBuffer::D3D12ResourceCommandBuffer(
	ID3D12Device* apiDevice,
	ID3D12CommandList* apiCommandList,
	ID3D12CommandAllocator* apiCommandAllocator)
{
	this->apiDevice = apiDevice;
	this->apiCommandList = apiCommandList;
	this->apiCommandAllocator = apiCommandAllocator;
}

D3D12ResourceCommandBuffer::~D3D12ResourceCommandBuffer()
{
	apiCommandList->Release();
	apiCommandAllocator->Release();
}
}
