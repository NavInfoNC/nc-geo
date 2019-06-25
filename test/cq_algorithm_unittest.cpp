#include "gtest/gtest.h"
#include "cq_algorithm.h"
#include "cq_vector.h"

TEST(Algorithm, sortPod)
{
	int a[4] = { 4, 3, 2, 1 };
	cq::sort(a, a + 4);

	for (size_t i = 1; i < element_of(a); i++)
	{
		EXPECT_TRUE(a[i - 1] < a[i]);
	}

	cq::sortWithComparator(a, a + 4, cq::defaultLargerComparator<int>);

	for (size_t i = 1; i < element_of(a); i++)
	{
		EXPECT_TRUE(a[i - 1] > a[i]);
	}
}

TEST(Algorithm, reverseNull)
{
	Vector<uint32> v;

	cq::reverse(v.begin(), v.end());
}
