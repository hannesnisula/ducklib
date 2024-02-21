#include "Thread.h"

namespace ducklib
{
Thread::Thread(uint32 (* func)(void*), void* data)
{
#ifdef _WIN32
	uint32 threadId {};
	
	osHandle = CreateThread(nullptr,
		0,
		(LPTHREAD_START_ROUTINE)func,
		data,
		0,
		(LPDWORD)&threadId);
#endif
}

void Thread::Join()
{
	WaitForSingleObject(osHandle, INFINITE);
}

void YieldThread(uint32 ms)
{
#ifdef _WIN32
	::Sleep(ms);
#endif
}
}