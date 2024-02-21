#include <iostream>
#include <conio.h>

#include "Threading/JobQueue.h"

using namespace ducklib;

std::atomic<uint32> numJobsCompleted {0};
uint32* jobItems;
constexpr uint32 NUM_JOBS_NOPAUSE = 512;
constexpr uint32 NUM_CHILD_JOBS = 16;
constexpr uint32 NUM_PAUSE_JOBS = 16;
constexpr uint32 QUEUE_SIZE = 1024;
constexpr uint32 NUM_FIBERS = 512;
uint32 pausePushCounter = 0;

JobQueue jobQueue(QUEUE_SIZE, NUM_FIBERS);

struct ChildJobData
{
	uint32 result;
	uint32 index;
};

struct PauseJobData
{	
	ChildJobData childJobData[NUM_CHILD_JOBS];
};

void JobFunc(void* data)
{
	uint32* jobItem = (uint32*)data;

	*jobItem = 0;
	
	++numJobsCompleted;
}

void ChildPauseJobFunc(void* data)
{
	ChildJobData* jobData = (ChildJobData*)data;

	jobData->result = jobData->index * 10 + 2;
}

void PauseJobFunc(void* data)
{
	PauseJobData* jobData = (PauseJobData*)data;
	Job* jobs = DefAlloc()->Allocate<Job>(NUM_CHILD_JOBS);

	for (uint32 i = 0; i < NUM_CHILD_JOBS; ++i)
	{
		jobData->childJobData[i].result = 0;
		jobData->childJobData[i].index = i + 1;

		jobs[i] = {&ChildPauseJobFunc, &jobData->childJobData[i]};
	}

	pausePushCounter++;
	JobCounter* counter = jobQueue.Push(jobs, NUM_CHILD_JOBS);

	jobQueue.WaitForCounter(counter);
}

void InitJobData(uint32 numJobs)
{
	jobItems = DefAlloc()->Allocate<uint32>(numJobs);
	
	for (uint32 i = 0; i < numJobs; ++i)
		jobItems[i] = i + 1;
}

Job* GenerateJobs(uint32 numJobs)
{
	Job* jobs = DefAlloc()->Allocate<Job>(numJobs);

	for (uint32 i = 0; i < numJobs; ++i)
		jobs[i] = {&JobFunc, &jobItems[i]};
	
	return jobs;
}

bool CheckJobResult(uint32 numJobs)
{
	for (uint32 i = 0; i < numJobs; ++i)
		if (jobItems[i] != 0)
			return false;
	return true;
}

bool CheckPauseJobResults(const PauseJobData* jobData)
{
	for (uint32 i = 0; i < NUM_PAUSE_JOBS; ++i)
		for (uint32 u = 0; u < NUM_CHILD_JOBS; ++u)
			if (jobData[i].childJobData[u].result != jobData[i].childJobData[u].index * 10 + 2)
				return false;

	return true;
}

void CleanupJobData(uint32 numJobs)
{
	DefAlloc()->Free(jobItems);
}

void NoPauseTest()
{
	InitJobData(NUM_JOBS_NOPAUSE);

	Job* jobs = GenerateJobs(NUM_JOBS_NOPAUSE);

	for (uint32 i = 0; i < NUM_JOBS_NOPAUSE; ++i)
		jobQueue.Push(&jobs[i], 1);

	while (numJobsCompleted.load() < NUM_JOBS_NOPAUSE)
		YieldThread(100);

	if (!CheckJobResult(NUM_JOBS_NOPAUSE))
		std::cout << "ERROR: Job result is incorrect" << std::endl;
	else
		std::cout << "Jobs executed successfully" << std::endl;

	CleanupJobData(NUM_JOBS_NOPAUSE);
}

void PauseTest()
{
	PauseJobData* pauseJobData = DefAlloc()->Allocate<PauseJobData>(NUM_PAUSE_JOBS);
	Job* pauseJobs = DefAlloc()->Allocate<Job>(NUM_PAUSE_JOBS);
	
	for (uint32 i = 0; i < NUM_PAUSE_JOBS; ++i)
		pauseJobs[i] = {&PauseJobFunc, &pauseJobData[i]};
	
	JobCounter* counter = jobQueue.Push(pauseJobs, NUM_PAUSE_JOBS); // check if counter ever changes. Also check workers if they actually do any job
	
	jobQueue.WaitForCounter(counter);

	if (!CheckPauseJobResults(pauseJobData))
		std::cout << "Data check failed!" << std::endl;
	else
		std::cout << "Data check passed!" << std::endl;
}

int main()
{
	// NoPauseTest();
	PauseTest();

	_getch();

	return 0;
}