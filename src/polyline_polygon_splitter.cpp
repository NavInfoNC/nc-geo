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
#include "polyline_polygon_splitter.h"
#include "cq_vector.h"

enum IntersectionStatus
{
	NO_INTERSECTION,
	COINCIDE,
	ON_MIDDLE,
	ON_SECOND_LINE_START_POINT,
	ON_SECOND_LINE_END_POINT,
};

// (p1 - p0) X (p2 - p0)
inline static int64 _getCrossProduct(const Point &p0, const Point &p1, const Point &p2)
{
	int64 x1 = p1.x - p0.x, y1 = p1.y - p0.y, x2 = p2.x - p0.x, y2 = p2.y - p0.y;
	return x1 * y2 - x2 * y1;
}

inline static int32 _sgn(int64 x)
{
	return x == 0 ? 0 : x > 0 ? 1 : -1;
}

inline static bool _isBetween(const Point &lineStart, const Point &lineEnd, const Point &query)
{
	return _sgn(query.y - lineStart.y) * _sgn(query.y - lineEnd.y) <= 0 
		&& _sgn(query.x - lineStart.x) * _sgn(query.x - lineEnd.x) <= 0;
}

static IntersectionStatus _getIntersection(const Point &line1Start, const Point &line1End,
	const Point &line2Start, const Point &line2End, Point &intersection)
{
	int64 cross1 = _getCrossProduct(line1Start, line1End, line2Start);
	int64 cross2 = _getCrossProduct(line1Start, line1End, line2End);
	if (cross1 == 0 && cross2 == 0)
	{
		int32 check1, check2, check3, check4;
		if (line1Start.x == line1End.x)
		{
			check1 = line1Start.y;
			check2 = line1End.y;
			check3 = line2Start.y;
			check4 = line2End.y;
		}
		else
		{
			check1 = line1Start.x;
			check2 = line1End.x;
			check3 = line2Start.x;
			check4 = line2End.x;
		}
		if (check1 > check2)
			cq_swap(int32, check1, check2);
		if (check3 > check4)
			cq_swap(int32, check3, check4);
		int32 comp1 = cq_max(check1, check3), comp2 = cq_min(check2, check4);
		if (comp1 <= comp2)
			return COINCIDE;
		else
			return NO_INTERSECTION;
	}

	if (cross1 == 0)
	{
		if (_isBetween(line1Start, line1End, line2Start))
		{
			intersection = line2Start;
			return ON_SECOND_LINE_START_POINT;
		}
		else
			return NO_INTERSECTION;
	}

	if (cross2 == 0)
	{
		if (_isBetween(line1Start, line1End, line2End))
		{
			intersection = line2End;
			return ON_SECOND_LINE_END_POINT;
		}
		else
			return NO_INTERSECTION;
	}

	if (_sgn(cross1) * _sgn(cross2) > 0)
		return NO_INTERSECTION;
	intersection.x = (int32)(((double)line2Start.x * cross2 - (double)line2End.x * cross1) / (cross2 - cross1));
	intersection.y = (int32)(((double)line2Start.y * cross2 - (double)line2End.y * cross1) / (cross2 - cross1));
	if (_isBetween(line1Start, line1End, intersection))
		return ON_MIDDLE;
	else
		return NO_INTERSECTION;
}

static void _getCoincide(const Point* line1Start, const Point* line1End, const Point* line2Start, 
	const Point* line2End, const Point* &result1, const Point* &result2)
{
	if (line1Start->x == line1End->x)
	{
		bool isSwapped = false;
		if (line1Start->y > line1End->y)
		{
			isSwapped = true;
			cq_swap(const Point*, line1Start, line1End);
		}
		if (line2Start->y > line2End->y)
			cq_swap(const Point*, line2Start, line2End);
		result1 = line1Start->y < line2Start->y ? line2Start : line1Start;
		result2 = line1End->y < line2End->y ? line1End : line2End;
		if (isSwapped)
			cq_swap(const Point*, result1, result2);
	}
	else
	{
		bool isSwapped = false;
		if (line1Start->x > line1End->x)
		{
			isSwapped = true;
			cq_swap(const Point*, line1Start, line1End);
		}
		if (line2Start->x > line2End->x)
			cq_swap(const Point*, line2Start, line2End);
		result1 = line1Start->x < line2Start->x ? line2Start : line1Start;
		result2 = line1End->x < line2End->x ? line1End : line2End;
		if (isSwapped)
			cq_swap(const Point*, result1, result2);
	}
}

#define nextPointer(x) ((x) + 1 == poly.pointsEnd() ? poly.points() : (x) + 1)
#define prevPointer(x) ((x) == poly.points() ? poly.pointsEnd() - 1 : (x) - 1)

// Moving p1 towards p2 for an infinitely small distance, check whether the point is inside the polygon.
static bool _isPointInsidePolygonWithDirection(StaticPolygon &poly, const Point &p1, const Point &p2)
{
	bool cnt = false;
	const Point* end = poly.pointsEnd();
	for (const Point *i = poly.points(); i != end; ++i)
	{
		const Point *next = nextPointer(i);
		int64 cross1 = _getCrossProduct(p1, p2, *i), cross2 = _getCrossProduct(p1, p2, *next);
		if (cross1 == 0 && cross2 == 0)
			continue;
		if (cross1 == 0)
		{
			if (cross2 > 0 && _sgn(p2.x - p1.x) == _sgn(i->x - p1.x) && _sgn(p2.y - p1.y) == _sgn(i->y - p1.y))
				cnt = !cnt;
			continue;
		}
		if (cross2 == 0)
		{
			if (cross1 > 0 && _sgn(p2.x - p1.x) == _sgn(next->x - p1.x) && _sgn(p2.y - p1.y) == _sgn(next->y - p1.y))
				cnt = !cnt;
			continue;
		}
		
		if (_sgn(cross1) == _sgn(cross2))
			continue;

		int32 sgn1 = _sgn(_getCrossProduct({ 0, 0 }, { p2.x - p1.x, p2.y - p1.y }, { next->x - i->x, next->y - i->y }));
		int32 sgn2 = _sgn(_getCrossProduct(*i, *next, p1));
		if (sgn1 == sgn2)
			cnt = !cnt;
	}
	return cnt;
}

//	      /                                  -------------------
//	     /                                       /         \
//	 ---/------                                 /           \
//	   /                                       /             \
//	  /                                       /               \
//	 Be marked as NORMAL_INTERSECTION        Be marked as COINCIDE1_START and COINCIDE1_END
//	
//	           /
//	          /
//	 -------------      Be marked as COINCIDE2_START and COINCIDE2_END
//	   /
//	  /
class LineCutterImple
{
private:
	enum PointType
	{
		COINCIDE1_START,
		COINCIDE2_START,
		NORMAL_INTERSECTION,
		COINCIDE1_END,
		COINCIDE2_END,
	};
	typedef struct
	{
		Point point;
		PointType type;
	}Intersections;

	Vector <Intersections> intersections;
	Vector <Point> semiResult;
	Vector <Point> result, result2;
public:

	LineCutterImple()
	{
		intersections.reserve(1024);
		semiResult.reserve(1024);
		result.reserve(1024);
		result2.reserve(1024);
	}

	~LineCutterImple()
	{

	}

	void splitOneLine(StaticPolygon& poly, const Point& p1, const Point& p2, bool isInsideRemains, Vector <Point> &result)
	{
		intersections.clear();

		if (p1.x == p2.x && p1.y == p2.y)
			return;

		Rect lineBounding;
		Rect_setAsNegtiveMinimum(&lineBounding);
		Rect_combinePoint(&lineBounding, &p1);
		Rect_combinePoint(&lineBounding, &p2);
		// Shouldn't change to Rect_notIntersects
		Rect bbox = poly.boundingBox();
		if (cq_max(bbox.left, lineBounding.left) > cq_min(bbox.right, lineBounding.right)
			|| cq_max(bbox.top, lineBounding.top) > cq_min(bbox.bottom, lineBounding.bottom))
		{
			if (!isInsideRemains)
			{
				result.append(p1);
				result.append(p2);
			}
			return;
		}

		const Point *ends = poly.pointsEnd();
		bool needStartPointAdded = true, needEndPointAdded = true;
		for (const Point *i = poly.points(); i != ends; ++i)
		{
			const Point *next = nextPointer(i);
			Point curIntersec;
			IntersectionStatus status = _getIntersection(p1, p2, *i, *next, curIntersec);
			switch (status)
			{
			case NO_INTERSECTION:
			case ON_SECOND_LINE_START_POINT:
				continue;

			case ON_MIDDLE:
				intersections.append({ curIntersec, NORMAL_INTERSECTION });
				continue;

			case ON_SECOND_LINE_END_POINT:
				if (_sgn(_getCrossProduct(p1, p2, *i)) * _sgn(_getCrossProduct(p1, p2, *nextPointer(next))) < 0)
					intersections.append({ curIntersec, NORMAL_INTERSECTION });
				continue;

			case COINCIDE:
				// Find a series of points in one line.
				const Point *l = i, *r = next + 1;
				for (; r != ends && _getCrossProduct(p1, p2, *r) == 0; ++r);
				assert(!(i == poly.points() && r == poly.pointsEnd()));  // Not a line.
				r--;
				if (l == poly.points())
				{
					for (l = poly.pointsEnd() - 1; _getCrossProduct(p1, p2, *l) == 0; --l);
					ends = l + 1;
					l = nextPointer(l);
				}

				// Get coincide segment.
				const Point *coincide1, *coincide2;
				_getCoincide(&p1, &p2, l, r, coincide1, coincide2);
				if (_sgn(_getCrossProduct(p1, p2, *prevPointer(l))) *_sgn(_getCrossProduct(p1, p2, *nextPointer(r))) > 0)
				{
					intersections.append({ *coincide1, COINCIDE1_START });
					intersections.append({ *coincide2, COINCIDE1_END });
				}
				else
				{
					intersections.append({ *coincide1, COINCIDE2_START });
					intersections.append({ *coincide2, COINCIDE2_END });
				}

				if (coincide1 == &p1)
					needStartPointAdded = false;
				if (coincide2 == &p2)
					needEndPointAdded = false;
				if (r > i)
					i = r - 1;
				continue;
			}
		}

		if (needStartPointAdded)
		{
			// Hack for initialization.
			intersections.append({ p1, COINCIDE1_START });
			intersections.append({ p1, COINCIDE1_END });
		}
		if (needEndPointAdded)
			intersections.append({ p2, NORMAL_INTERSECTION });
		
		if (p1.x <= p2.x)
		{
			if (p1.y <= p2.y)
				intersections.sortWithComparator([](const Intersections &a, const Intersections &b)
			{
				if (a.point.x == b.point.x)
					if (a.point.y == b.point.y)
						return a.type < b.type;
					else
						return a.point.y < b.point.y;
				else
					return a.point.x < b.point.x;
			});
			else
				intersections.sortWithComparator([](const Intersections& a, const Intersections& b)
			{
				if (a.point.x == b.point.x)
					if (a.point.y == b.point.y)
						return a.type < b.type;
					else
						return a.point.y > b.point.y;
				else
					return a.point.x < b.point.x;
			});
		}
		else
		{
			if (p1.y <= p2.y)
				intersections.sortWithComparator([](const Intersections &a, const Intersections &b)
			{
				if (a.point.x == b.point.x)
					if (a.point.y == b.point.y)
						return a.type < b.type;
					else
						return a.point.y < b.point.y;
				else
					return a.point.x > b.point.x;
			});
			else
				intersections.sortWithComparator([](const Intersections &a, const Intersections &b)
			{
				if (a.point.x == b.point.x)
					if (a.point.y == b.point.y)
						return a.type < b.type;
					else
						return a.point.y > b.point.y;
				else
					return a.point.x > b.point.x;
			});
		}

		semiResult.clear();
		bool isInside = true;
		for (Intersections *i = intersections.begin(); i < intersections.end() - 1;)
		{
			Intersections *next = i + 1;
			if (i->type != COINCIDE1_START && i->type != COINCIDE2_START)
			{
				if (isInside == isInsideRemains)
				{
					semiResult.append(i->point);
					semiResult.append(next->point);
				}
				if (next->type == NORMAL_INTERSECTION)
					isInside = !isInside;
			}
			else
			{
				int intersectCnt = 1;
				for (; next < intersections.end() && intersectCnt > 0; ++next)
				{
					switch (next->type)
					{
					case NORMAL_INTERSECTION:
						isInside = !isInside;
						continue;
					case COINCIDE1_START:
					case COINCIDE2_START:
						intersectCnt++;
						continue;
					case COINCIDE1_END:
						intersectCnt--;
						continue;
					case COINCIDE2_END:
						intersectCnt--;
						isInside = !isInside;
						continue;
					}
				}
				next--;
				if (isInsideRemains)
				{
					semiResult.append(i->point);
					semiResult.append(next->point);
				}
				if (i == intersections.begin())
				{
					isInside = _isPointInsidePolygonWithDirection(poly, p1, p2);
				}
			}
			i = next;
		}

		// Merge continuous segments, and delete segments of zero length.
		for (Point *i = semiResult.begin(); i < semiResult.end(); i += 2)
		{
			if (Point_equal(i, i + 1))
				continue;
			Point *j;
			for (j = i + 2; j < semiResult.end() && Point_equal(j - 1, j); j += 2);
			result.append(*i);
			result.append(*(j - 1));
			i = j - 2;
		}
	}

	void split(StaticPolygon& poly, const Point* lines, size_t n, bool isInsideRemains,
		void* userData, PolylinePolygonSplitter::SplitResultCallback callback)
	{
		for (const Point *p = lines; p < lines + n - 1; p++)
		{
			size_t cur = 0;
			result2.clear();
			splitOneLine(poly, p[0], p[1], isInsideRemains, result2);
			assert(result2.size() % 2 == 0);

			if (!result.empty() && (result.end() - 1)->x == result2.begin()->x && (result.end() - 1)->y == result2.begin()->y)
			{
				result.append(result2.at(1));
				cur = 2;
			}

			if (cur < result2.size())
			{
				if (!result.empty())
				{
					callback(result.cvector(), result.size(), userData);
					result.clear();
				}
				for (size_t i = cur; i < result2.size() - 2; i += 2)
					callback(&result2[i], 2, userData);
				result.append(result2.at(result2.size() - 2));
				result.append(result2.at(result2.size() - 1));
			}
		}
		if (!result.empty())
		{
			callback(result.cvector(), result.size(), userData);
			result.clear();
		}
	}
};

#undef nextPointer
#undef prevPointer

PolylinePolygonSplitter::PolylinePolygonSplitter()
{
	m_imple = new LineCutterImple();
}

PolylinePolygonSplitter::~PolylinePolygonSplitter()
{
	delete m_imple;
}

void PolylinePolygonSplitter::split(StaticPolygon &poly, const Point* lines, size_t n, bool isInsideRemains, 
	void* userData, SplitResultCallback callback)
{
	m_imple->split(poly, lines, n, isInsideRemains, userData, callback);
}

