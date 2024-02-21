#include <exception>
#include "D3D12CommandBuffer.h"
#include "D3D12Common.h"
#include "D3D12ResourceStates.h"
#include "D3D12SwapChain.h"

namespace ducklib::Render
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

void* D3D12CommandBuffer::GetApiHandle() const
{
	return apiCommandList;
}

void D3D12CommandBuffer::Reset()
{
	if (apiCommandAllocator->Reset() != S_OK)
		throw std::runtime_error("Failed to reset D3D12 command allocator");
	if (apiCommandList->Reset(apiCommandAllocator, nullptr) != S_OK)
		throw std::runtime_error("Failed to reset D3D12 command list");
}

void D3D12CommandBuffer::Begin()
{
	// apiCommandList->ClearRenderTargetView(
	// 	*(D3D12_CPU_DESCRIPTOR_HANDLE*)&rt->apiDescriptor,
	// 	clearColorRgba,
	// 	0,
	// 	nullptr);
}

void D3D12CommandBuffer::End()
{
	apiCommandList->Close();
}
void D3D12CommandBuffer::BeginPass(const IPass* pass, const IFrameBuffer*)
{
	
}
void D3D12CommandBuffer::EndPass() {}
void D3D12CommandBuffer::SetPipelineState(PipelineState pipelineState) {}

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

void D3D12CommandBuffer::SetIndexBuffer(Buffer* buffer)
{
	apiCommandList->IASetIndexBuffer((D3D12_INDEX_BUFFER_VIEW*)buffer->apiResource);
}

void D3D12CommandBuffer::SetVertexBuffers(Buffer** buffer, uint32 count, uint32 startSlot)
{
	D3D12_VERTEX_BUFFER_VIEW apiViews[MAX_SET_VB_COUNT];

	for (uint32 i = 0; i < count; ++i)
		apiViews[i] = *(D3D12_VERTEX_BUFFER_VIEW*)buffer[i]->apiResource;

	apiCommandList->IASetVertexBuffers(startSlot, count, apiViews);
}

void D3D12CommandBuffer::SetInputDeclaration(InputDescription* inputDescription)
{
	// D3D12_INPUT_ELEMENT_DESC desc;
}

void D3D12CommandBuffer::SetViewport(const Viewport& viewport)
{
	D3D12_VIEWPORT d3dViewport{
		viewport.topLeftX,
		viewport.topLeftY,
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
	apiCommandList->IASetPrimitiveTopology(MapD3D12PrimitiveTopology(topology));
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
