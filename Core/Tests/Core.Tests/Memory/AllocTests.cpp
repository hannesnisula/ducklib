#include "gtest/gtest.h"
#include <Core/Memory/HeapAllocator.h>

class Foo
{
public:

	Foo()
	{
		v1 = 16;
		v2 = 69;
	}

	~Foo()
	{
		*p = 32;
	}

	uint32_t v1;
	uint32_t v2;
	uint32_t* p;
};

TEST(AllocTest, NewDelete)
{
	DuckLib::HeapAllocator alloc;
	uint32_t v = 2;
	Foo* foo = alloc.New<Foo>();
	foo->p = &v;

	EXPECT_EQ(16, foo->v1);
	EXPECT_EQ(69, foo->v2);

	alloc.Delete(foo);

	EXPECT_EQ(32, v);
}

TEST(AllocTest, NewDeleteArray)
{
	DuckLib::HeapAllocator alloc;
	uint32_t v = 2;
	constexpr uint32_t arraySize = 32;
	Foo* foo = alloc.Allocate<Foo>(arraySize);

	for (uint32_t i = 0; i < arraySize; ++i)
		new(&foo[i]) Foo();

	for (uint32_t i = 0; i < arraySize; ++i)
		foo[i].p = &v;

	for (uint32_t i = 0; i < arraySize; ++i)
	{
		EXPECT_EQ(16, foo->v1);
		EXPECT_EQ(69, foo->v2);
	}

	for (uint32_t i = 0; i < arraySize; ++i)
		foo[i].~Foo();

	alloc.Free(foo);

	EXPECT_EQ(32, v);
}
