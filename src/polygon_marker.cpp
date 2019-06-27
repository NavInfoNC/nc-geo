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
#include "static_polygon.h"
#include "polygon_marker.h"
#include "cq_algorithm.h"

const static double sqrtOfTwo = sqrt(2.0);

bool _isPointInPolygon(StaticPolygon** polygons, const int polygonCount, Point testPoint)
{
	bool isInside = false;

	for (int i = 0; i < polygonCount; i++)
	{
		int ptCount = polygons[i]->pointNumber();
		const Point* pts = polygons[i]->points();
		for (int preIndex = ptCount - 1, curIndex = 0; curIndex < ptCount; preIndex = curIndex++)
		{
			const Point* left, * right;
			if (pts[preIndex].x < pts[curIndex].x)
			{
				left = &pts[preIndex];
				right = &pts[curIndex];
			}
			else
			{
				right = &pts[preIndex];
				left = &pts[curIndex];
			}

			if ((testPoint.x > left->x) == (testPoint.x <= right->x)
				&& ((int64)left->x - (int64)testPoint.x) * ((int64)right->y - (int64)testPoint.y)
				<= ((int64)right->x - (int64)testPoint.x) * ((int64)left->y - (int64)testPoint.y))
			{
				isInside = !isInside;
			}
		}
	}

	return isInside;
}

int PolygonMarker::Cell::segmentPointDistance(Point p1, Point p2, Point p)
{
	int64 x = p1.x;
	int64 y = p1.y;
	int64 dx = p2.x - p1.x;
	int64 dy = p2.y - p1.y;

	if (dx != 0 || dy != 0)
	{
		double t = double((p.x - p1.x) * dx + (p.y - p1.y) * dy) / (dx * dx + dy * dy);

		if (t > 1.f)
		{
			x = p2.x;
			y = p2.y;
		}
		else if (t > 0)
		{
			x = x + int64(dx * t);
			y = y + int64(dy * t);
		}
	}

	dx = p.x - x;
	dy = p.y - y;

	return (int)sqrt(dx * dx + dy * dy);
}

int PolygonMarker::Cell::pointPolygonDistance(Point p)
{
	int minDistance = INT_MAX;
	for (int i = 0; i < s_polygonCount; i++)
	{
		int pointNum = s_polygon[i]->pointNumber();
		const Point* pts = s_polygon[i]->points();

		for (int i = 0, j = pointNum - 1; i < pointNum; j = i++)
		{
			int dist = segmentPointDistance(pts[j], pts[i], p);
			if (dist < minDistance)
				minDistance = dist;
		}
	}
	return minDistance;
}

void PolygonMarker::findCentralMarkPoint(StaticPolygon** polygon, int polygonCount, Point* outCenter, int* outRadius)
{
	m_memPools.freeAllObjects();

	Cell::s_polygon = polygon;
	Cell::s_polygonCount = polygonCount;

	Rect boundingBox = polygon[0]->boundingBox();
	for (int i = 0; i < polygonCount; i++)
		boundingBox.combineRect(polygon[i]->boundingBox());

	m_candidatesA.clear();
	m_candidatesB.clear();

	// initial center is the center of bounding box
	m_bestCell = allocCell();
	Cell::s_polygonCenter = boundingBox.center();
	m_bestCell->init(boundingBox.center(), (cq_max(boundingBox.width(), boundingBox.height()) + 1)/2);
	m_candidatesA.append(m_bestCell);
	
	while (m_candidatesA.size() != 0 || m_candidatesB.size() != 0)
	{
		if (m_candidatesA.size() != 0)
			searchCandidateCells(m_candidatesA, m_candidatesB);
		else
			searchCandidateCells(m_candidatesB, m_candidatesA);
	}
	
	outCenter->x = m_bestCell->center.x;
	outCenter->y = m_bestCell->center.y;
	if (outRadius != NULL)
		*outRadius = m_bestCell->distance;
}

template<class Type>
static bool cellComparator(const Type& l, const Type& r)
{ 
	return l->potentialMax < r->potentialMax; 
}

void PolygonMarker::searchCandidateCells(CellVector& candidates, CellVector& newCandidates)
{
	cq::sortWithComparator<Cell*>(candidates.begin(), candidates.end(), cellComparator);

	while (!candidates.empty())
	{
		Cell* c = candidates.back();
		candidates.popBack();
		int halfSize = (c->halfSize + 1) / 2;

		if (c->potentialMax - m_bestCell->distance <= m_precision)
			continue;		

		Cell* subCells[4];
		for (int i = 0; i < 4; i++)
			subCells[i] = allocCell();

		subCells[0]->init(Point_make(c->center.x - halfSize, c->center.y - halfSize), halfSize);
		subCells[1]->init(Point_make(c->center.x + halfSize, c->center.y - halfSize), halfSize);
		subCells[2]->init(Point_make(c->center.x + halfSize, c->center.y + halfSize), halfSize);
		subCells[3]->init(Point_make(c->center.x - halfSize, c->center.y + halfSize), halfSize);


		for (int i = 0; i < 4; i++)
			newCandidates.append(subCells[i]);
		if (c->distance > m_bestCell->distance)
			m_bestCell = c;
	}
}

PolygonMarker::PolygonMarker()
{
	m_candidatesA.reserve(1000);
	m_candidatesB.reserve(1000);
	m_precision = 1;
}

PolygonMarker::~PolygonMarker()
{
}

void PolygonMarker::Cell::init(Point cellCenter, int cellHalfSize)
{	
	center = cellCenter;
	halfSize = cellHalfSize;
	distance = pointPolygonDistance(cellCenter);
	distance = _isPointInPolygon(s_polygon, s_polygonCount, center) ? distance : -distance;
	potentialMax = distance + int(cellHalfSize * sqrtOfTwo);
}

StaticPolygon** PolygonMarker::Cell::s_polygon = NULL;
int PolygonMarker::Cell::s_polygonCount = 0;

Point PolygonMarker::Cell::s_polygonCenter = Point_make(0, 0);
