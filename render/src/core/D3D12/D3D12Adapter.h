#pragma once
#include "../../../Core/Memory/IAllocator.h"
#include <dxgi.h>
#include "D3D12Device.h"
#include "../IAdapter.h"

namespace ducklib::Render
{
class D3D12Adapter : public IAdapter
{
public:

	friend class D3D12RHI;

	void* GetApiHandle() const override;
	IDevice* CreateDevice() override;

protected:
	D3D12Adapter(const char* description, bool isHardware, IDXGIAdapter1* apiAdapter, IDXGIFactory4* factory);
	~D3D12Adapter() override;

	IAllocator* alloc;

	IDXGIFactory4* factory;
	IDXGIAdapter1* apiAdapter;
};
}
