#include <exception>
#include "D3D12CommandBuffer.h"
#include "D3D12Common.h"
#include "D3D12ResourceStates.h"
#include "D3D12SwapChain.h"

namespace DuckLib
{
namespace Render
{
struct Buffer;

D3D12CommandBuffer::D3D12CommandBuffer(
	ID3D12GraphicsCommandList1* apiCommandList,
	ID3D12CommandAllocator* apiCommandAllocator)
	: apiCommandList(apiCommandList)
	, apiCommandAllocator(apiCommandAllocator) { }

D3D12CommandBuffer::~D3D12CommandBuffer()
{
	// TODO: do stuff?
}

const void* D3D12CommandBuffer::GetApiHandle() const
{
	return apiCommandList;
}

void D3D12CommandBuffer::Reset()
{
	if (apiCommandAllocator->Reset() != S_OK)
		throw std::exception("Failed to reset D3D12 command allocator");
	if (apiCommandList->Reset(apiCommandAllocator, nullptr) != S_OK)
		throw std::exception("Failed to reset D3D12 command list");
}

void D3D12CommandBuffer::Close()
{
	apiCommandList->Close();
}

void D3D12CommandBuffer::Transition(ImageBuffer* image, ResourceState from, ResourceState to)
{
	D3D12_RESOURCE_BARRIER barrierDesc;

	barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierDesc.Transition.StateAfter = MapToD3D12ResourceState(to);
	barrierDesc.Transition.StateBefore = MapToD3D12ResourceState(from);
	barrierDesc.Transition.pResource = (ID3D12Resource*)image->apiResource;
	barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

	apiCommandList->ResourceBarrier(1, &barrierDesc);
}

void D3D12CommandBuffer::SetRT(ImageBuffer* rt)
{
	apiCommandList->OMSetRenderTargets(
		1,
		(const D3D12_CPU_DESCRIPTOR_HANDLE*)&rt->apiDescriptor,
		FALSE,
		nullptr);
}

void D3D12CommandBuffer::SetRT(ISwapChain* swapChain)
{
	D3D12SwapChain* d3dSwapChain = (D3D12SwapChain*)swapChain;
	D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle = d3dSwapChain->GetCurrentCpuDescriptorHandle();

	apiCommandList->OMSetRenderTargets(
		1,
		&descriptorHandle,
		FALSE,
		nullptr);
}

void D3D12CommandBuffer::Clear(ImageBuffer* rt, float* rgbaColor)
{
	apiCommandList->ClearRenderTargetView(
		*(D3D12_CPU_DESCRIPTOR_HANDLE*)&rt->apiDescriptor,
		rgbaColor,
		0,
		nullptr);
}

void D3D12CommandBuffer::SetViewport(const Viewport& viewport)
{
	D3D12_VIEWPORT d3dViewport{
		viewport.topLeftX,
		viewport.topleftY,
		viewport.width,
		viewport.height,
		viewport.minDepth,
		viewport.maxDepth
	};

	apiCommandList->RSSetViewports(1, &d3dViewport);
}

void D3D12CommandBuffer::SetScissorRect(const Rect& scissorRect)
{
	D3D12_RECT d3dScissorRect { scissorRect.left, scissorRect.top, scissorRect.right, scissorRect.bottom };

	apiCommandList->RSSetScissorRects(1, &d3dScissorRect);
}

void D3D12CommandBuffer::SetPrimitiveTopology(PrimitiveTopology topology)
{
	apiCommandList->IASetPrimitiveTopology(MapPrimitiveTopology(topology));
}

void D3D12CommandBuffer::SetVertexBuffer(uint32_t startIndex, uint32_t numViews, Buffer** buffers)
{
	// d3dbuffers here

	// apiCommandList->IASetVertexBuffers(startIndex, numViews, buffers);
}
void D3D12CommandBuffer::Draw()
{
	
}
}
}