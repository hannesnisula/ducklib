#pragma once
#include "D3D12Adapter.h"
#include "../Resources/ImageBuffer.h"
#include "Core/Memory/Containers/TArray.h"

namespace DuckLib::Render
{
class D3D12ResourceAllocator
{
public:

	D3D12ResourceAllocator();

	ImageBuffer* AllocatorImageBuffer();
	D3D12Adapter* AllocateAdapter();

	void FreeImageBuffer(ImageBuffer* imageBuffer);
	void FreeAdapter(D3D12Adapter* adapter);
	
private:

	IAlloc* alloc;

	TArray<D3D12Adapter*> adapters;
};
}
