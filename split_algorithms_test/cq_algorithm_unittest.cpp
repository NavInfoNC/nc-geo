#include "stdafx.h"
#include "split_algorithms/cq_algorithm.h"
#include "split_algorithms/cq_heap.h"

TEST(Algorithm, binarySearch)
{
	int a[] = { 1, 2, 3, 3, 5, 10 };
	int* begin = a;
	int* end = &a[element_of(a)];

	int value;
	int* result;

	value = 1;
	result = cq::lowerBound(begin, end, &value);
	EXPECT_EQ(&a[0], result);
	result = cq::upperBound(begin, end, &value);
	EXPECT_EQ(&a[1], result);
	EXPECT_TRUE(cq::binarySearch(begin, end, &value));

	value = 3;
	result = cq::lowerBound(begin, end, &value);
	EXPECT_EQ(&a[2], result);
	result = cq::upperBound(begin, end, &value);
	EXPECT_EQ(&a[4], result);
	EXPECT_TRUE(cq::binarySearch(begin, end, &value));

	value = 5;
	result = cq::lowerBound(begin, end, &value);
	EXPECT_EQ(&a[4], result);
	result = cq::upperBound(begin, end, &value);
	EXPECT_EQ(&a[5], result);
	EXPECT_TRUE(cq::binarySearch(begin, end, &value));

	value = 15;
	result = cq::lowerBound(begin, end, &value);
	EXPECT_EQ(end, result);
	result = cq::upperBound(begin, end, &value);
	EXPECT_EQ(end, result);
	EXPECT_FALSE(cq::binarySearch(begin, end, &value));

	value = 7;
	result = cq::lowerBound(begin, end, &value);
	EXPECT_EQ(&a[5], result);
	result = cq::upperBound(begin, end, &value);
	EXPECT_EQ(&a[5], result);
	EXPECT_FALSE(cq::binarySearch(begin, end, &value));
}

TEST(Algorithm, insertionSort)
{
	int a[] = { 4, 6, 3, 7, 0, 5, 4, 2, 6 };
	cq::insertionSort(a, a + element_of(a));
	for (size_t i = 0; i < element_of(a) - 1; ++i)
	{
		EXPECT_LE(a[i], a[i + 1]);
	}
	cq::insertionSortWithComparator(a, a + element_of(a), cq::defaultLargerComparator<int>);
	for (size_t i = 0; i < element_of(a) - 1; ++i)
	{
		EXPECT_GE(a[i], a[i + 1]);
	}
}

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

TEST(Algorithm, heapArray)
{
	int a[] = { 4, 6, 3, 7, 0, 5, 4, 2, 6 };
	Heap<int> heap;
	heap.initWithArray(a, element_of(a));
	EXPECT_EQ(element_of(a), heap.size());
	EXPECT_EQ(7, heap.peek());
	heap.pop();
	EXPECT_EQ(6, heap.peek());
	heap.pop();
	EXPECT_EQ(6, heap.peek());
	heap.push(9);
	EXPECT_EQ(9, heap.peek());
	heap.clear();
	EXPECT_EQ(0, heap.size());
}

typedef struct HeapStruct
{
	int otherVal;
	int cmpVal;
} HeapStruct;

bool heapStructComp(HeapStruct* const * l, HeapStruct* const * r)
{
	return (*l)->cmpVal < (*r)->cmpVal;
}

TEST(Algorithm, heapComplexObject)
{
	HeapStruct hs[5];
	Heap<HeapStruct*> heap;
	heap.setComparator(heapStructComp);
	for (int i = 0; i < element_of(hs); ++i)
	{
		hs[i].cmpVal = i;
		hs[i].otherVal = element_of(hs) - i - 1;
		heap.push(&hs[i]);
	}

	EXPECT_EQ(4, heap.peek()->cmpVal);
	EXPECT_EQ(0, heap.peek()->otherVal);
	heap.pop();
	EXPECT_EQ(3, heap.peek()->cmpVal);
	EXPECT_EQ(1, heap.peek()->otherVal);
	hs[2].cmpVal = 9;
	heap.rebuild();
	EXPECT_EQ(9, heap.peek()->cmpVal);
	EXPECT_EQ(2, heap.peek()->otherVal);
}

TEST(Algorithm, unique)
{
	uint8 v[] = { 1, 2, 2, 3, 3, 3, 4, 5, 6, 6 };
	uint8 result[] = { 1, 2, 3, 4, 5, 6 };
	size_t n = cq::unique(v, v + element_of(v)) - v;
	EXPECT_TRUE(memcmp(v, result, n) == 0);
}

TEST(Algorithm, reverseNull)
{
	Vector<uint32> v;

	cq::reverse(v.begin(), v.end());
}
