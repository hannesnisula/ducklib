#pragma once
#include "Core/Types.h"
#include "Lib/d3d12.h"
#include "Render/Core/IFence.h"

namespace ducklib::Render
{
class D3D12Fence : public IFence
{
public:
	~D3D12Fence() override;

	void Wait() override;

protected:
	friend class D3D12Device;

	D3D12Fence(ID3D12Fence* d3dFence);

	ID3D12Fence* d3dFence;
	HANDLE fenceEventHandle;
	uint64 expectedValue;
};
}
