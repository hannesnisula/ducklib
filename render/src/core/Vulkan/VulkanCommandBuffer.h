#pragma once

#include "Lib/vulkan.h"
#include "Render/Core/ICommandBuffer.h"

namespace ducklib::Render
{
class VulkanCommandBuffer : public ICommandBuffer
{
public:
	friend class VulkanDevice;

	void* GetApiHandle() const override;

	void Reset() override;

	void Begin() override;
	void End() override;

	void BeginPass(const IPass* pass, const IFrameBuffer*) override;
	void EndPass() override;

	void SetPipelineState(PipelineState pipelineState) override;

	// TODO: Remove?
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
	VulkanCommandBuffer(VkCommandBuffer vkCommandBuffer);

	VkCommandBuffer vkCommandBuffer;
};
}
