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
	forceinline bool defaultLessComparator(const MTYPE* l, const MTYPE* r)
	{
		return *l < *r;
	}

	template<typename MTYPE>
	forceinline bool defaultLargerComparator(const MTYPE* l, const MTYPE* r)
	{
		return *r < *l;
	}

	template<typename MTYPE>
	MTYPE* lowerBound(MTYPE* first, MTYPE* last, const MTYPE* value)
	{
		return lowerBoundWithComparator(first, last, value, defaultLessComparator);
	}

	template<typename MTYPE>
	MTYPE* lowerBoundWithComparator(MTYPE* first, MTYPE* last, const MTYPE* value, bool (*cmp)(const MTYPE* l, const MTYPE* r))
	{
		size_t count = last - first;
		MTYPE* middle;

		for (; count > 0;)
		{
			size_t count2 = count / 2;
			middle = first + count2;
			CQ_ASSERT(!cmp(middle, first));
			if (cmp(middle, value))
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
	MTYPE* upperBoundWithComparator(MTYPE* first, MTYPE* last, const MTYPE* value, bool (*cmp)(const MTYPE* l, const MTYPE* r))
	{
		MTYPE* middle;
		size_t count = last - first;

		for (; count > 0;)
		{
			size_t count2 = count / 2;
			middle = first + count2;

			if (!cmp(value, middle))
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
		return (first != last && !defaultLessComparator(value, first));
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
	MTYPE* uniqueWithComparator(MTYPE* s, MTYPE* e, bool(*cmp)(const MTYPE* l, const MTYPE* r))
	{
		MTYPE* p1;
		MTYPE* p2;
		if (s == e)
			return e;
		p1 = s + 1;
		p2 = s;
		while (p1 != e)
		{
			if (cmp(p2, p1) || cmp(p1, p2))
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
	void insertionSortWithComparator(MTYPE* first, MTYPE* last, bool(*cmp)(const MTYPE* l, const MTYPE* r))
	{
		if (first != last)
		{
			for (MTYPE* next = first; ++next != last;)
			{	// order next element
				MTYPE* next1 = next;
				MTYPE val;
				cq::move(val, *next);

				if (cmp(&val, first))
				{	// found new earliest element, move to front
					cq::arrayMove(++next1 - (next - first), first, next - first);
					cq::move(*first, val);
				}
				else
				{	// look for insertion point after first
					for (MTYPE* first1 = next1; cmp(&val, --first1); next1 = first1)
						cq::move(*next1, *first1);	// move hole down
					cq::move(*next1, val);	// insert element in hole
				}
			}
		}
	}

	template<typename MTYPE>
	void _heapSort(MTYPE* first, MTYPE* last, bool (*cmp)(const MTYPE* l, const MTYPE* r))
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

			while (cmp(parent, _i))
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

				if (cmp(i, left))
				{
					right = left + 1;
					if (right < e - 1 && cmp(i, right))
					{
						if (cmp(left, right))
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
					if (right < e - 1 && cmp(i, right))
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

	template <typename MTYPE>
	class SortImple
	{
	public:
		struct pair
		{
			MTYPE* first;
			MTYPE* second;
		};

		static void _Med3(MTYPE* _First, MTYPE* _Mid, MTYPE* _Last)
		{
			if (_Mid < _First)
				swap(*_Mid, *_First);
			if (_Last < _Mid)
				swap(*_Last, *_Mid);
			if (_Mid < _First)
				swap(*_Mid, *_First);
		}

		static void _Median(MTYPE* _First, MTYPE* _Mid, MTYPE* _Last)
		{
			if (40 < _Last - _First)
			{
				size_t _Step = (_Last - _First + 1) / 8;
				_Med3(_First, _First + _Step, _First + 2 * _Step);
				_Med3(_Mid - _Step, _Mid, _Mid + _Step);
				_Med3(_Last - 2 * _Step, _Last - _Step, _Last);
				_Med3(_First + _Step, _Mid, _Last - _Step);
			}
			else
				_Med3(_First, _Mid, _Last);
		}

		static pair _UnguardedPartition(MTYPE* _First, MTYPE* _Last, bool (*cmp)(const MTYPE* l, const MTYPE* r))
		{
			pair rtn;
			MTYPE* _Pfirst;
			MTYPE* _Plast;
			MTYPE* _Gfirst;
			MTYPE* _Glast;

			MTYPE* _Mid = _First + (_Last - _First) / 2;
			_Median(_First, _Mid, _Last - 1);
			_Pfirst = _Mid;
			_Plast = _Pfirst + 1;

			while (_First < _Pfirst
				&& !cmp((_Pfirst - 1), _Pfirst)
				&& !(cmp(_Pfirst, (_Pfirst - 1))))
				--_Pfirst;
			while (_Plast < _Last
				&& !cmp(_Plast, _Pfirst)
				&& !(cmp(_Pfirst, _Plast)))
				++_Plast;

			_Gfirst = _Plast;
			_Glast = _Pfirst;

			for (;;)
			{
				for (; _Gfirst < _Last; ++_Gfirst)
				{
					if (cmp(_Pfirst, _Gfirst))
						;
					else if (cmp(_Gfirst, _Pfirst))
						break;
					else
					{
						swap(*_Plast, *_Gfirst);
						_Plast++;
					}
				}
				for (; _First < _Glast; --_Glast)
				{
					if (cmp((_Glast - 1), _Pfirst))
						;
					else if (cmp(_Pfirst, (_Glast - 1)))
						break;
					else
					{
						_Pfirst--;
						swap(*_Pfirst, *(_Glast - 1));
					}
				}
				if (_Glast == _First && _Gfirst == _Last)	{

					rtn.first = _Pfirst;
					rtn.second = _Plast;
					return rtn;
				}

				if (_Glast == _First)
				{
					if (_Plast != _Gfirst)
						swap(*_Pfirst, *_Plast);
					++_Plast;
					swap(*_Pfirst, *_Gfirst);
					_Pfirst++;
					_Gfirst++;
				}
				else if (_Gfirst == _Last)
				{
					if (--_Glast != --_Pfirst)
						swap(*_Glast, *_Pfirst);
					_Plast--;
					swap(*_Pfirst, *_Plast);
				}
				else
				{
					--_Glast;
					swap(*_Gfirst, *_Glast);
					_Gfirst++;
				}
			}
		}

		static void sortImple(MTYPE* _First, MTYPE* _Last, size_t _Ideal, bool (*cmp)(const MTYPE* l, const MTYPE* r))
		{
			size_t _Count;
			for (; _CQ_ISORT_MAX < (_Count = _Last - _First) && 0 < _Ideal;)
			{

				pair _Mid = _UnguardedPartition(_First, _Last, cmp);
				_Ideal /= 2, _Ideal += _Ideal / 2;

				if (_Mid.first - _First < _Last - _Mid.second)
				{
					sortImple(_First, _Mid.first, _Ideal, cmp), _First = _Mid.second;
				}
				else
				{
					sortImple(_Mid.second, _Last, _Ideal, cmp), _Last = _Mid.first;
				}
			}

			if (_CQ_ISORT_MAX < _Count)
			{
				_heapSort(_First, _Last, cmp);
			}
			else if (1 < _Count)
			{
				insertionSortWithComparator<MTYPE>(_First, _Last, cmp);
			}
		}
	};

	template<typename MTYPE>
	forceinline void sort(MTYPE* first, MTYPE* last)
	{
		SortImple<MTYPE>::sortImple(first, last, last - first, defaultLessComparator);
	}

	template<typename MTYPE>
	forceinline void sortWithComparator(MTYPE* first, MTYPE* last, bool(*cmp)(const MTYPE* l, const MTYPE* r))
	{
		SortImple<MTYPE>::sortImple(first, last, last - first, cmp);
	}
}
