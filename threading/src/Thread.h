#pragma once
#include <Windows.h>
#include "Core/Types.h"

namespace ducklib
{
class Thread
{
public:

	Thread(uint32 (*func)(void*), void* data);

	void Join();

private:

#ifdef _WIN32
	HANDLE osHandle;
#else
	void* osHandle;
#endif
};

void YieldThread(uint32 ms);
}