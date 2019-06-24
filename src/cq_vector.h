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

#include <new>
#include <forward_list>
#include "basic_types.h"
#include "cq_algorithm.h"

template <typename ValueType>
class Vector
{
public:
	forceinline size_t size() const { return m_size; }
	forceinline ValueType* cvector() const { return m_cvector; }

protected:
	ValueType* m_cvector;
	size_t m_size;
	size_t m_reservedSize;
	
public:
	Vector() {
		m_reservedSize = 0;
		this->m_size = 0;
		this->m_cvector = NULL;
	}

	void copy(const Vector& r)
	{
		reserve(r.m_size);
		if (std::is_pod<ValueType>::value)
		{
			memcpy(this->m_cvector, r.m_cvector, r.m_size * sizeof(ValueType));
		}
		else
		{
			size_t minSize = cq_min(this->m_size, r.m_size);
			for (size_t i = 0; i < minSize; i++)
			{
				this->m_cvector[i] = r.m_cvector[i];
			}
			if (this->m_size < r.m_size)
			{
				for (size_t i = minSize; i < r.m_size; i++)
				{
					new (this->m_cvector + i)ValueType(r.m_cvector[i]);
				}
			}
			else
			{
				for (size_t i = minSize; i < this->m_size; i++)
				{
					this->m_cvector[i].~ValueType();
				}
			}
		}

		this->m_size = r.m_size;
	}

	~Vector() {
		if (!std::is_pod<ValueType>::value)
		{
			for (size_t i = 0; i < this->m_size; i++)
			{
				this->m_cvector[i].~ValueType();
			}
		}
		free(this->m_cvector);
	}

	void swap(Vector& r)
	{
		cq_swap(size_t, this->m_size, r.m_size);
		cq_swap(size_t, m_reservedSize, r.m_reservedSize);
		cq_swap(ValueType*, this->m_cvector, r.m_cvector);
	}

	Vector(const Vector& r)
	{
		m_reservedSize = 0;
		this->m_size = 0;
		this->m_cvector = NULL;

		resize(r.m_size);

		if (std::is_pod<ValueType>::value)
		{
			memcpy(this->m_cvector, r.m_cvector, sizeof(ValueType) * r.m_size);
		}
		else
		{
			for (size_t i = 0; i < this->m_size; i++)
			{
				new (this->m_cvector + i)ValueType(r.m_cvector[i]);
			}
		}
	}

	Vector(Vector&& r)
	{
		memcpy(this, &r, sizeof(*this));
		r.m_cvector = NULL;
		r.m_size = 0;
	}

	forceinline void assignWithArray(const ValueType* v, size_t n) { this->clear(); this->appendArray(v, n); }

	void fillWithValue(size_t n, ValueType value)
	{
		clear();
		reserve(n);
		if (std::is_pod<ValueType>::value)
		{
			for (size_t i = 0; i != n; i++)
			{
				this->m_cvector[i] = value;
			}
		}
		else
		{
			for (size_t i = 0; i != n; i++)
			{
				new (this->m_cvector + i)ValueType(value);
			}
		}
		this->m_size = n;
	}

	forceinline bool empty() const {
		return this->m_size == 0;
	}

	forceinline ValueType* begin() const {
		return this->m_cvector;
	}

	forceinline ValueType* end() const {
		return this->m_cvector + this->m_size;
	}

	forceinline ValueType& back() const {
		return *(this->m_cvector + this->m_size - 1);
	}

	void popBack() {
		CQ_ASSERT(this->m_size != 0); 
		if (!std::is_pod<ValueType>::value)
			this->m_cvector[this->m_size - 1].~ValueType();
		this->m_size--;
	}

	forceinline ValueType& operator[] (size_t i) {
		return this->m_cvector[i];
	}

	forceinline const ValueType& operator[] (size_t i) const {
		return this->m_cvector[i];
	}

	forceinline ValueType& at(size_t i) {
		CQ_ASSERT(this->m_size > i);
		return this->m_cvector[i];
	}

	forceinline ValueType* atptr(size_t i) {
		return this->m_cvector + i;
	}

	void clear() {
		if (!std::is_pod<ValueType>::value){
			for (size_t i = 0; i < this->m_size; i++)
				this->m_cvector[i].~ValueType();
		}
		this->m_size = 0;
	}

	void append(const ValueType& obj) {
		if (this->m_size + 1 > m_reservedSize)
			reserve(this->m_size + 1);
		if (m_reservedSize > this->m_size)
		{
			if (std::is_pod<ValueType>::value)
				this->m_cvector[this->m_size] = obj;
			else
				new (this->m_cvector + this->m_size)ValueType(obj);
			this->m_size++;
		}
	}

	void appendArray(const ValueType* v, size_t n) {
		if (this->m_size + n > m_reservedSize)
			reserve(this->m_size + n);
		if (m_reservedSize > this->m_size)
		{
			ValueType* p = this->m_cvector + this->m_size;
			this->m_size += n;
			if (std::is_pod<ValueType>::value)
			{
				memcpy(p, v, n * sizeof(ValueType));
			}
			else
			{
				for (; n != 0u; n--)
				{
					new (p++)ValueType(*v++);
				}
			}
		}
	}

	void appendVector(Vector<ValueType>& v) {
		appendArray(&v[0], v.size());
	}

	void resize(size_t size) {
		reserve(size);

		if (size < this->m_size)
		{
			if (!std::is_pod<ValueType>::value)
			{
				for (size_t i = size; i < this->m_size; i++)
					this->m_cvector[i].~ValueType();
			}
		}
		else if (size > this->m_size)
		{
			if (!std::is_pod<ValueType>::value)
			{
				for (size_t i = size; i < this->m_size; i++)
					new (this->m_cvector + i)ValueType();
			}
		}

		this->m_size = size;
	}

	void reserve(size_t newSize) {
		uint32 index;
		uint32 mask;

		if (newSize <= m_reservedSize)
			return;

		if (newSize > 1)
			newSize--;

		mask = (uint32)(newSize << 1);
		if (cq_bitScanReverse(&index, mask))
			newSize = (size_t)(1) << index;
		else
			return;

		this->m_cvector = (ValueType*)realloc(this->m_cvector, newSize * sizeof(ValueType));
		if (this->m_cvector == NULL)
		{
			CQ_ASSERT(false && "Vector::reserve, realloc returns NULL");
			return;
		}
		m_reservedSize = newSize;
	}

	const ValueType* find(const ValueType& v)
	{
		for (const ValueType& item : *this)
		{
			if (item == v)
				return &item;
		}

		return NULL;
	}

	forceinline void sort()
	{
		cq::sort(this->m_cvector, this->m_cvector + this->m_size);
	}

	forceinline void sortWithComparator(bool(*cmp)(const ValueType& l, const ValueType& r))
	{
		cq::sortWithComparator(this->m_cvector, this->m_cvector + this->m_size, cmp);
	}

	forceinline void operator = (const Vector& r) {
		copy(r);
	}
};

class VectorChar : public Vector < char >
{
public:
	void appendString(const char *str)
	{
		size_t len = strlen(str);
		size_t oldSize = this->m_size;
		resize(oldSize + len);
		char* start = this->m_cvector + oldSize;
		memcpy(start, str, len * sizeof(char));
	}

	void appendEndOfString()
	{
		size_t oldSize = this->m_size;
		resize(oldSize + 1);
		char* end = this->m_cvector + oldSize;
		end[0] = L'\0';
	}

	forceinline void appendBuffer(void* p, size_t n) {
		this->appendArray((char*)p, n);
	}
};
