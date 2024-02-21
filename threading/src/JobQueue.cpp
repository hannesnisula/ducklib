#include <Windows.h>
#include <atomic>
#include <cassert>
#include "JobQueue.h"
#include <mutex>
#include <unordered_map>

namespace ducklib
{
Job::Job()
	: jobCounter(nullptr)
	, jobFunction(nullptr)
	, jobData(nullptr) {}

Job::Job(void (*jobFunction)(void*), void* jobData)
	: jobCounter(nullptr)
	, jobFunction(jobFunction)
	, jobData(jobData) {}

namespace Internal
{
std::atomic<bool> runWorkers;
thread_local Fiber workerThreadFiber{};
thread_local bool isWorkerThread{ false };
thread_local Fiber* currentFiber{};

void SwitchFiber(const Fiber* fiber)
{
#ifdef _WIN32
	SwitchToFiber(fiber->osFiber);
#endif
}

void SwitchToWorker()
{
	SwitchFiber(&workerThreadFiber);
}

void InitWorkerThread()
{
	isWorkerThread = true;
#ifdef _WIN32
	workerThreadFiber.osFiber = ConvertThreadToFiber(&workerThreadFiber);
#endif
}

uint32 _stdcall WorkerThreadJob(void* data)
{
	JobQueue::WorkerThreadData* workerThreadData = (JobQueue::WorkerThreadData*)data;
	std::atomic<bool>& runFlag = workerThreadData->runFlag;
	std::atomic<bool>& startFlag = workerThreadData->startFlag;
	JobQueue* jobQueue = workerThreadData->jobQueue;

	InitWorkerThread();

	while (!startFlag.load())
		YieldThread(10);

	while (runFlag.load())
	{
		Fiber* jobFiber = jobQueue->GetReadyJobAndFiber();

		if (!jobFiber)
		{
			YieldThread(10);
			continue;
		}

		if (!jobFiber->currentJob.jobFunction)
			throw std::runtime_error("Tried to start a fiber with a nullptr job");

		currentFiber = jobFiber;
		SwitchFiber(currentFiber);
		currentFiber = nullptr;
		jobQueue->ReturnFiberIfJobCompleted(jobFiber);
	}

	return 0;
}

void _stdcall FiberJobWrapper(void* data)
{
	Fiber* fiberData = (Fiber*)data;

	while (true)
	{
		Job job = fiberData->currentJob;

		job.jobFunction(job.jobData);
		job.jobCounter->Decrement();
		fiberData->currentJob = {};
		SwitchToWorker();
	}
}
}

void JobCounter::Reset()
{
	waitingJobFiber = nullptr;
}

void JobCounter::Decrement()
{
	if (--counter == 0 && waitingJobFiber)
		jobQueue->FinalizeCompletedJobCounter(this);
}

JobQueue::JobQueue(uint32 size, uint32 numFibers, uint32 numWorkers)
	: alloc(DefAlloc())
{
	this->numWorkers = numWorkers == MATCH_NUM_LOGICAL_CORES ? GetNumLogicalCores() : numWorkers;
	queueSize = size;

	uintptr_t* initPtrArrayBuffer = (uintptr_t*)alloc->Allocate(max(size, numFibers) * sizeof(uintptr_t));

	SetupCounters(size, initPtrArrayBuffer);
	SetupFibers(numFibers, initPtrArrayBuffer);
	SetupJobStorage(size);
	SetupWorkers(this->numWorkers);

	alloc->Free(initPtrArrayBuffer);

	workerThreadData.startFlag.store(true);
}

JobQueue::~JobQueue()
{
	TearDownWorkers();
	TearDownJobStorage();
	TearDownFibers();
	TearDownCounters();
}

JobCounter* JobQueue::Push(Job* jobs, uint32 numJobs)
{
	JobCounter* jobCounter;

	if (!counterQueue->TryPop(&jobCounter))
		throw std::runtime_error("Failed to acquire job counter");

	for (uint32 i = 0; i < numJobs; ++i)
		jobs[i].jobCounter = jobCounter;

	uint32 jobsQueued = jobQueue->TryPush(jobs, numJobs);

	if (jobsQueued != numJobs)
		throw std::runtime_error("Failed to push jobs to queue");

	jobCounter->counter.store(jobsQueued);

	return jobCounter;
}

void JobQueue::WaitForCounter(JobCounter* counter)
{
	if (Internal::isWorkerThread)
	{
		counter->waitingJobFiber = Internal::currentFiber;
		std::atomic_thread_fence(std::memory_order_release);
		Internal::SwitchToWorker();
	}
	else
		WaitIdle(counter);
}

Internal::Fiber* JobQueue::GetReadyJobAndFiber()
{
	Internal::Fiber* readyPausedJobFiber;

	if (readyPausedJobFiberQueue->TryPop(&readyPausedJobFiber))
		return readyPausedJobFiber;

	Job newJob;
	Internal::Fiber* newJobFiber;

	if (jobQueue->TryPop(&newJob))
	{
		if (!fiberQueue->TryPop(&newJobFiber))
			throw std::runtime_error("Failed to acquire fiber for new job");

		newJobFiber->currentJob = newJob;
		return newJobFiber;
	}

	return nullptr;
}

void JobQueue::ReturnFiberIfJobCompleted(Internal::Fiber* completedFiber)
{
	// jobFunction == nullptr -> completed, otherwise paused
	if (completedFiber->currentJob.jobFunction)
		return;

	if (!fiberQueue->TryPush(completedFiber))
		throw std::runtime_error("Failed to push used fiber back on fiber queue. Wtf?");
}

void JobQueue::FinalizeCompletedJobCounter(JobCounter* counter)
{
	if (!readyPausedJobFiberQueue->TryPush(counter->waitingJobFiber))
		throw std::runtime_error("Failed to push job onto job queue");

	counter->Reset();

	if (!counterQueue->TryPush(counter))
		throw std::runtime_error("Failed to push job counter back on job counter queue");
}

Internal::Fiber JobQueue::CreateFiber(void* fiberData)
{
	Internal::Fiber fiber;

	fiber.currentJob = {};
#ifdef _WIN32
	fiber.osFiber = ::CreateFiber(
		Internal::Fiber::DEFAULT_STACK_SIZE,
		&Internal::FiberJobWrapper,
		fiberData);
#endif

	return fiber;
}

void JobQueue::DeleteFiber(Internal::Fiber* fiber)
{
#ifdef _WIN32
	::DeleteFiber(fiber->osFiber);
#endif
	fiber->~Fiber();
}

uint32 JobQueue::GetNumLogicalCores() const
{
#ifdef _WIN32
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	return sysInfo.dwNumberOfProcessors;
#endif
}

void JobQueue::WaitIdle(const JobCounter* counter)
{
	while (counter->counter.load() != 0)
		Sleep(5);
}

void JobQueue::SetupCounters(uint32 numCounters, uintptr_t* initPtrArrayBuffer)
{
	this->numCounters = numCounters;
	counters = alloc->Allocate<JobCounter>(numCounters);

	for (uint32 i = 0; i < numCounters; ++i)
	{
		new(&counters[i]) JobCounter();
		counters[i].jobQueue = this;
		initPtrArrayBuffer[i] = (uintptr_t)&counters[i];
	}

	counterQueue = alloc->New<ConcurrentQueue<JobCounter*>>(numCounters, (JobCounter**)initPtrArrayBuffer, numCounters);
}

void JobQueue::SetupFibers(uint32 numFibers, uintptr_t* initPtrArrayBuffer)
{
	this->numFibers = numFibers;
	fibers = alloc->Allocate<Internal::Fiber>(numFibers);

	for (uint32 i = 0; i < numFibers; ++i)
	{
		new(&fibers[i]) Internal::Fiber();
		fibers[i] = CreateFiber(&fibers[i]);
		initPtrArrayBuffer[i] = (uintptr_t)&fibers[i];
	}

	fiberQueue = alloc->New<ConcurrentQueue<Internal::Fiber*>>(
		numFibers,
		(Internal::Fiber**)initPtrArrayBuffer,
		numFibers);
}

void JobQueue::SetupJobStorage(uint32 size)
{
	readyPausedJobFiberQueue = alloc->New<ConcurrentQueue<Internal::Fiber*>>(size);
	jobQueue = alloc->New<ConcurrentQueue<Job>>(size);
}

void JobQueue::SetupWorkers(uint32 numWorkers)
{
	workerThreads = alloc->Allocate<Thread*>(numWorkers);

	workerThreadData.jobQueue = this;
	workerThreadData.runFlag.store(true);
	workerThreadData.startFlag.store(false);

	for (uint32 i = 0; i < numWorkers; ++i)
		workerThreads[i] = alloc->New<Thread>(Internal::WorkerThreadJob, &workerThreadData);
}

void JobQueue::TearDownWorkers()
{
	workerThreadData.runFlag.store(false);

	for (uint32 i = 0; i < numWorkers; ++i)
	{
		workerThreads[i]->Join();
		alloc->Delete(workerThreads[i]);
	}

	alloc->Free(workerThreads);
}

void JobQueue::TearDownJobStorage()
{
	// TODO: Consider checking if all jobs have been completed before tearing down? Or quick exit?
	alloc->Delete(jobQueue);
	alloc->Delete(readyPausedJobFiberQueue);
}

void JobQueue::TearDownFibers()
{
	for (uint32 i = 0; i < numFibers; ++i)
		DeleteFiber(&fibers[i]);

	alloc->Delete(fiberQueue);
	alloc->Free(fibers);
}

void JobQueue::TearDownCounters()
{
	alloc->Delete(counterQueue);

	for (uint32 i = 0; i < numCounters; ++i)
		counters[i].~JobCounter();

	alloc->Free(counters);
}
}
