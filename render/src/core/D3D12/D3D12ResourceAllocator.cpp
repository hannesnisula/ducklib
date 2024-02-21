#include "D3D12ResourceAllocator.h"
#include "Core/Memory/IAllocator.h"

namespace ducklib::Render
{
D3D12ResourceAllocator::D3D12ResourceAllocator()
{
	alloc = DefAlloc();
}

ImageBuffer* D3D12ResourceAllocator::AllocatorImageBuffer()
{
	return nullptr;
}

D3D12Adapter* D3D12ResourceAllocator::AllocateAdapter()
{
	// D3D12Adapter* adapter = alloc->New<D3D12Adapter>();
	// Is this a good idea?
	D3D12Adapter* adapter = alloc->Allocate<D3D12Adapter>();

	adapters.Append(adapter);

	return adapter;
}

void D3D12ResourceAllocator::FreeImageBuffer(ImageBuffer* imageBuffer)
{
	
}

void D3D12ResourceAllocator::FreeAdapter(D3D12Adapter* adapter)
{
	// This is kind of verbose and annoying
	// std::vector<D3D12Adapter*>::iterator it = std::find(adapters.begin(), adapters.end(), adapter);
	//
	// if (it != adapters.end())
	// 	adapters.erase(it);

	// TODO: Lots of work to be done for render resource allocation!
	// alloc->Delete(adapter);
}
}
