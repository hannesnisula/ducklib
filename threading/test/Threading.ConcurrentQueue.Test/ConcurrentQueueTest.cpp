#include <conio.h>
#include <exception>
#include <iostream>
#include "Threading/ConcurrentQueue.h"
#include "Threading/Thread.h"

using namespace ducklib;

struct WorkerConfig
{
	ConcurrentQueue<uint32>* queue;
	std::atomic<uint32>* pushCounter;
	std::atomic<uint32>* pushReservation;
	uint32* items;
	uint32* allPoppedItems;
	uint32 numItems;
	std::atomic<uint32>* numAllPoppedItems;
	bool push;
	bool pop;
};

uint32 QueueWorker(void* configData)
{
	WorkerConfig* config = (WorkerConfig*)configData;
	bool queueIsEmpty = !config->pop;
	uint32 value {};
	uint32* poppedItems = DefAlloc()->Allocate<uint32>(config->numItems);
	uint32 numPopped = 0;
	uint64 counter = config->pushCounter->load();
	uint64 pushReservation = config->push ? (*config->pushReservation)++ : (uint64)-1;

	while (counter < config->numItems || pushReservation < config->numItems || !queueIsEmpty)
	{
		YieldThread(10);

		if (config->push && pushReservation < config->numItems)
			if (config->queue->TryPush(config->items[pushReservation]))
			{
				pushReservation = (*config->pushReservation)++;
				++(*config->pushCounter);
			}

		if (config->pop)
		{
			queueIsEmpty = !config->queue->TryPop(&value);

			if (!queueIsEmpty)
				poppedItems[numPopped++] = value;
		}

		counter = config->pushCounter->load();
	}

	uint32 startTotalPoppedIndex = config->numAllPoppedItems->fetch_add(numPopped);

	for (uint32 i = 0; i < numPopped; ++i)
		config->allPoppedItems[startTotalPoppedIndex + i] = poppedItems[i];

	DefAlloc()->Free(poppedItems);

	return 0;
}

int main()
{
	// TODO: Get these from console input or at least print them when running
	const uint32 numPushers = 8;
	const uint32 numPoppers = 8;
	const uint32 numHybrids = 4;
	const uint32 queueSize = 2;
	const uint32 numItems = 256;

	static_assert(numItems < (uint32)-1, "numItems must be at least 1 less");

	ConcurrentQueue<uint32> queue(queueSize);
	std::atomic<uint32> pushCounter {0};
	std::atomic<uint32> pushReservation {0};
	uint32 items[numItems];
	uint32 allPoppedItems[numItems];
	std::atomic<uint32> numAllPoppedItems {0};
	Thread* workers[numPushers + numPoppers + numHybrids];
	uint32 numWorkers = 0;

	for (uint32 i = 0; i < numItems; ++i)
		items[i] = numItems - i;

	// TODO: Fix temp object ptr being passed as arg
	static WorkerConfig pusherConfig =
	{
		&queue,
		&pushCounter,
		&pushReservation,
		items,
		allPoppedItems,
		numItems,
		&numAllPoppedItems,
		true,
		false
	};
	static WorkerConfig popperConfig =
	{
		&queue,
		&pushCounter,
		&pushReservation,
		items,
		allPoppedItems,
		numItems,
		&numAllPoppedItems,
		false,
		true
	};
	static WorkerConfig hybridConfig =
	{
		&queue,
		&pushCounter,
		&pushReservation,
		items,
		allPoppedItems,
		numItems,
		&numAllPoppedItems,
		true,
		true
	};
	
	for (uint32 i = 0; i < numPushers; ++i)
		workers[numWorkers + i] = DefAlloc()->New<Thread>(&QueueWorker, &pusherConfig);

	numWorkers += numPushers;

	for (uint32 i = 0; i < numPoppers; ++i)
		workers[numWorkers + i] = DefAlloc()->New<Thread>(&QueueWorker, &popperConfig);

	numWorkers += numPoppers;

	for (uint32 i = 0; i < numHybrids; ++i)
		workers[numWorkers + i] = DefAlloc()->New<Thread>(&QueueWorker, &hybridConfig);

	numWorkers += numHybrids;

	for (uint32 i = 0; i < numWorkers; ++i)
		workers[i]->Join();

	bool itemIsUsed[numItems];

	for (uint32 i = 0; i < numItems; ++i)
		itemIsUsed[i] = false;

	for (uint32 i = 0; i < numItems; ++i)
	{
		uint32 matchedIndex = (uint32)-1;

		for (uint32 u = 0; u < numItems; ++u)
			if (!itemIsUsed[u] && items[i] == allPoppedItems[u])
			{
				matchedIndex = u;
				itemIsUsed[u] = true;
				break;
			}

		if (matchedIndex == (uint32)-1)
			throw std::runtime_error("ERROR: Item missing!");
	}

	std::cout << "Test completed successfully" << std::endl;

	_getch();

	return 0;
}