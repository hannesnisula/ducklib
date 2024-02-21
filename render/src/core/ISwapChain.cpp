#include <stdexcept>
#include "ISwapChain.h"

namespace ducklib::Render
{
ImageBuffer* ISwapChain::GetBuffer(uint32 index)
{
	return &buffers[index];
}

ImageBuffer* ISwapChain::GetCurrentBuffer()
{
	return &buffers[currentFrameIndex % numBuffers];
}

uint32 ISwapChain::GetCurrentBufferIndex() const
{
	return currentFrameIndex;
}

ISwapChain::ISwapChain()
	: currentFrameIndex(0)
{
	for (uint32 i = 0; i < MAX_BUFFERS; ++i)
		frameCounters[i] = UINT32_MAX;
}
}
