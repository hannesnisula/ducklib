#pragma once
#include <atomic>
#include "../Core/Memory/IAlloc.h"

namespace DuckLib
{
template <typename T>
class ConcurrentQueue
{
public:

	ConcurrentQueue(uint32_t size, T* initialItems = nullptr, uint32_t numInitialItems = 0);
	
	bool TryPush(T item);
	bool TryPop(T* item);

private:

	static const uint32_t CACHE_LINE_SIZE = 128;

	struct alignas(CACHE_LINE_SIZE) Slot
	{
		std::atomic<uint64_t> gen;
		T item;
	};

	alignas(CACHE_LINE_SIZE) std::atomic<uint64_t> head;
	alignas(CACHE_LINE_SIZE) std::atomic<uint64_t> tail;

	Slot* slots;
	const uint32_t size;
};

template <typename T>
ConcurrentQueue<T>::ConcurrentQueue(uint32_t size, T* initialItems, uint32_t numInitialItems)
	: size(size)
{
	slots = DL_NEW_ARRAY(DefAlloc(), Slot, size);

	for (uint32_t i = 0; i < numInitialItems; ++i)
	{
		slots[i].item = initialItems[i];
		slots[i].gen.store(0);
	}
	
	for (uint32_t i = numInitialItems; i < size; ++i)
		slots[i].gen.store(1);
	
	head.store(0);
	tail.store(numInitialItems);
}

template <typename T>
bool ConcurrentQueue<T>::TryPush(T item)
{
	while (true)
	{
		uint64_t cachedTail = tail.load();
		uint64_t index = cachedTail % size;
		uint64_t tailItemGen = slots[index].gen.load();
		uint64_t tailGen = cachedTail / size;
		uint64_t shiftedTailGen = tailGen << 1;

		if (shiftedTailGen + 1 == tailItemGen)
		{
			uint64_t newTail = cachedTail + 1;

			if (tail.compare_exchange_strong(cachedTail, newTail))
			{
				Slot& queueSlot = slots[index];

				queueSlot.item = item;
				queueSlot.gen.store(shiftedTailGen, std::memory_order_release);

				return true;
			}
		}
		else
		{
			uint64_t oldCachedTail = cachedTail;
			cachedTail = tail.load();

			if (cachedTail == oldCachedTail)
				return false;
		}
	}
}

template <typename T>
bool ConcurrentQueue<T>::TryPop(T* item)
{
	while (true)
	{
		uint64_t cachedHead = head.load();
		uint64_t index = cachedHead % size;
		uint64_t headItemGen = slots[index].gen;
		uint64_t headGen = cachedHead / size;

		if (headGen << 1 == headItemGen)
		{
			uint64_t newHead = cachedHead + 1;

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
			uint64_t oldCachedHead = cachedHead;
			cachedHead = head.load();

			if (cachedHead == oldCachedHead)
				return false;
		}
	}
}
}