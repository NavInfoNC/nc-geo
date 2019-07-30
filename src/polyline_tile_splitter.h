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

typedef uint32 GridId;

typedef void(*SplitResultCallback)(const Point* points, size_t n, void* userData);

class LineSplitter
{
public:
	LineSplitter();
	~LineSplitter();

	void setTileSize(int32 tileSize);
	bool split(const Point* points, size_t n, void* userData, SplitResultCallback callback);

	void _reservePoints(size_t newSize);

private:
	enum Position
	{
		Position_none = 0,
		Position_left = 1,	///< 0001
		Position_right = 2,	///< 0010
		Position_top = 8,	///< 1000
		Position_bottom = 4,	///< 0100
		Position_topLeft = 9,	///< 1001
		Position_topRight = 10,	///< 1010
		Position_bottomLeft = 5,	///< 0101
		Position_bottomRight = 6		///< 0110
	};

	Position getRelativeGridPos(GridId targetGridId, GridId baseGridId);
	GridId generateGridId(GridId currentGridId, const Point *p, Position nextPointPosition);
	Position testPointOnEdge(const Point *p, GridId gridId);
	Position getCrosspoint(GridId gridId, const Point* p1, const Point* p2, Point* pOut);

private:
	int32 m_tileSize;

	Point* m_points;
	size_t m_pointsMaxSize;
	
	Point* m_originalPoints;
	size_t m_originalPointsMaxSize;
};
