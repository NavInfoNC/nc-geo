/*
MIT License

Copyright (c) 2019 GIS Core R&D Department, NavInfo Co., Ltd

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
