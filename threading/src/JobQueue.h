#pragma once
#include <cstdint>
#include "ConcurrentQueue.h"
#include "Thread.h"

namespace ducklib
{
struct JobCounter;
class JobQueue;

namespace Internal
{
void SwitchToWorker();
void InitWorkerThread();
uint32 __stdcall WorkerThreadJob(void* data);
void __stdcall FiberJobWrapper(void* data);
}

struct Job
{
	Job();
	Job(void (*jobFunction)(void*), void* jobData);

private:

	friend void __stdcall Internal::FiberJobWrapper(void*);
	friend uint32 __stdcall Internal::WorkerThreadJob(void*);
	friend class JobQueue;

	JobCounter* jobCounter;
	void (*jobFunction)(void*);
	void* jobData;
};

namespace Internal
{
void _stdcall FiberJobWrapper(void*);
	
struct alignas(CACHE_LINE_SIZE) Fiber
{
	Job currentJob;
	void* osFiber;

	static const uint32 DEFAULT_STACK_SIZE = 65536;
};

void SwitchFiber(const Fiber* fiber);
}

struct alignas(CACHE_LINE_SIZE) JobCounter
{
	friend void _stdcall Internal::FiberJobWrapper(void*);
	friend class JobQueue;

protected:

	void Reset();
	void Decrement();
	
	JobQueue* jobQueue;
	std::atomic<uint32> counter;
	Internal::Fiber* waitingJobFiber;
};

class JobQueue
{
public:

	static constexpr uint32 MATCH_NUM_LOGICAL_CORES = 0;
	
	JobQueue(uint32 size, uint32 numFibers, uint32 numWorkers = MATCH_NUM_LOGICAL_CORES);
	~JobQueue();

	JobCounter* Push(Job* jobs, uint32 numJobs);

	void WaitForCounter(JobCounter* counter);

private:

	friend struct Internal::Fiber;
	friend struct JobCounter;
	friend uint32 __stdcall Internal::WorkerThreadJob(void* data);
	friend void __stdcall Internal::FiberJobWrapper(void* data);

	struct WorkerThreadData
	{
		JobQueue* jobQueue;
		std::atomic<bool> runFlag;
		std::atomic<bool> startFlag;
	};

	Internal::Fiber* GetReadyJobAndFiber();
	void ReturnFiberIfJobCompleted(Internal::Fiber* completedFiber);

	void FinalizeCompletedJobCounter(JobCounter* counter);

	Internal::Fiber CreateFiber(void* fiberData);
	void DeleteFiber(Internal::Fiber* fiber);
	uint32 GetNumLogicalCores() const;

	void WaitIdle(const JobCounter* counter);

	void SetupCounters(uint32 numCounters, uintptr_t* initPtrArrayBuffer);
	void SetupFibers(uint32 numFibers, uintptr_t* initPtrArrayBuffer);
	void SetupJobStorage(uint32 size);
	void SetupWorkers(uint32 numWorkers);

	void TearDownWorkers();
	void TearDownJobStorage();
	void TearDownFibers();
	void TearDownCounters();

	IAllocator* alloc;

	uint32 numCounters;
	JobCounter* counters;
	ConcurrentQueue<JobCounter*>* counterQueue;
	
	uint32 numFibers;
	Internal::Fiber* fibers;
	ConcurrentQueue<Internal::Fiber*>* fiberQueue;

	uint32 queueSize;
	ConcurrentQueue<Internal::Fiber*>* readyPausedJobFiberQueue;
	ConcurrentQueue<Job>* jobQueue;

	uint32 numWorkers;
	Thread** workerThreads;
	WorkerThreadData workerThreadData;

	// TODO: Implement
#ifdef _DEBUG
	uint32 maxNumFibers;
	uint32 maxNumJobs;
	uint32 maxNumPausedJobs;
	uint32 maxNumJobCounters;
#endif
};
}