#include "gtest/gtest.h"
#include "cq_hashmap.h"

typedef ::Hashmap<size_t, size_t> SizeTHashmap;

TEST(Hashmap, findIteratorBug2)
{
	SizeTHashmap sm;
	SizeTHashmap::iterator iter;

	for (size_t i = 0; i < 20; ++i)
	{
		ASSERT_FALSE(sm.findIterator(i, &iter));
		sm.insertAt(i, i, &iter);

		EXPECT_TRUE(sm.findIterator(i, &iter));
		EXPECT_EQ(i, iter.value);
	}
}

struct MySize
{
	size_t x;
};

class MySizeHasher :public DefaultHasher<MySize>
{
public:
	static size_t hash(const MySize& t) {
		return t.x;
	}
	static bool equal(const MySize& l, const MySize& r) {
		return l.x == r.x;
	}
};

TEST(Hashmap, findIteratorBug3)
{
	Hashmap<MySize, size_t, MySizeHasher> sm;
	Hashmap<MySize, size_t, MySizeHasher>::iterator iter;

	MySize keys[2];
	keys[0].x = 1;
	keys[1].x = 17;
	for (size_t i = 0; i < 2; ++i)
	{
		EXPECT_FALSE(sm.findIterator(keys[i], &iter));
		sm.insertAt(keys[i], i, &iter);

		EXPECT_TRUE(sm.findIterator(keys[i], &iter));
		EXPECT_EQ(i, iter.value);
	}
}