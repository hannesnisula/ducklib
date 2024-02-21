#pragma once
#include <d3d12.h>
#include "../ICommandBuffer.h"

namespace ducklib::Render
{
class D3D12CommandBuffer : public ICommandBuffer
{
public:

	friend class D3D12Device;

	void* GetApiHandle() const override;

	// TODO: Remove?
	void Reset() override;

	void Begin() override;
	void End() override;

	void BeginPass(const IPass* pass, const IFrameBuffer*) override;
	void EndPass() override;

	void SetPipelineState(PipelineState pipelineState) override;

	void Transition(ImageBuffer* image, ResourceState from, ResourceState to) override;
	void SetRT(ImageBuffer* rt) override;
	void SetIndexBuffer(Buffer* buffer) override;
	void SetVertexBuffers(Buffer** buffer, uint32 count, uint32 startSlot) override;
	void SetInputDeclaration(InputDescription* inputDescription) override;

	void SetViewport(const Viewport& viewport) override;
	void SetScissorRect(const Rect& rect) override;

	void SetPrimitiveTopology(PrimitiveTopology topology) override;
	void SetVertexBuffer(uint32_t startIndex, uint32_t numViews, Buffer** buffers) override;

	void Draw() override;

protected:

	D3D12CommandBuffer(
		ID3D12GraphicsCommandList1* apiCommandList,
		ID3D12CommandAllocator* apiCommandAllocator);
	~D3D12CommandBuffer();

private:

	static constexpr uint32 MAX_SET_VB_COUNT = 32;

	ID3D12GraphicsCommandList1* apiCommandList;
	ID3D12CommandAllocator* apiCommandAllocator;
};
}
