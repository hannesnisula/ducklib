#include <stdexcept>
#include "D3D12RHI.h"
#include "D3D12Common.h"
#include "D3D12Device.h"
#include "Core/Utility.h"
#include "Core/Memory/IAllocator.h"
#include "Core/Memory/Containers/Iterators.h"

namespace ducklib::Render
{
IRHI* D3D12RHI::GetInstance()
{
	static D3D12RHI rhi;

	return &rhi;
}

D3D12RHI::~D3D12RHI()
{
	for (IAdapter* adapter : adapters)
	{
		((D3D12Adapter*)adapter)->~D3D12Adapter();
		alloc->Free(adapter);
	}
}

const TArray<IAdapter*>& D3D12RHI::GetAdapters() const
{
	return adapters;
}

D3D12RHI::D3D12RHI()
	: alloc(nullptr)
	, factory(nullptr)
{
	alloc = DefAlloc();
	InitFactory();
	EnumerateAdapters();
}

void D3D12RHI::InitFactory()
{
#ifdef _DEBUG
	DL_D3D12_CHECK(
		CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&factory)),
		"Failed to create DXGI factory");

	DL_D3D12_CHECK(
		D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)),
		"Failed to get debugInterface interface");

	debugInterface->EnableDebugLayer();
#else
	DL_D3D12_CHECK(
		CreateDXGIFactory2(IID_PPV_ARGS(&d3dFactory)),
		"Failed to create DXGI factory");
#endif
}

void D3D12RHI::EnumerateAdapters()
{
	IDXGIAdapter1* adapterIt;
	uint32_t adapterCounter = 0;

	for (; factory->EnumAdapters1(adapterCounter, &adapterIt) != DXGI_ERROR_NOT_FOUND; ++adapterCounter)
	{
		DXGI_ADAPTER_DESC1 adapterDesc;
		char descriptionBuffer[ADAPTER_DESCRIPTION_BUFFER_SIZE];

		adapterIt->GetDesc1(&adapterDesc);
		Utility::WideToMultiByteText(adapterDesc.Description, descriptionBuffer, ADAPTER_DESCRIPTION_BUFFER_SIZE);

		// S_FALSE when testing device (nullptr) == success
		if (D3D12CreateDevice(adapterIt, DL_D3D_FEATURE_LEVEL, _uuidof(ID3D12Device), nullptr) == S_FALSE)
		{
			D3D12Adapter* adapter = alloc->Allocate<D3D12Adapter>();

			new(adapter) D3D12Adapter(
				descriptionBuffer,
				(adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0,
				adapterIt,
				factory);

			adapters.Append(adapter);
		}
	}
}
}
