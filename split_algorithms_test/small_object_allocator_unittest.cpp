#include "stdafx.h"
#include "split_algorithms/small_object_allocator.h"

TEST(SmallObjectAllocator, basic)
{
	class SmallObject
	{
	public:
		int data;
		forceinline void prepareForReuse() {}
	};

	SmallObjectAllocator<SmallObject> o;
	SmallObject* p1 = o.allocObject();
	SmallObject* p2 = o.allocObject();
	EXPECT_EQ(p1 + 1, p2);	// p1 and p2 should be densely packed 
	o.freeObject(p1);
	SmallObject* p3 = o.allocObject();
	EXPECT_EQ(p1, p3);	// p1's memory is recycled.
}
