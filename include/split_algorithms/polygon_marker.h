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
#include "split_algorithms/small_object_allocator.h"

class StaticPolygon;

// Each MarkPoint has a levelStride, which means the point is visible in level range [maxZoomLevel - levelStride, maxZoomLevel].
struct MarkPoint
{
	union
	{
		Point pos;
		struct  
		{
			int x;
			int y;
		};
	};

	int level;	// 0 means the base level. The point is visible from 0 to |level|, inclusive.
};

/**
	@brief Find suitable positions to add label text within a polygon.
*/
class PolygonMarker
{
public:
	struct Cell
	{
		Point center;
		int halfSize;
		int distance;
		int potentialMax;

		void init(Point cellCenter, int cellHalfSize);

		static StaticPolygon** s_polygon;
		static int s_polygonCount;
		static Point s_polygonCenter;

		static int pointPolygonDistance(Point p);
		static int segmentPointDistance(Point p1, Point p2, Point p);
	};

	typedef Vector<Cell*> CellVector;

	PolygonMarker();
	~PolygonMarker();

	NC_PROPERTY(centralPointPrecision, default = 1);
	forceinline void setCentralPointPrecision(int precision) { m_precision = precision; }

	/**
		Find the most suitable mark point of a polygon.
		@remarks
			The center of the largest inscribed circle is considered as the best mark point.
			It supports polygon with holes.

		@param [in] polygon
			It contains the shape points of outer contour and inner holes, each of which is a StaticPolygon.
	*/
	void findCentralMarkPoint(StaticPolygon** polygon, int polygonCount, Point* outCenter, int* outRadius);

protected:
	void searchCandidateCells(CellVector& candidates, CellVector& newCandidates);
	forceinline Cell* allocCell() { return m_memPools.allocObject(); }
	
private:
	CellVector m_candidatesA;
	CellVector m_candidatesB;
	Vector<MarkPoint> m_outputMarkPoints;
	Cell* m_bestCell;

	int m_precision;
	SmallObjectAllocator<Cell> m_memPools;
};

bool _isPointInPolygon(StaticPolygon** polygons, const int polygonCount, Point testPoint);