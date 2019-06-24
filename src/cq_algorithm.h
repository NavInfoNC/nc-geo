/*
MIT License

Copyright (c) 2019 NavInfo's NaviCore Department

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#pragma once

#include <algorithm>
#include "basic_types.h"

namespace cq
{
#ifndef _CQ_ISORT_MAX
#	define _CQ_ISORT_MAX 32
#endif

	template<typename T>
	__inline void swap(T& l, T& r)
	{
		T tmp(l);
		l = r;
		r = tmp;
	}

	template<typename T>
	static forceinline void reverse(T* begin, T* end)
	{
		for (; begin != end && begin != --end; ++begin)
		{
			swap(*begin, *end);
		}
	}

	template<typename T>
	forceinline T* arrayMove(T* dest, T* src, size_t count)
	{
		memmove(dest, src, count * sizeof(T));
		return dest;
	}

	template<typename T>
	forceinline void move(T& dest, T& src)
	{
		static_assert(std::is_pod<T>::value, "Non-pod-type must implement cq::move.");
		dest = src;
	}
	
	template<typename MTYPE>
	forceinline bool defaultLessComparator(const MTYPE& l, const MTYPE& r)
	{
		return l < r;
	}

	template<typename MTYPE>
	forceinline bool defaultLargerComparator(const MTYPE& l, const MTYPE& r)
	{
		return r < l;
	}

	template<typename MTYPE>
	MTYPE* lowerBound(MTYPE* first, MTYPE* last, const MTYPE* value)
	{
		return lowerBoundWithComparator(first, last, value, defaultLessComparator);
	}

	template<typename MTYPE>
	MTYPE* lowerBoundWithComparator(MTYPE* first, MTYPE* last, const MTYPE* value, bool (*cmp)(const MTYPE& l, const MTYPE& r))
	{
		size_t count = last - first;
		MTYPE* middle;

		for (; count > 0;)
		{
			size_t count2 = count / 2;
			middle = first + count2;
			CQ_ASSERT(!cmp(*middle, *first));
			if (cmp(*middle, *value))
			{
				first = ++middle, count -= count2 + 1;
			}
			else
				count = count2;
		}

		return first;
	}

	template<typename MTYPE>
	MTYPE* upperBound(MTYPE* first, MTYPE* last, const MTYPE* value)
	{
		return upperBoundWithComparator(first, last, value, defaultLessComparator);
	}

	template<typename MTYPE>
	MTYPE* upperBoundWithComparator(MTYPE* first, MTYPE* last, const MTYPE* value, bool (*cmp)(const MTYPE& l, const MTYPE& r))
	{
		MTYPE* middle;
		size_t count = last - first;

		for (; count > 0;)
		{
			size_t count2 = count / 2;
			middle = first + count2;

			if (!cmp(*value, *middle))
			{
				first = ++middle, count -= count2 + 1;
			}
			else
				count = count2;
		}

		return first;
	}

	template<typename MTYPE>
	bool binarySearch(MTYPE* first, MTYPE* last, const MTYPE* value)
	{
		first = lowerBound(first, last, value);
		return (first != last && !defaultLessComparator(*value, *first));
	}

	template<typename MTYPE>
	bool binarySearchWithComparator(MTYPE* first, MTYPE* last, const MTYPE* value, bool (*cmp)(const MTYPE* l, const MTYPE* r))
	{
		first = lowerBoundWithComparator(first, last, value, cmp);
		return (first != last && !cmp(value, first));
	}

	template<typename MTYPE>
	MTYPE* unique(MTYPE* s, MTYPE* e)
	{
		return uniqueWithComparator(s, e, defaultLessComparator);
	}

	template<typename MTYPE>
	MTYPE* uniqueWithComparator(MTYPE* s, MTYPE* e, bool(*cmp)(const MTYPE& l, const MTYPE& r))
	{
		MTYPE* p1;
		MTYPE* p2;
		if (s == e)
			return e;
		p1 = s + 1;
		p2 = s;
		while (p1 != e)
		{
			if (cmp(*p2, *p1) || cmp(*p1, *p2))
			{
				p2++;
				*p2 = *p1;
			}
			p1++;
		}
		return p2 + 1;
	}

	template<typename MTYPE>
	forceinline void insertionSort(MTYPE* first, MTYPE* last)
	{
		insertionSortWithComparator(first, last, defaultLessComparator);
	}

	template <typename MTYPE>
	void insertionSortWithComparator(MTYPE* first, MTYPE* last, bool(*cmp)(const MTYPE& l, const MTYPE& r))
	{
		if (first != last)
		{
			for (MTYPE* next = first; ++next != last;)
			{	// order next element
				MTYPE* next1 = next;
				MTYPE val;
				cq::move(val, *next);

				if (cmp(val, *first))
				{	// found new earliest element, move to front
					cq::arrayMove(++next1 - (next - first), first, next - first);
					cq::move(*first, val);
				}
				else
				{	// look for insertion point after first
					for (MTYPE* first1 = next1; cmp(val, *(--first1)); next1 = first1)
						cq::move(*next1, *first1);	// move hole down
					cq::move(*next1, val);	// insert element in hole
				}
			}
		}
	}

	template<typename MTYPE>
	void _heapSort(MTYPE* first, MTYPE* last, bool (*cmp)(const MTYPE& l, const MTYPE& r))
	{
		MTYPE* i;
		for (i = first + 2; i <= last; i++)
		{
			MTYPE* s = first;
			MTYPE* e = i;
			MTYPE* _i;
			MTYPE* parent;

			CQ_ASSERT(s < e);

			_i = e - 1;
			parent = s + (_i - s - 1) / 2;

			while (cmp(*parent, *_i))
			{
				swap(*_i, *parent);

				_i = parent;
				parent = s + (_i - s - 1) / 2;
			}
		}

		for (; last - first > 1; --last)
		{
			MTYPE* s = first;
			MTYPE* e = last;
			MTYPE* left;
			MTYPE* right;

			i = e - 1;
			swap(*i, *s);

			i = s;
			for (;;)
			{
				left = s + (i - s) * 2 + 1;
				if (left >= e - 1)
					break;

				if (cmp(*i, *left))
				{
					right = left + 1;
					if (right < e - 1 && cmp(*i, *right))
					{
						if (cmp(*left, *right))
						{
							swap(*i, *right);
							i = right;
						}
						else
						{
							swap(*i, *left);
							i = left;
						}
					}
					else
					{
						swap(*i, *left);
						i = left;
					}
				}
				else
				{
					right = left + 1;
					if (right < e - 1 && cmp(*i, *right))
					{
						swap(*i, *right);
						i = right;
					}
					else
						break;
				}
			}
		}
	}

	template<typename MTYPE>
	forceinline void sort(MTYPE* first, MTYPE* last)
	{
		typedef bool(*CompFunc)(const MTYPE& l, const MTYPE& r);
		std::sort<MTYPE*, CompFunc>(first, last, defaultLessComparator);
	}

	template<typename MTYPE>
	forceinline void sortWithComparator(MTYPE* first, MTYPE* last, bool(*cmp)(const MTYPE& l, const MTYPE& r))
	{
		typedef bool(*CompFunc)(const MTYPE& l, const MTYPE& r);
		std::sort<MTYPE*, CompFunc>(first, last, cmp);
	}
}
