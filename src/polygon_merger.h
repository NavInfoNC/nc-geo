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
#include <hash_map>
#include <hash_set>
#include "basic_types.h"

class StaticPolygon;

/**
	@brief 
		This class merge a group of polygons. if holes exists in merge result, 
		the hole would be represented as a polygon with inverse direction.

		Firstly it would put all line segment into a map, then traversal segments,
		link all connecting segments to generate a contour, and remove them from segment map,
		until no segment exists in map.

		refer to : https://stackoverflow.com/questions/643995/algorithm-to-merge-adjacent-rectangles-into-polygon
 */

class PolygonMerger
{
public:
	PolygonMerger(){}
	~PolygonMerger();

	StaticPolygon** mergePolygons(const std::vector<StaticPolygon*>& polygons, int& outPolygonCount);

protected:
	void _buildSegmentSet(StaticPolygon* poly);
	void _generateContours();
	void _reset();
	
	static inline bool lessCompare(const Point *pt1, const Point * pt2)
	{
		if (pt1->x < pt2->x)
		{
			return true;
		}
		else if (pt1->y < pt2->y)
		{
			return true;
		}
		return false;
	}

	struct PointHasher : stdext::hash_compare<Point>
	{
		size_t operator()(const Point& pt) const
		{
			return pt.x * 7 + pt.y * 11;
		}
		bool operator()(const Point& lhs, const Point& rhs) const
		{
			return lessCompare(&lhs, &rhs);
		}
	};

	typedef std::vector<Point> Contour;
	typedef stdext::hash_set<Point, PointHasher> PointSet;
	typedef stdext::hash_map<Point, PointSet, PointHasher> SegmentSet;

private:
	SegmentSet m_segments;
	std::vector<StaticPolygon*> m_result;
};
