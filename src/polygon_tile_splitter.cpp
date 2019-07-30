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
#include "polygon_tile_splitter.h"
#include "cq_vector.h"
#include "cq_algorithm.h"
#include "mutable_polygon.h"
#include "small_object_allocator.h"

#define divideRoundTowardNegativeInfinity(a, b) ((a)/(b) - ((a)%(b)< 0 ? 1 : 0))
#define modRoundTowardNegativeInfinity(a, b) ((a)%(b) + ( (a)%(b)<0 ? (b) : 0))


struct Node
{
	const Point* p;
	Node* nextNode;
	size_t index;
	bool used;

	StaticPolygon* poly;
};

typedef Node NodePV;
typedef Node NodePH;

#define MOVE_NEXT(p) if (p##Forward) { p++; if (p == poly->pointsEnd()) p = poly->points(); } else { if (p == poly->points()) p = poly->pointsEnd() - 1; else p--; }

static double area(const Point* contour, size_t n)
{
	double A = 0.0;

	for (size_t p = n - 1, q = 0; q < n; p = q++)
	{
		A += (double)contour[p].x * (double)contour[q].y - (double)contour[q].x * (double)contour[p].y;
	}
	return A * 0.5;
}

static bool NodePV_less(const Node& l, const Node& r)
{
	bool crossed = false;
	StaticPolygon* poly = l.poly;

	if (l.p->y < r.p->y)
		return true;
	else if (l.p->y > r.p->y)
		return false;
	else
	{
		const Point* p1 = l.p;
		bool p1Forward = true;
		MOVE_NEXT(p1);
		if (p1->x == l.p->x)
			p1Forward = false;

		const Point* p2 = r.p;
		bool p2Forward = true;
		MOVE_NEXT(p2);
		if (p2->x == r.p->x)
			p2Forward = false;

		const Point* b = l.p;
		p1 = l.p;
		p2 = r.p;
		MOVE_NEXT(p1);
		MOVE_NEXT(p2);
		bool isLeft = p1->x < l.p->x;
		while (p1 != l.p)
		{
			if (p1 == r.p)
			{
				// The p1Forward and p2Forward direction may not be right at the beginning. For example:
				//  Q     L R     P
				//	*     * *     *
				//         * O    
				//     *       *
				// The polygon starts from R, CCW toward L.
				// The split parameter are hor = TRUE, top = TRUE. There are 4 nodes,
				// among which L and R are on the same point. 
				// In this case, p1Forward will be false, p2Forward will be true.
				// But follow this point order direction, L and R will both reach O and then cross each other. 
				// L will reach P and R will reach Q and we will reach a false conclusion that L greater than R(because P.x > Q.x).
				// So we must detect this senario and correct it.
				crossed = true;
			}

			if (*p1 == *p2)
			{
				b = p1;
				MOVE_NEXT(p1);
				MOVE_NEXT(p2);
			}
			else
			{
				int64 crossProduct;
				if (p1->x == p2->x && p1->x == b->x && ((p1->y - b->y > 0) ^ (p2->y - b->y > 0)))
				{
					crossProduct = p1->y < p2->y ? 1 : -1;
					if (crossed)
						crossProduct = -crossProduct;
				}
				else
				{
					crossProduct = int64(p2->y - p1->y)*(b->x - p1->x) - int64(b->y - p1->y)*(p2->x - p1->x);
					if (!isLeft)
						crossProduct = -crossProduct;
				}
				if (crossProduct > 0)
				{
					return true;
				}
				else if (crossProduct < 0)
				{
					return false;
				}
				else
				{
					if (int64(p1->x - b->x) * (p1->x - b->x) + int64(p1->y - b->y) * (p1->y - b->y)
						< int64(p2->x - b->x) * (p2->x - b->x) + int64(p2->y - b->y) * (p2->y - b->y))
					{
						MOVE_NEXT(p1);
					}
					else
					{
						MOVE_NEXT(p2);
					}
				}
			}
		}

		return p1Forward ^ isLeft;
	}
}

static bool NodePH_less(const Node& l, const Node& r)
{
	bool crossed = false;
	StaticPolygon* poly = l.poly;

	if (l.p->x < r.p->x)
		return true;
	else if (l.p->x > r.p->x)
		return false;
	else
	{
		const Point* p1 = l.p;
		bool p1Forward = true;
		MOVE_NEXT(p1);
		if (p1->y == l.p->y)
			p1Forward = false;

		const Point* p2 = r.p;
		bool p2Forward = true;
		MOVE_NEXT(p2);
		if (p2->y == r.p->y)
			p2Forward = false;

		const Point* b = l.p;
		p1 = l.p;
		p2 = r.p;
		MOVE_NEXT(p1);
		MOVE_NEXT(p2);
		bool isLeft = p1->y > l.p->y;
		while (p1 != l.p)
		{
			if (p1 == r.p)
			{
				crossed = true;
			}

			if (*p1 == *p2)
			{
				b = p1;
				MOVE_NEXT(p1);
				MOVE_NEXT(p2);
			}
			else
			{
				int64 crossProduct;
				if (p1->y == p2->y && p1->y == b->y && ((p1->x - b->x > 0) ^ (p2->x - b->x > 0)))
				{
					crossProduct = p1->x < p2->x ? 1 : -1;
					if (crossed)
						crossProduct = -crossProduct;
				}
				else
				{
					crossProduct = int64(p2->y - p1->y)*(b->x - p1->x) - int64(b->y - p1->y)*(p2->x - p1->x);
					if (!isLeft)
						crossProduct = -crossProduct;
				}
				if (crossProduct > 0)
				{
					return true;
				}
				else if (crossProduct < 0)
				{
					return false;
				}
				else
				{
					if (int64(p1->x - b->x) * (p1->x - b->x) + int64(p1->y - b->y) * (p1->y - b->y)
						< int64(p2->x - b->x) * (p2->x - b->x) + int64(p2->y - b->y) * (p2->y - b->y))
					{
						MOVE_NEXT(p1);
					}
					else
					{
						MOVE_NEXT(p2);
					}
				}
			}
		}

		return p1Forward ^ isLeft;
	}
}

#define SIDENESS(p) ( p->x < x ? 1 : (p->x > x ? 2 : 3) )

static void _splitVertical(StaticPolygon& poly, int x, MutablePolygon& result1, MutablePolygon& result2)
{
	int s1, s2;

	const Point* p1 = poly.points();
	const Point* p2 = p1;
	const Point* pbegin = p1;
	const Point* pend = poly.pointsEnd();

	s1 = SIDENESS(p1);

	for (;;)
	{
		p2++;
		if (p2 == pend)
			p2 = pbegin;

		s2 = SIDENESS(p2);

		if (s1 & s2)
		{
			if (s2 != 3)
				s1 = s2;
		}
		else
		{
			MutablePolygon* resultIn;
			MutablePolygon* resultOut;
			if (s2 == 2)
			{
				resultIn = &result1;
				resultOut = &result2;
			}
			else
			{
				resultIn = &result2;
				resultOut = &result1;
			}

			while (p1 != p2 && p1 != pend)
			{
				resultIn->addUniquePoint(*p1);
				p1++;
			}
			p1--;
			if (p1->x == x)
			{
				resultOut->addUniquePoint(*p1);
			}
			else
			{
				Point intersection;
				intersection.x = x;
				if (p1->x < p2->x)
					intersection.y = p1->y + (int64)(p2->y - p1->y) * (x - p1->x) / (p2->x - p1->x);
				else
					intersection.y = p2->y + (int64)(p1->y - p2->y) * (x - p2->x) / (p1->x - p2->x);
				result1.addUniquePoint(intersection);
				result2.addUniquePoint(intersection);
			}

			p1 = p2;
			s1 = SIDENESS(p1);
		}

		if (p2 == pbegin)
			break;
	}

	MutablePolygon* resultSide = (s1 & 1) ? &result1 : &result2;
	if (resultSide->points() == resultSide->pointsEnd())
	{
		int pointNumber = poly.pointNumber();
		resultSide->assignPoints(poly.points(), pointNumber);
	}
	else
	{
		if (p2->x == x && p2 == pbegin)
			p2++;
		while (p1 != p2)
		{
			resultSide->addUniquePoint(*p1);
			p1++;
			if (p1 == pend)
				p1 = pbegin;
		}
	}

	if (result1.pointNumber() != 0 && result1.firstPoint() == result1.lastPoint())
		result1.removeLastPoint();

	if (result2.pointNumber() != 0 && result2.firstPoint() == result2.lastPoint())
		result2.removeLastPoint();
}

#undef SIDENESS
#define SIDENESS(p) ( p->y < y ? 1 : (p->y > y ? 2 : 3) )

static void _splitHorizontal(StaticPolygon& poly, int y, MutablePolygon& result1, MutablePolygon& result2)
{
	int s1, s2;

	const Point* p1 = poly.points();
	const Point* p2 = p1;
	const Point* pbegin = p1;
	const Point* pend = poly.pointsEnd();

	s1 = SIDENESS(p1);

	for (;;)
	{
		p2++;
		if (p2 == pend)
			p2 = pbegin;

		s2 = SIDENESS(p2);

		if (s1 & s2)
		{
			if (s2 != 3)
				s1 = s2;
		}
		else
		{
			MutablePolygon* resultIn;
			MutablePolygon* resultOut;
			if (s2 == 2)
			{
				resultIn = &result1;
				resultOut = &result2;
			}
			else
			{
				resultIn = &result2;
				resultOut = &result1;
			}

			while (p1 != p2 && p1 != pend)
			{
				resultIn->addUniquePoint(*p1);
				p1++;
			}
			p1--;
			if (p1->y == y)
			{
				resultOut->addUniquePoint(*p1);
			}
			else
			{
				Point intersection;
				intersection.y = y;
				if (p1->y < p2->y)
					intersection.x = p1->x + (int64)(p2->x - p1->x) * (y - p1->y) / (p2->y - p1->y);
				else
					intersection.x = p2->x + (int64)(p1->x - p2->x) * (y - p2->y) / (p1->y - p2->y);
				result1.addUniquePoint(intersection);
				result2.addUniquePoint(intersection);
			}

			p1 = p2;
			s1 = SIDENESS(p1);
		}

		if (p2 == pbegin)
			break;
	}

	MutablePolygon* resultSide = (s1 & 1) ? &result1 : &result2;
	if (resultSide->pointNumber() == 0)
	{
		resultSide->copy(poly);
	}
	else
	{
		if (p2->y == y && p2 == pbegin)
			p2++;
		while (p1 != p2)
		{
			resultSide->addUniquePoint(*p1);
			p1++;
			if (p1 == pend)
				p1 = pbegin;
		}
	}

	if (result1.pointNumber() != 0 && result1.firstPoint() == result1.lastPoint())
		result1.removeLastPoint();

	if (result2.pointNumber() != 0 && result2.firstPoint() == result2.lastPoint())
		result2.removeLastPoint();
}

// Split polygon against uniform grids
class PolygonSplitterImple
{
public:
	PolygonSplitterImple()
	{
		m_nodes.reserve(1024);
		m_results = (StaticPolygon**)malloc(sizeof(StaticPolygon*) * 1024);
		m_resultMaxSize = 1024;
		m_resultNum = 0;
	}

	~PolygonSplitterImple()
	{
		for (int i = 0; i < m_resultNum; ++i)
			m_polygonAllocator.freeObject(m_results[i]);
		free(m_results);
	}

	void resultPushBack(StaticPolygon *element)
	{
		if (m_resultMaxSize <= m_resultNum)
		{
			m_resultMaxSize *= 2;
			m_results = (StaticPolygon**)realloc(m_results, sizeof(StaticPolygon*) * m_resultMaxSize);
		}
		m_results[m_resultNum++] = element;
	}

	virtual StaticPolygon** split(StaticPolygon* poly, int* num)
	{
		// clear last result
		for (int i = 0; i < m_resultNum; ++i)
			m_polygonAllocator.freeObject(m_results[i]);

		m_resultNum = 0;

		StaticPolygon* polyBack = m_polygonAllocator.allocObject();
		if (area(poly->points(), poly->pointNumber()) < 0)
		{
			polyBack->initWithReversedPoints(poly->points(), poly->pointNumber());
		}
		else
		{
			polyBack->initWithPoints(poly->points(), poly->pointNumber());
		}
		polyBack->adjustBoundingBoxByGridSize(m_tileSize);
		
		int reserveSize = (poly->pointNumber()) << 1;
		m_left.reserve(reserveSize);
		m_right.reserve(reserveSize);
		m_correctPoly.reserve(reserveSize);

		//////////////////////////////////////////////////////////////////////////
		// START 

		// plant seed
		m_que.clear();
		m_que.append(polyBack);

		while (!m_que.empty())
		{
			// get the last one and pop it from the list
			polyBack = m_que.back();
			m_que.popBack();

			polyBack->adjustBoundingBoxByGridSize(m_tileSize);

			if (polyBack->pointNumber() < 3)	// ignore trivial polygon)
			{
				m_polygonAllocator.freeObject(polyBack);
			}
			// if it fits into the smallest grid, collect it.
			else if (Rect_getWidth(polyBack->boundingBox()) <= m_tileSize && Rect_getHeight(polyBack->boundingBox()) <= m_tileSize)
			{
				resultPushBack(polyBack);
			}
			else if (polyBack->pointNumber() == 4 && collectSimpleRectangle(*polyBack, m_tileSize))
			{
				m_polygonAllocator.freeObject(polyBack);
			}
			// if not, split it into pieces.
			else
			{
				bool hor = Rect_getWidth(polyBack->boundingBox()) < Rect_getHeight(polyBack->boundingBox());
				int middle;

				// clear
				m_left.clear();
				m_right.clear();

				splitOnce(*polyBack, m_left, m_right, hor, middle, m_tileSize);

				if (m_left.pointNumber() != 0)
				{
					if (!divideWeakSimplePolygon(m_left, middle, hor, true))
						goto Fails;
				}

				if (m_right.pointNumber() != 0)
				{
					if (!divideWeakSimplePolygon(m_right, middle, hor, false))
						goto Fails;
				}
				m_polygonAllocator.freeObject(polyBack);
				continue;
			Fails:
				m_polygonAllocator.freeObject(polyBack);
				while (!m_que.empty())
				{
					polyBack = m_que.back();
					m_polygonAllocator.freeObject(polyBack);
					m_que.popBack();
				}
				return NULL;
			}
		}
		*num = m_resultNum;
		return m_results;
	}

	virtual void setTileSize(int32 tileSize) 
	{
		m_tileSize = tileSize;
	}

private:
	void splitOnce(StaticPolygon& poly, MutablePolygon& result1, MutablePolygon& result2, bool hor, int& middle, int gridSize)
	{
		if (hor)
		{
			//middle = (poly.m_box.bottom + poly.m_box.top) / 2;
			middle = (poly.boundingBox().bottom >> 1) + (poly.boundingBox().top >> 1) + (poly.boundingBox().bottom & poly.boundingBox().top & 1);
			middle = middle / gridSize * gridSize;
			_splitHorizontal(poly, middle, result1, result2);
		}
		else
		{
			//middle = (poly.m_box.right + poly.m_box.left) / 2;
			int64 right = (int64)poly.boundingBox().right + 1;		// avoid middle align to left bounding when right is INT_MAX
			middle = (int32)((right >> 1) + (poly.boundingBox().left >> 1) + (poly.boundingBox().right & poly.boundingBox().left & 1));
			middle = middle / gridSize * gridSize;
			_splitVertical(poly, middle, result1, result2);
		}
		result1.adjustBoundingBoxByGridSize(m_tileSize);
		result2.adjustBoundingBoxByGridSize(m_tileSize);
	}

	bool divideWeakSimplePolygon(MutablePolygon& poly, int middle, bool hor, bool top)
	{
		m_nodes.clear();

		const Point* start = poly.points();
		const Point* cur = start;
		const Point* next = start + 1;
		const Point* pend = poly.pointsEnd();
		const Point* last = pend - 1;

		while (cur != pend)
		{
			if (
				(hor && ((last->y == middle && cur->y == middle && next->y != middle) || (last->y != middle && cur->y == middle && next->y == middle)))
				||
				(!hor && ((last->x == middle && cur->x == middle && next->x != middle) || (last->x != middle && cur->x == middle && next->x == middle)))
				)
			{
				Node node;
				node.poly = &poly;
				node.used = false;
				node.p = cur;
				node.index = m_nodes.size();
				m_nodes.append(node);
			}
			last++;
			cur++;
			next++;
			if (next == pend)
				next = start;
			if (last == pend)
				last = start;
		}

		if (m_nodes.empty())
		{
			StaticPolygon* polygon = m_polygonAllocator.allocObject();
			polygon->initWithPoints(poly.points(), poly.pointNumber());
			m_que.append(polygon);
		}
		else
		{
			//assert((vector_size(&m_nodes) & 1) == 0);
			if ((m_nodes.size() & 1) != 0)
				goto Fails;

			{
				Node* ibegin = m_nodes.begin();
				Node* iend = m_nodes.end();
				if (hor)
					cq::sortWithComparator(ibegin, iend, NodePH_less);
				else
					cq::sortWithComparator(ibegin, iend, NodePV_less);

				if (hor ^ top)
					cq::reverse(ibegin, iend);
			}

			size_t nodeNum = m_nodes.size();

			size_t* arr = (size_t*)alloca(sizeof(size_t) * nodeNum);
			for (size_t i = 0; i < nodeNum; i++)
			{
				arr[m_nodes.at(i).index] = i;
			}
			for (size_t i = 0; i < nodeNum; i++)
			{
				size_t nextIndex = m_nodes.at(i).index + 1;
				if (nextIndex == nodeNum)
					nextIndex = 0;
				m_nodes.at(i).nextNode = m_nodes.atptr(arr[nextIndex]);
			}

			size_t startNodeIndex = 0;
			Node* pNode = NULL;
			size_t nodeIndex = UINT_MAX;
			for (;;)
			{
				if (nodeIndex == UINT_MAX)
				{
					for (nodeIndex = 0; nodeIndex < m_nodes.size(); nodeIndex++)
					{
						if (!m_nodes.at(nodeIndex).used)
						{
							break;
						}
					}

					if (nodeIndex == m_nodes.size())
						break;

					startNodeIndex = nodeIndex;

					pNode = m_nodes.atptr(nodeIndex);
					m_correctPoly.clear();
				}

				pNode->used = true;
				pNode->nextNode->used = true;

				const Point* p = pNode->p;
				while (p != pNode->nextNode->p)
				{
					m_correctPoly.addUniquePoint(*p);
					p++;
					if (p == pend)
						p = start;
				}

				m_correctPoly.addUniquePoint(*p);

				nodeIndex = arr[pNode->nextNode->index] - 1;
				//assert(nodeIndex != (size_t)(-1));
				if (nodeIndex == (size_t)(-1))
					goto Fails;
				if (nodeIndex == startNodeIndex)
				{
					StaticPolygon* polygon = m_polygonAllocator.allocObject();
					polygon->initWithPoints(m_correctPoly.points(), m_correctPoly.pointNumber());
					m_que.append(polygon);
					nodeIndex = UINT_MAX;
				}
				else
				{
					pNode = m_nodes.atptr(nodeIndex);
				}
			}
		}

		return true;

	Fails:
		return false;
	}

	bool collectSimpleRectangle(StaticPolygon& poly, int m_tileSize)
	{
		int i;

		for (i = 0; i < 4; i++)
		{
			if (poly.pointAtIndex(i).x % m_tileSize != 0 || poly.pointAtIndex(i).y % m_tileSize != 0)
			{
				return false;
			}
		}

		Rect box = poly.boundingBox();

		for (i = 0; i < 4; i++)
		{
			if ((poly.pointAtIndex(i).x != box.left && poly.pointAtIndex(i).x != box.right)
				|| (poly.pointAtIndex(i).y != box.top && poly.pointAtIndex(i).y != box.bottom))
			{
				return false;
			}
		}

		Rect gridBox;
		gridBox.left = divideRoundTowardNegativeInfinity(box.left, m_tileSize);
		gridBox.top = divideRoundTowardNegativeInfinity(box.top, m_tileSize);
		gridBox.right = divideRoundTowardNegativeInfinity(box.right, m_tileSize);
		gridBox.bottom = divideRoundTowardNegativeInfinity(box.bottom, m_tileSize);

		for (int y = gridBox.top; y < gridBox.bottom; y++)
		{
			for (int x = gridBox.left; x < gridBox.right; x++)
			{
				StaticPolygon* staticPoly = m_polygonAllocator.allocObject();
				staticPoly->initWithGrid(x, y, m_tileSize);
				resultPushBack(staticPoly);
			}
		}
		return true;
	}

	int32 m_tileSize;

	StaticPolygon** m_results;
	int m_resultNum;
	int m_resultMaxSize;

	Vector<StaticPolygon*> m_que;

	MutablePolygon m_left, m_right;
	MutablePolygon m_correctPoly;

	Vector <Node> m_nodes;
	SmallObjectAllocator<StaticPolygon> m_polygonAllocator;
};

PolygonTileSplitter::PolygonTileSplitter()
{
	m_imple = new PolygonSplitterImple();
}

PolygonTileSplitter::~PolygonTileSplitter()
{
	delete m_imple;
}

void PolygonTileSplitter::setTileSize(int32 tileSize)
{
	m_imple->setTileSize(tileSize);
}

StaticPolygon** PolygonTileSplitter::split(StaticPolygon* poly, int* num)
{
	return m_imple->split(poly, num);
}
