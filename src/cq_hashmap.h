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

#undef INCREASE
#undef DECREASE

#define INCREASE(i) i++; if (i == m_tableSize) i = 0
#define DECREASE(i) if (i == 0) i = m_tableSize - 1; else i--
#define IN_USE(i) ((m_inUse)[(i) / 8] & (1 << ((i) % 8)))
#define SET_IN_USE(i) m_inUse[(i) / 8] |= 1 << ((i) % 8)
#define SET_NOT_USE(i) m_inUse[(i) / 8] &= ~(1 << ((i) % 8))


forceinline size_t Math_hashSizeT(size_t x)
{
	x = ((x >> 16) ^ x) * 0x45d9f3b;
	x = ((x >> 16) ^ x) * 0x45d9f3b;
	x = (x >> 16) ^ x;
	return x;
}

	template<typename T>
	class DefaultHasher
	{
	public:
		static size_t hash(const T& t) {
			return Math_hashSizeT((size_t)t);
		}
		static bool equal(const T& l, const T& r) {
			return l == r;
		}
	};

	/**
		A hash map container.

		Unlike STL, which is bloated, slow and hard to use, this implementation of
		hash map is designed with both efficiency and ease of use in mind. But to achieve 
		that goal. There are one limitation. It can only use POD as key and value.

		There are 2 setters. set() and insert().
		There are 2 getters. get() and find().
		There are 2 delete functions. remove() and erase().
		These functions behave just like STL's hash_map.

		The iterator is dramatically different from STL.
		@code
		Hashmap<size_t, int>::iterator iter(&map);
		while(iter.next())
		{
			if (iter.key == 1 or iter.value == 1)
			{
				map.erase(&iter);	// you can delete some items during iteration.
			}
		}
		@endcode

		There are two ways to use a customized type as the KeyType.
		1. Add a converter function.
			struct Point {
				int x, y;
				operator size_t() const {
					return x + y;
				}
			};
			Hashmap<Point, size_t> map;
		2. Define a hasher class. This is very useful if method one is not possible or convenient.
			class StringHasher
			{
			public:
				static size_t hash(char* str) {
					size_t rtn = 0;
					size_t i = 0;
					for(; *str; str++)
					{
						rtn += *str << ((i & 3) << 3);
						i++;
					}
					return Math_hashSizeT(rtn);
				}
				static bool equal(char* l, char* r)	{
					return cq_strcmp(l, r) == 0;
				}
			};
			Hashmap<char*, size_t, StringHasher> map;
	*/
	template<typename KeyType, typename ValueType, typename Hasher = DefaultHasher<KeyType> >
	class Hashmap
	{
	private:
		enum Error{
			Error_missing = -3,  /* No such element */
			Error_full = -2, 	/* Hashmap is full */
			Error_omem = -1, 	/* Out of Memory */
			Error_ok =  0 	/* OK */
		};

		struct element{
			KeyType key;
			ValueType value;
		};

	public:
		class iterator;
		friend class iterator;

		class iterator
		{
			friend class Hashmap;
		private:
			size_t m_tableSize;
			const Vector<element>* m_data;	// if m_data == NULL, it can only be used for deletion
			const char* m_inUse;
			size_t m_i, m_end;

		public:
			const KeyType& key;
			const ValueType& value;

			iterator& operator = (iterator& r) {
				m_tableSize = r.m_tableSize;
				m_data = r.m_data;
				m_inUse = r.m_inUse;
				m_i = r.m_i;
				m_end = r.m_end;
			}

			iterator() : key(*((KeyType*)NULL)), value(*((ValueType*)NULL))
			{
				reset(NULL);
			}

			iterator(const Hashmap* map) : key(*((KeyType*)NULL)), value(*((ValueType*)NULL))
			{
				reset(map);
			}

			void reset(const Hashmap* map)
			{
				size_t i;

				if (map == NULL)
				{
					m_tableSize = 0;
					m_i = (size_t)-1;
					m_end = 0;
					return;
				}

				m_tableSize = map->m_tableSize;
				m_data = &map->m_data;
				m_inUse = map->m_inUse;

				if (map->m_size == 0)
				{
					m_i = (size_t)-1;
					m_end = 0;
					return;
				}

				// find the first object with a hole before it.
				for(i = 0; i < m_tableSize; i++)
				{
					// find first hole
					if (!IN_USE(i))
					{
						// pass the hole
						for(; i < m_tableSize; i++)
						{
							if (IN_USE(i))
								break;
						}
						break;
					}
				}
				if (i == m_tableSize)
					i = 0;
				m_end = m_i = i;
				DECREASE(m_i);
				DECREASE(m_end);
				_setKeyAndValueAtIndex(m_data, i);
			}

			bool next() 
			{
				CQ_ASSERT(m_data != NULL);	// some iterators are for deletion only and cannot be used to iterate all members.

				INCREASE(m_i);

				while(m_i != m_end && !IN_USE(m_i))
				{
					INCREASE(m_i);
				};

				if (m_i != m_end)
				{
					_setKeyAndValueAtIndex(m_data, m_i);
					return true;
				}
				else
				{
					return false;
				}
			}

			forceinline void _setKeyAndValueAtIndex(const Vector<element>* data, size_t index)
			{
				size_t* ppkey = &this->m_end + 1;
				size_t* ppvalue = &this->m_end + 2;
				*ppkey = (size_t)(&data->cvector()[index].key);
				*ppvalue = (size_t)(&data->cvector()[index].value);
			}
		};

	public:
		Hashmap() {
			_reserve(16);
		}

		Hashmap(size_t bucketSize) {
			_reserve(bucketSize);
		}

		~Hashmap() {
			if (!std::is_pod<KeyType>::value || !std::is_pod<ValueType>::value)
			{
				for (size_t i = 0; i < m_tableSize; i++) 
				{
					if (IN_USE(i)) 
					{
						m_data[i].key.~KeyType();
						m_data[i].value.~ValueType();
					}
				}
			}

			free(m_inUse);
		}

		forceinline iterator getIterator() const {
			iterator iter(this);
			return iter;
		}

		void clear() {
			memset(m_inUse, 0, (m_tableSize + 7) / 8);
			m_size = 0;
		}

		bool empty() const {
			return m_size == 0;
		}

		size_t size() const {
			return m_size;
		}

		size_t bucketSize() const {
			return m_tableSize;
		}

		void set(const KeyType& key, const ValueType& value)
		{
			size_t index = 0;
			Error err = _findInsertPosition(key, &index);

			if (err != Error_ok)
				return;

			/* Set the data */
			if (!IN_USE(index))
			{
				m_size++;
				if (!std::is_pod<KeyType>::value)
				{
					new (&m_data[index].key)KeyType(key);
				}
				else
				{
					m_data[index].key = key;
				}

				if (!std::is_pod<ValueType>::value)
				{
					new (&m_data[index].value)ValueType(value);
				}
				else
				{
					m_data[index].value = value;
				}
			}
			else
			{
				m_data[index].key = key;
				m_data[index].value = value;
			}

			SET_IN_USE(index);
		}

		void setWithIterator(const iterator* iter, const ValueType& value)
		{
			size_t index = iter->m_i;
			CQ_ASSERT(IN_USE(index));
			m_data[index].value = value;
		}

		forceinline bool insert(const KeyType& key, const ValueType& value)
		{
			return insertAndReturnIteratorIfFailed(key, value, NULL);
		}

		bool insertAndReturnIteratorIfFailed(const KeyType& key, const ValueType& value, iterator* iter)
		{
			size_t index = 0;
			Error err = _findInsertPosition(key, &index);

			if (err != Error_ok || IN_USE(index))
			{
				if (err == Error_ok && iter != NULL)
				{
					iter->m_data = NULL;
					iter->m_i = index;
					iter->_setKeyAndValueAtIndex(&this->m_data, index);
				}
				return false;
			}

			_insertAtIndex(key, value, index);
			SET_IN_USE(index);

			return true;
		}

		/**
			It's faster than insert(), when findIterator() returns false.
		*/
		forceinline void insertAt(const KeyType& key, const ValueType& value, iterator* iter)
		{
			_insertAtIndex(key, value, iter->m_i);
		}

		void _insertAtIndex(const KeyType& key, const ValueType& value, size_t index)
		{
			/* If 3/4 full, rehash and use normal insertion*/
			if (m_size + 1 > (m_tableSize >> 1) + (m_tableSize >> 2))
			{
				insert(key, value);
				return;
			}

			m_size++;
			if (!std::is_pod<KeyType>::value)
			{
				new (&m_data[index].key)KeyType(key);
			}
			else
			{
				m_data[index].key = key;
			}

			if (!std::is_pod<ValueType>::value)
			{
				new (&m_data[index].value)ValueType(value);
			}
			else
			{
				m_data[index].value = value;
			}
			SET_IN_USE(index);
		}

		bool getValue(const KeyType& key, ValueType& value) const
		{
			size_t curr;
			if (_find(key, &curr))
			{
				value = m_data[curr].value;
				return true;
			}
			else
			{
				return false;
			}
		}

		bool get(const KeyType& key, ValueType** value)
		{
			size_t curr;
			if (_find(key, &curr))
			{
				*value = &m_data[curr].value;
				return true;
			}
			else
			{
				return false;
			}
		}

		forceinline bool find(const KeyType& key) const {
			size_t curr;
			return _find(key, &curr);
		}

		/**
			@param iter		
				1. When return true, it can only be used in erase() or for reading of old value.
				2. When return false, it can only be used in insertAt().
		*/
		bool findIterator(const KeyType& key, iterator* iter)
		{
			size_t curr;
			if (_find(key, &curr))
			{
				iter->m_data = NULL;
				iter->m_i = curr;
				iter->_setKeyAndValueAtIndex(&this->m_data, curr);
				return true;
			}
			else
			{
				iter->m_i = curr;
				return false;
			}
		}

		bool remove(const KeyType& key)
		{
			size_t curr;
			if (_find(key, &curr))
			{
				iterator iter;
				iter.m_i = curr;
				erase(&iter);
				return true;
			}
			else
			{
				return false;
			}
		}

		void erase(iterator* iter)
		{
			size_t curr;

			curr = iter->m_i;

			/* Blank out the fields */
			if (!std::is_pod<KeyType>::value)
			{
				if (IN_USE(curr))
				{
					m_data[curr].key.~KeyType();
					m_data[curr].value.~ValueType();
				}
			}
			SET_NOT_USE(curr);

			/* Reduce the size */
			m_size--;

			/* Calculate the number of elements that need rehash*/
			size_t num = 0;
			{
				size_t p = curr;
				INCREASE(p);
				while(IN_USE(p))
				{
					num++;
					INCREASE(p);
				}
			}

			element* rehashArr = NcNewArray(element, num);

			/* Store all succeeding elements into a temporary array.
				Because maybe they were pushed into the current place. 
				With an item removed, they will need to be rehashed. */
			element* p = rehashArr;
			INCREASE(curr);

			while (IN_USE(curr))
			{
				*p++ = m_data[curr];
				SET_NOT_USE(curr);
				m_data[curr].key.~KeyType();
				m_data[curr].value.~ValueType();
				INCREASE(curr);
				m_size--;
			}

			/*
				Rehash these stored elements
			*/
			p = rehashArr;
			element* pEnd = rehashArr + num;
			for(; p < pEnd; p++)
			{
				set(p->key, p->value);
			}

			NcDeleteArray(rehashArr);

			if(IN_USE(iter->m_i))
			{
				DECREASE(iter->m_i);
			}
		}

		void swap(Hashmap<KeyType, ValueType, Hasher>* r)
		{
			m_data.swap(r->m_data);
			cq_swap(char*, m_inUse, r->m_inUse);
			cq_swap(size_t, m_tableSize, r->m_tableSize);
			cq_swap(size_t, m_size, r->m_size);
		}

	private:
		bool _find(const KeyType& key, size_t* pos) const
		{
			size_t curr;
			size_t i;

			/* Find data location */
			curr = Hasher::hash(key) % m_tableSize;

			if (m_size == 0)
			{
				*pos = curr;
			}

			/* Linear probing, if necessary */
			for(i = 0; i < m_tableSize; i++) {
				bool isUse = IN_USE(curr) != 0;
				if(isUse && Hasher::equal(m_data[curr].key, key)) {
					*pos = curr;
					return true;
				}
				else if (!isUse)
				{
					*pos = curr;
					break;
				}

				INCREASE(curr);
			}

			/* Not found */
			return false;
		}

		Error _findInsertPosition(const KeyType& key, size_t* pos)
		{
			Error err;

			/* Find a place to put our value */
			err = _hash(key, pos);
			while(err == Error_full){
				if (_rehash() == Error_omem) {
					CQ_ASSERT(!"Out of memory");
					return Error_omem;
				}
				err = _hash(key, pos);
			}

			return err;
		}

		Error _hash(const KeyType& key, size_t* index)
		{
			size_t curr;
			size_t i;

			/* If 3/4 full, return immediately */
			if (m_size + 1 > (m_tableSize >> 1) + (m_tableSize >> 2))
				return Error_full;

			/* Find the best index */
			curr = Hasher::hash(key) % m_tableSize;

			/* Linear probling */
			for(i = 0; i< m_tableSize; i++){
				if(!IN_USE(curr) || Hasher::equal(m_data[curr].key, key))
				{
					*index = curr;
					return Error_ok;
				}

				INCREASE(curr);
			}

			return Error_full;
		}

		Error _rehash()
		{
			Hashmap<KeyType, ValueType, Hasher> tmp(m_tableSize * 2);

			/* Rehash the elements */
			for(size_t i = 0; i < m_tableSize; i++) 
			{
				if (IN_USE(i))
				{
					tmp.set(m_data[i].key, m_data[i].value);
				}
			}

			swap(&tmp);

			return Error_ok;
		}

		void _reserve(size_t bucketSize) 
		{
			m_data.reserve(bucketSize);
			m_inUse = (char*)malloc(((bucketSize + 7) / 8) * sizeof(char));
			memset(m_inUse, 0,((bucketSize + 7) / 8) * sizeof(char));

			m_tableSize = bucketSize;
			m_size = 0;
		}

	private:
		size_t m_tableSize;
		size_t m_size;
		Vector<element> m_data;
		char* m_inUse;
	};

#undef INCREASE
#undef DECREASE
#undef IN_USE
#undef SET_IN_USE
#undef SET_NOT_USE

