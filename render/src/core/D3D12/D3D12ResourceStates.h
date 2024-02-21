#pragma once
#include <d3d12.h>

namespace ducklib::Render
{
// The order of these must match the one in the ResourceState enum
static D3D12_RESOURCE_STATES d3d12ResourceStates[] =
{
	D3D12_RESOURCE_STATE_PRESENT,
	D3D12_RESOURCE_STATE_RENDER_TARGET
};

inline D3D12_RESOURCE_STATES MapToD3D12ResourceState(ResourceState resourceState)
{
	return d3d12ResourceStates[(uint32)resourceState];
}
}
