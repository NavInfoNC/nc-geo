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
