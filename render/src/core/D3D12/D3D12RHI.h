#pragma once

#include "D3D12Adapter.h"
#include "Lib/dxgi1_4.h"
#include "../IRHI.h"
#include "Core/Memory/Containers/TArray.h"

namespace ducklib::Render
{
class D3D12RHI : IRHI
{
public:
	static IRHI* GetInstance();

	~D3D12RHI() override;

	const TArray<IAdapter*>& GetAdapters() const override;

protected:
	D3D12RHI();

	void InitFactory();
	// Currently only run when creating the RHI
	void EnumerateAdapters();

	static constexpr uint32_t MAX_NUM_ADAPTERS = 16;
	static constexpr uint32_t ADAPTER_DESCRIPTION_BUFFER_SIZE = 256;

	IAllocator* alloc;

	IDXGIFactory4* factory;
#ifdef _DEBUG
	ID3D12Debug* debugInterface;
#endif

	TArray<IAdapter*> adapters;
};
}
