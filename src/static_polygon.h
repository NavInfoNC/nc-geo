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

int64 Polygon_calculateArea(Point* pts, size_t pointNum);

class StaticPolygon
{
public:
	forceinline StaticPolygon() { m_points = NULL; m_area = -1;  Rect_invalidate(&m_boundingBox); m_pointNumber = 0; m_isPointsCopied = true; }
	forceinline ~StaticPolygon() { if (m_isPointsCopied) free(m_points); }

	void initWithPoints(const Point* points, int n);
	void initWithPointsNoCopy(Point* points, int n);
	void initWithGrid(int x, int y, int gridSize);
	void initWithReversedPoints(const Point* points, int n);

	NC_PROPERTY(pointNumber);
	forceinline int pointNumber() { return m_pointNumber; }

	NC_PROPERTY(points);
	forceinline const Point* points() { return m_points; }

	NC_PROPERTY(pointsEnd);
	forceinline const Point* pointsEnd() { return m_points + m_pointNumber; }

	NC_PROPERTY(firstPoint);
	forceinline Point firstPoint() { return m_points[0]; }

	NC_PROPERTY(lastPoint);
	forceinline Point lastPoint() { return m_points[m_pointNumber - 1]; }

	forceinline Point pointAtIndex(int i) { return m_points[i]; }

	NC_PROPERTY(boundingBox);
	forceinline Rect boundingBox() { if (!Rect_isValid(&m_boundingBox)) updateBBox(); return m_boundingBox; }

    void adjustBoundingBoxByGridSize(int gridSize);

	void prepareForReuse() { free(m_points); m_points = NULL; Rect_invalidate(&m_boundingBox); m_pointNumber = 0; m_isPointsCopied = true; }

	int64 area() { if (m_area == -1) { m_area = Polygon_calculateArea(m_points, m_pointNumber); } return m_area; }

protected:
	Point* m_points;
	int m_pointNumber;
	Rect m_boundingBox;
	bool m_isPointsCopied;
	int64 m_area;

	void updateBBox();
};


