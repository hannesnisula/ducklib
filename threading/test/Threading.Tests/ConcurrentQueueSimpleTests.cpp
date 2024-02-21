#include <gtest/gtest.h>
#include "Threading/ConcurrentQueue.h"
#include "Threading/Thread.h"

using namespace ducklib;

constexpr uint32 pushValue = 420;

uint32 PushThread(void* data)
{
	ConcurrentQueue<uint32>* queue = (ConcurrentQueue<uint32>*)data;

	queue->TryPush(pushValue);

	return 0;
}

TEST(ConcurrentQueueTest, TryPushSingle)
{
	ConcurrentQueue<uint32> queue(128);
	uint32 result;

	queue.TryPush(200);
	EXPECT_TRUE(queue.TryPop(&result));
	EXPECT_TRUE(result == 200);
}

TEST(ConcurrentQueueTest, TryPushMultiple)
{
	ConcurrentQueue<uint32> queue(128);
	uint32 result;

	queue.TryPush(200);
	queue.TryPush(100);
	queue.TryPush(69);
	queue.TryPush(420);
	queue.TryPush(1337);

	EXPECT_TRUE(queue.TryPop(&result));
	EXPECT_TRUE(result == 200);
	EXPECT_TRUE(queue.TryPop(&result));
	EXPECT_TRUE(result == 100);
	EXPECT_TRUE(queue.TryPop(&result));
	EXPECT_TRUE(result == 69);
	EXPECT_TRUE(queue.TryPop(&result));
	EXPECT_TRUE(result == 420);
	EXPECT_TRUE(queue.TryPop(&result));
	EXPECT_TRUE(result == 1337);
}

TEST(ConcurrentQueueTest, TryPopEmpty)
{
	ConcurrentQueue<uint32> queue(128);
	uint32 result;

	EXPECT_FALSE(queue.TryPop(&result));
}

TEST(ConcurrentQueueTest, TryPushFull)
{
	ConcurrentQueue<uint32> queue(2);

	EXPECT_TRUE(queue.TryPush(1));
	EXPECT_TRUE(queue.TryPush(2));
	EXPECT_FALSE(queue.TryPush(3));
}

TEST(ConcurrentQueueTest, TryPopAfterPushFull)
{
	ConcurrentQueue<uint32> queue(2);
	uint32 result {};

	EXPECT_TRUE(queue.TryPush(1));
	EXPECT_TRUE(queue.TryPush(2));
	EXPECT_FALSE(queue.TryPush(3));

	EXPECT_TRUE(queue.TryPop(&result));
	EXPECT_EQ(1, result);
	EXPECT_TRUE(queue.TryPop(&result));
	EXPECT_EQ(2, result);
	EXPECT_FALSE(queue.TryPop(&result));
}