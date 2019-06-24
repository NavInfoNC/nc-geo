#include "stdafx.h"
#include "split_algorithms/static_polygon.h"

int64 Polygon_calculateArea(Point* pts, size_t pointNum)
{
	int64 area = 0;
	for (size_t i = 0, j = pointNum - 1; i < pointNum; j = i++)
	{
		area += (int64)(pts[i].x) * pts[j].y - (int64)(pts[j].x) * pts[i].y;
	}

	area /= 2;

	return area > 0 ? area : -area;
}

//////////////////////////////////////////////////////////////////////////

void StaticPolygon::initWithPointsNoCopy(Point* points, int n)
{
	CQ_ASSERT(m_points == NULL);
	m_points = points;
	m_pointNumber = n;
	m_isPointsCopied = false;
}

void StaticPolygon::initWithPoints(const Point* points, int pointNum)
{
	CQ_ASSERT(m_points == NULL);
	size_t size = pointNum * sizeof(Point);
	m_points = (Point*)malloc(size);
	memcpy(m_points, points, size);
	m_pointNumber = pointNum;
}

void StaticPolygon::initWithReversedPoints(const Point* points, int n)
{
	CQ_ASSERT(m_points == NULL);
	size_t size = n * sizeof(Point);
	m_points = (Point*)malloc(size);
	for (int i = 0; i < n; i++)
	{
		m_points[i] = points[n - i - 1];
	}
	m_pointNumber = n;
}

void StaticPolygon::initWithGrid(int x, int y, int gridSize)
{
	CQ_ASSERT(m_points == NULL);

	m_boundingBox.left = x * gridSize;
	m_boundingBox.top = y * gridSize;
	m_boundingBox.right = m_boundingBox.left + gridSize;
	m_boundingBox.bottom = m_boundingBox.top + gridSize;

	m_pointNumber = 4;

	m_points = (Point*)malloc(m_pointNumber * sizeof(Point));
	m_points[0].x = m_boundingBox.left;
	m_points[0].y = m_boundingBox.top;

	m_points[1].x = m_boundingBox.left;
	m_points[1].y = m_boundingBox.bottom;

	m_points[2].x = m_boundingBox.right;
	m_points[2].y = m_boundingBox.bottom;

	m_points[3].x = m_boundingBox.right;
	m_points[3].y = m_boundingBox.top;
}

void StaticPolygon::updateBBox()
{
	Rect_invalidate(&m_boundingBox);
	for (int i = 0; i < m_pointNumber; i++)
	{
		Rect_combinePoint(&m_boundingBox, &m_points[i]);
	}
}

void StaticPolygon::adjustBoundingBoxByGridSize(int gridSize)
{
	Rect gridBox;
	if (!Rect_isValid(&m_boundingBox))
		updateBBox();
	gridBox.left = divideRoundTowardNegativeInfinity(m_boundingBox.left, gridSize);
	gridBox.top = divideRoundTowardNegativeInfinity(m_boundingBox.top, gridSize);
	gridBox.right = divideRoundTowardNegativeInfinity(m_boundingBox.right, gridSize);
	if (m_boundingBox.right % gridSize != 0)
		gridBox.right++;

	gridBox.bottom = divideRoundTowardNegativeInfinity(m_boundingBox.bottom, gridSize);
	if (m_boundingBox.bottom % gridSize != 0)
		gridBox.bottom++;
	m_boundingBox.left = gridBox.left * gridSize;
	m_boundingBox.top = gridBox.top * gridSize;
	int64 right = (int64)gridBox.right * gridSize;
	m_boundingBox.right = right <= INT_MAX ? (int)right : INT_MAX;
	m_boundingBox.bottom = gridBox.bottom * gridSize;
}
