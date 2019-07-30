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

#include "static_polygon.h"
#include "cq_vector.h"

class MutablePolygon : public StaticPolygon
{
public:
	forceinline MutablePolygon() { m_reservedSize = 0; }

	void reserve(int num);

	NC_PROPERTY(reservedSize);
	forceinline int reservedSize() { return m_reservedSize; }

	forceinline void addPoint(Point point) { if (m_pointNumber + 1 > m_reservedSize) { reserveAligned(m_pointNumber + 1); } m_points[m_pointNumber++] = point; }
	void assignPoints(const Point* points, int n);

	forceinline void addUniquePoint(Point point) { if (m_pointNumber == 0 || m_points[m_pointNumber - 1] != point) { addPoint(point); } }

	forceinline void copy(StaticPolygon& poly) { assignPoints(poly.points(), poly.pointNumber()); }

	forceinline void clear() { m_pointNumber = 0; Rect_invalidate(&m_boundingBox); }

	void removeFirstPoint() { assert(m_pointNumber != 0); memmove(m_points, m_points + 1, sizeof(Point) * (m_pointNumber - 1)); m_pointNumber--; updateBBox(); }
	void removeLastPoint() { assert(m_pointNumber != 0); m_pointNumber--; updateBBox(); }
	void removeDuplicatedPoints();

private:
	int m_reservedSize;

	void reserveAligned(int num);
};
