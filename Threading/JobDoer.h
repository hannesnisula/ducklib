#pragma once
#include <cstdint>
#include "ConcurrentQueue.h"
#include "Thread.h"

namespace DuckLib
{
struct Job
{
	void (*jobFunction)(void*);
	void* jobData;
};

namespace Internal
{
namespace Job
{
struct Fiber
{
	void* osFiber;
	DuckLib::Job* currentJob;

	static const uint32_t DEFAULT_STACK_SIZE = 65536;
};

void SwitchFiber(Fiber* fiber);
void InitWorkerThread();
uint32_t _stdcall WorkerThreadJob(void* data);
void _stdcall FiberJobWrapper(void* data);
}
}

class JobDoer
{
public:

	JobDoer(uint32_t jobQueueSize, uint32_t numFibers);

	bool Queue(Job* job);

private:

	friend uint32_t _stdcall Internal::Job::WorkerThreadJob(void* data);
	friend void _stdcall Internal::Job::FiberJobWrapper(void* data);

	static const uint8_t CACHE_LINE_SIZE = 64;

	struct WorkerThreadData
	{
		Thread* thread;
		JobDoer* jobDoer;
	};

	Internal::Job::Fiber CreateFiber(void* fiberData);
	uint32_t GetNumLogicalCores() const;

	Internal::Job::Fiber* fibers;
	uint32_t numFibers;

	uint32_t jobQueueSize;

	ConcurrentQueue<Internal::Job::Fiber>* fiberQueue;
	ConcurrentQueue<Job>* jobQueue;

	Thread** workerThreads;
};
}