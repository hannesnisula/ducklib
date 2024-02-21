#pragma once
#include "IFrameBuffer.h"
#include "IPass.h"
#include "PipelineState.h"
#include "Resources/Buffer.h"
#include "Resources/ImageBuffer.h"
#include "Resources/InputDescription.h"
#include "Resources/PrimitiveTopology.h"
#include "Resources/ResourceStates.h"
#include "Resources/Viewport.h"
#include "Resources/Rect.h"

namespace ducklib::Render
{
class ICommandBuffer
{
public:
	virtual void* GetApiHandle() const = 0;

	// TODO: Make virtual?
	// RGBA
	void SetClearColor(const float* rgbaColor);

	virtual void Reset() = 0;

	virtual void Begin() = 0;
	virtual void End() = 0;

	virtual void BeginPass(const IPass* pass, const IFrameBuffer*) = 0;
	virtual void EndPass() = 0;

	virtual void SetPipelineState(PipelineState pipelineState) = 0;

	virtual void Transition(ImageBuffer* image, ResourceState from, ResourceState to) = 0;
	virtual void SetRT(ImageBuffer* rt) = 0;
	virtual void SetIndexBuffer(Buffer* buffer) = 0;
	virtual void SetVertexBuffers(Buffer** buffer, uint32 count, uint32 startSlot) = 0;
	virtual void SetInputDeclaration(InputDescription* inputDescription) = 0;

	virtual void SetViewport(const Viewport& viewport) = 0;
	virtual void SetScissorRect(const Rect& rect) = 0;
	virtual void SetPrimitiveTopology(PrimitiveTopology topology) = 0;
	virtual void SetVertexBuffer(uint32_t startIndex, uint32_t numViews, Buffer** buffers) = 0;

	virtual void Draw() = 0;

protected:
	float clearColorRgba[4];
};

inline void ICommandBuffer::SetClearColor(const float* rgbaColor)
{
	clearColorRgba[0] = rgbaColor[0];
	clearColorRgba[1] = rgbaColor[1];
	clearColorRgba[2] = rgbaColor[2];
	clearColorRgba[3] = rgbaColor[3];
}
}
