#pragma once
#include <atomic>
#include "Core/Memory/IAllocator.h"

namespace ducklib
{
constexpr uint32 CACHE_LINE_SIZE = 128;

template <typename T>
class ConcurrentQueue
{
public:

	ConcurrentQueue(uint32 size, T* initialItems = nullptr, uint32 numInitialItems = 0);
	~ConcurrentQueue();
	
	uint32 TryPush(T item);
	uint32 TryPush(T* items, uint32 numItems);
	bool TryPop(T* item);

private:

	struct alignas(CACHE_LINE_SIZE) Slot
	{
		std::atomic<uint64> gen;
		T item;
	};

	alignas(CACHE_LINE_SIZE) std::atomic<uint64> head;
	alignas(CACHE_LINE_SIZE) std::atomic<uint64> tail;

	Slot* slots;
	const uint32 size;
};

template <typename T>
ConcurrentQueue<T>::ConcurrentQueue(uint32 size, T* initialItems, uint32 numInitialItems)
	: size(size)
{
	slots = DefAlloc()->Allocate<Slot>(size);

	for (uint32 i = 0; i < numInitialItems; ++i)
	{
		new(&slots[i]) Slot();
		slots[i].item = initialItems[i];
		slots[i].gen.store(0);
	}
	
	for (uint32 i = numInitialItems; i < size; ++i)
		slots[i].gen.store(1);
	
	head.store(0);
	tail.store(numInitialItems);
}

template <typename T>
ConcurrentQueue<T>::~ConcurrentQueue()
{
	DefAlloc()->Free(slots);
}

template <typename T>
uint32 ConcurrentQueue<T>::TryPush(T item)
{
	while (true)
	{
		uint64 cachedTail = tail.load();
		uint64 index = cachedTail % size;
		uint64 tailItemGen = slots[index].gen.load();
		uint64 tailGen = cachedTail / size;
		uint64 shiftedTailGen = tailGen << 1;

		if (shiftedTailGen + 1 == tailItemGen)
		{
			uint64 newTail = cachedTail + 1;

			if (tail.compare_exchange_strong(cachedTail, newTail))
			{
				Slot& queueSlot = slots[index];

				queueSlot.item = item;
				queueSlot.gen.store(shiftedTailGen, std::memory_order_release);

				return 1;
			}
		}
		else
		{
			uint64 oldCachedTail = cachedTail;
			cachedTail = tail.load();

			if (cachedTail == oldCachedTail)
				return 0;
		}
	}
}

template < typename T>
uint32 ConcurrentQueue<T>::TryPush(T* items, uint32 numItems)
{
	uint32 i = 0;
	
	while (i < numItems && TryPush(items[i]))
	{
		++i;
	}

	return i;
}

template <typename T>
bool ConcurrentQueue<T>::TryPop(T* item)
{
	while (true)
	{
		uint64 cachedHead = head.load();
		uint64 index = cachedHead % size;
		uint64 headItemGen = slots[index].gen;
		uint64 headGen = cachedHead / size;

		if (headGen << 1 == headItemGen)
		{
			uint64 newHead = cachedHead + 1;

			if (head.compare_exchange_strong(cachedHead, newHead))
			{
				Slot& queueSlot = slots[index];
				
				*item = queueSlot.item;
				queueSlot.gen.store(((headGen + 1) << 1) + 1, std::memory_order_release);

				return true;
			}
		}
		else
		{
			uint64 oldCachedHead = cachedHead;
			cachedHead = head.load();

			if (cachedHead == oldCachedHead)
				return false;
		}
	}
}
}