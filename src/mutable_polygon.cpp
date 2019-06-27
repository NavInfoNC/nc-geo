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
#include "basic_types.h"
#include "mutable_polygon.h"
#include "cq_algorithm.h"

static forceinline bool Point_less(const Point& l, const Point& r)
{
	return l.x < r.x || (l.x == r.x && l.y < r.y);
}

void MutablePolygon::reserve(int newSize)
{
	if (newSize > m_reservedSize)
	{
		m_points = (Point*)realloc(m_points, newSize * sizeof(Point));
		m_reservedSize = newSize;
	}
}

void MutablePolygon::reserveAligned(int newSize)
{
	uint32 index;
	uint32 mask;

	if (newSize > 1)
		newSize--;

	mask = (uint32)(newSize << 1);
	if (cq_bitScanReverse(&index, mask))
		newSize = (size_t)(1) << index;

	reserve(newSize);
}

void MutablePolygon::assignPoints(const Point* points, int n)
{
	reserve(n);
	memcpy(m_points, points, n * sizeof(Point));
	m_pointNumber = n;
	updateBBox();
}

void MutablePolygon::removeDuplicatedPoints()
{
	Point* end = cq::uniqueWithComparator(m_points, m_points + m_pointNumber, Point_less);
	m_pointNumber = (int)(end - m_points);
	if (m_pointNumber > 2 && m_points[0] == *(end - 1))
		m_pointNumber--;
}
