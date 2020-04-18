#include <gtest/gtest.h>
#include "Memory/AllocTracker.h"

using namespace DuckLib::Internal::Memory;

TEST( TestAllocTest, Track )
{
	AllocTracker::Clear();
	AllocTracker::Track( (void*)0x1337, 420, "dummy.cpp", "DoDummy", 200 );
	const AllocTracker::Entry* entries = AllocTracker::GetEntries();
	uint32_t entryCount = AllocTracker::GetEntryCount();

	EXPECT_EQ( 1, entryCount );
	EXPECT_EQ( (void*)0x1337, entries[0].ptr );
	EXPECT_EQ( 420, entries[0].size );
	EXPECT_STREQ( "dummy.cpp", entries[0].file );
	EXPECT_STREQ( "DoDummy", entries[0].function );
	EXPECT_EQ( 200, entries[0].line );
}

TEST( TestAllocTest, TrackModify )
{
	AllocTracker::Clear();
	AllocTracker::Track( (void*)0x16, 16, "dummer.cpp", "DoDummer", 20 );
	AllocTracker::Modify( (void*)0x16, (void*)0x1337, 420, "dummy.cpp", "DoDummy", 200 );
	const AllocTracker::Entry* entries = AllocTracker::GetEntries();
	uint32_t entryCount = AllocTracker::GetEntryCount();

	EXPECT_EQ( 1, entryCount );
	EXPECT_EQ( (void*)0x1337, entries[0].ptr );
	EXPECT_EQ( 420, entries[0].size );
	EXPECT_STREQ( "dummy.cpp", entries[0].file );
	EXPECT_STREQ( "DoDummy", entries[0].function );
	EXPECT_EQ( 200, entries[0].line );
}

TEST( TestAllocTest, TrackDelete )
{
	AllocTracker::Clear();
	AllocTracker::Track( (void*)0x1337, 420, "dummy.cpp", "DoDummy", 200 );
	AllocTracker::Remove( (void*)0x1337 );
	uint32_t entryCount = AllocTracker::GetEntryCount();

	EXPECT_EQ( 0, entryCount );
}