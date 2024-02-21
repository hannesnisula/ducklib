#pragma once
#include "Core/Memory/Containers/TArray.h"
#include "IAdapter.h"

namespace ducklib::Render
{
class IRHI
{
public:
	virtual ~IRHI() = default;
	virtual const TArray<IAdapter*>& GetAdapters() const = 0;
};
}
