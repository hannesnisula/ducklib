#include "D3D12Adapter.h"

#include <dxgi1_3.h>
#include "D3D12Common.h"
#include "D3D12Device.h"

namespace ducklib::Render
{
void* D3D12Adapter::GetApiHandle() const
{
	return apiAdapter;
}

IDevice* D3D12Adapter::CreateDevice()
{
	ID3D12Device* d3dDevice;
	
	DL_D3D12_CHECK(
		D3D12CreateDevice(apiAdapter, DL_D3D_FEATURE_LEVEL, IID_PPV_ARGS(&d3dDevice)),
		"Failed to create D3D12 device");

	D3D12Device* device = alloc->Allocate<D3D12Device>();

	new(device) D3D12Device(d3dDevice, factory);

	return device;
}

D3D12Adapter::D3D12Adapter(const char* description, bool isHardware, IDXGIAdapter1* apiAdapter, IDXGIFactory4* factory)
	: IAdapter(description, isHardware)
	, alloc(DefAlloc())
	, factory(factory)
	, apiAdapter(apiAdapter) {}

D3D12Adapter::~D3D12Adapter()
{
	if (apiAdapter)
		apiAdapter->Release();
}
}
