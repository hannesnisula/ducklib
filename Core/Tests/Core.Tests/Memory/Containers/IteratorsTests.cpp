#include <gtest/gtest.h>
#include "Core/Memory/Containers/Iterators.h"

using namespace DuckLib;

class Foo
{
public:

	uint32_t v1;
	uint32_t v2;
	uint32_t* p;
};

// TODO: Try to find way to replicate the allocation issue caused here and put it in HeapAllocatorTests.cpp
TEST(IteratorsTest, TArrayRangedLoop)
{
	// TODO: Support for braced initialization list would be handy here
	TArray<uint32_t> a;

	for (uint32_t i = 0; i < 10; ++i)
		a.Append(i);

	uint32_t n = 0;

	for (uint32_t i : a)
		EXPECT_EQ(n++, i);
}
