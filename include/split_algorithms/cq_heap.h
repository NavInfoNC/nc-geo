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

#include "cq_vector.h"

template<typename ValueType>
class Heap
{
private:
	Vector<ValueType> m_elements;
	bool(*m_cmp)(const ValueType* l, const ValueType* r);

	void _pushLastElement(ValueType* s, ValueType* e)
	{
		ValueType* i;
		ValueType* parent;

		CQ_ASSERT(s < e);

		i = e - 1;
		parent = s + (i - s - 1) / 2;

		while (m_cmp(parent, i))
		{
			cq::swap(*parent, *i);

			i = parent;
			parent = s + (i - s - 1) / 2;
		}
	}

public:
	forceinline Heap() { setAsMaxHeap(); }

	forceinline void setAsMinHeap() { m_cmp = cq::defaultLargerComparator<ValueType>; }
	forceinline void setAsMaxHeap() { m_cmp = cq::defaultLessComparator<ValueType>; }

	forceinline void initWithArray(const ValueType* array, size_t size) { m_elements.assignWithArray(array, size); rebuild(); }

	forceinline void initWithVector(const Vector<ValueType>* v) { initWithArray(v->begin(), v->size()); }

	forceinline void reserve(size_t size) { m_elements.reserve(size); }

	forceinline void copy(const Heap& r) { m_elements.copy(r.m_elements); m_cmp = r.m_cmp; }

	forceinline void swap(Heap& r) { cq_swap(Comparator, r.m_cmp, m_cmp); m_elements.swap(r.m_elements); }

	forceinline void push(const ValueType& v) { m_elements.append(v); _pushLastElement(m_elements.begin(), m_elements.end()); }

	void pop()
	{
		ValueType* s = m_elements.begin();
		ValueType* e = m_elements.end();
		ValueType* i;
		ValueType* left;
		ValueType* right;

		i = e - 1;
		cq::swap(*i, *s);

		i = s;
		for (;;)
		{
			left = s + (i - s) * 2 + 1;
			if (left >= e - 1)
				break;

			if (m_cmp(i, left))
			{
				right = left + 1;
				if (right < e - 1 && m_cmp(i, right))
				{
					if (m_cmp(left, right))
					{
						cq::swap(*i, *right);
						i = right;
					}
					else
					{
						cq::swap(*i, *left);
						i = left;
					}
				}
				else
				{
					cq::swap(*i, *left);
					i = left;
				}
			}
			else
			{
				right = left + 1;
				if (right < e - 1 && m_cmp(i, right))
				{
					cq::swap(*i, *right);
					i = right;
				}
				else
					break;
			}
		}

		m_elements.popBack();
	}

	forceinline const ValueType& peek() { return m_elements.at(0); }

	forceinline size_t size() const { return m_elements.size(); }

	void rebuild() 
	{
		ValueType* s = (ValueType*)m_elements.begin();
		ValueType* e = (ValueType*)m_elements.end();
		ValueType* i;
		for (i = s + 2; i <= e; i++)
		{
			_pushLastElement(s, i);
		}
	}

	forceinline void clear() { m_elements.clear(); }

	forceinline bool empty() const { return m_elements.size() == 0; }

	forceinline void setComparator(bool(*cmp)(const ValueType* l, const ValueType* r)) { this->m_cmp = cmp; }
};