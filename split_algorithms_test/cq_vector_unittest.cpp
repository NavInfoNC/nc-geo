#include "stdafx.h"
#include "split_algorithms/cq_algorithm.h"
#include "split_algorithms/cq_vector.h"

TEST(Vector, basic)
{
	Vector<int> sv;

	// append
	sv.append(1);
	sv.append(2);
	EXPECT_EQ(sv.size(), 2);
	EXPECT_EQ(sv[0], 1);
	EXPECT_EQ(sv[1], 2);

	// copy constructor
	Vector<int> sv2(sv);
	EXPECT_EQ(sv2.size(), 2);
	EXPECT_EQ(sv2[0], 1);
	EXPECT_EQ(sv2[1], 2);

	// swap
	sv2.append(3);
	sv.swap(sv2);

	EXPECT_EQ(sv.size(), 3);
	EXPECT_EQ(sv2.size(), 2);
}

TEST(Vector, iterate)
{
	Vector<int> sv;
	sv.append(1);
	sv.append(2);
	sv.append(3);

	const int results[] = { 1, 2, 3 };
	size_t count = 0;
	for(int a : sv)
	{
		EXPECT_EQ(results[count++], a);
	}
}

TEST(Vector, assign)
{
	Vector<int> ivec;
	ivec.append(5);
	ivec.append(6);
	EXPECT_EQ(ivec.size(), 2);
	EXPECT_EQ(ivec[0], 5);
	EXPECT_EQ(ivec[1], 6);

	ivec.fillWithValue(4, 8);
	EXPECT_EQ(ivec.size(), 4);
	EXPECT_EQ(ivec[0], 8);
	EXPECT_EQ(ivec[1], 8);
	EXPECT_EQ(ivec[2], 8);
	EXPECT_EQ(ivec[3], 8);
}

TEST(Vector, sort)
{
	Vector < int > o;
	o.append(4);
	o.append(3);
	o.append(2);
	o.append(1);

	o.sort();

	for (size_t i = 1; i < o.size(); i++)
	{
		EXPECT_TRUE(o[i - 1] < o[i]);
	}

	o.sortWithComparator(cq::defaultLargerComparator<int>);
	for (size_t i = 1; i < o.size(); i++)
	{
		EXPECT_TRUE(o[i - 1] > o[i]);
	}
}

TEST(Vector, bitScaneReverse)
{
	uint32 x = 0x1;
	uint32 index = UINT_MAX;
	EXPECT_TRUE(cq_bitScanReverse(&index, x));
	EXPECT_EQ(index, 0);

	x = 0x80000000;
	EXPECT_TRUE(cq_bitScanReverse(&index, x));
	EXPECT_EQ(index, 31);

	x = 0x2c190000;
	EXPECT_TRUE(cq_bitScanReverse(&index, x));
	EXPECT_EQ(index, 29);

	x = 0;
	EXPECT_FALSE(cq_bitScanReverse(&index, x));
}
