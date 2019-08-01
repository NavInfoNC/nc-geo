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

#include "basic_types.h"
#include "cq_vector.h"
 
template <typename T>
class SmallObjectAllocator
{
private:
	static const size_t m_poolSlotSize = 1024 * 4;	// 4k

	size_t m_objNum;
	Vector<T*> m_pool;	// one slot of the pool contains m_poolSlotSize objects.
	Vector<T*> m_freeList;

public:
	SmallObjectAllocator() : m_objNum(0)
	{
	}

	virtual ~SmallObjectAllocator()
	{
		freeAllObjects();
	}

	T* allocArray(size_t num)
	{
		if (!m_freeList.empty() && num == 1)
		{
			T* rtn = m_freeList.back();
			m_freeList.popBack();
			return rtn;
		}

		size_t slot = m_objNum / m_poolSlotSize;
		size_t index = m_objNum % m_poolSlotSize;

		// actualy num shoule < m_poolSlotSize / 3
		assert(num <= m_poolSlotSize);
		if ((m_objNum + num) >= m_pool.size() * m_poolSlotSize)
		{
			// push obj in slot not used to free list.
			if (!m_pool.empty())
			{
				m_objNum += m_poolSlotSize - index;
				while (index < m_poolSlotSize)
					m_freeList.append(&m_pool[slot][index++]);
			}

			m_pool.resize(m_pool.size() + 1);
			m_pool.back() = new T[m_poolSlotSize];
			slot = m_pool.size() - 1;
			index = 0;
		}

		m_objNum += num;
		return &m_pool[slot][index];
	}

	void freeArray(T* obj, size_t num)
	{
		// here has a bug, when a pointer be free twice, it will store double time in freeList, error when alloc.
		// to avoid this, we can use set..but it use more memory
		for (size_t i = 0; i < num; i++)
		{
			obj[i].prepareForReuse();
			m_freeList.append(obj + i);
		}
	}

	forceinline T* allocObject()
	{
		return allocArray(1);
	}

	forceinline void freeObject(T* obj)
	{
		return freeArray(obj, 1);
	}

	void freeAllObjects()
	{
		for(T* slot : m_pool)
		{
			delete[] slot;
		}
		m_pool.clear();
		m_freeList.clear();
		m_objNum = 0;
	}

	bool isPointerInFreeList(T *obj)
	{
		return find(m_freeList.begin(), m_freeList.end(), obj) != m_freeList.end();
	}

	size_t getDataNumber()
	{
		return m_objNum;
	}
};

/**
	this is suit to alloc memory but never want to free at running.
*/
template <typename T>
class FixedObjectAllocator
	: public SmallObjectAllocator<T>
{
public:
	typedef FixedObjectAllocator<T> Allocator;
	static Allocator* instance()
	{
		if (!m_instance)
		{
			m_instance = new Allocator;
		}
		return m_instance;
	}

private:
	void freeObj(T* obj, size_t num = 1);

	FixedObjectAllocator()
	{}

	static Allocator* m_instance;
};

template<typename T>
FixedObjectAllocator<T>* FixedObjectAllocator<T>::m_instance = NULL;

