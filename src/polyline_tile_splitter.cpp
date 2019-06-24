#include "stdafx.h"
#include "split_algorithms/polyline_tile_splitter.h"

enum LineSplitter::Position
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

typedef LineSplitter::Position Position;

void getColRowByGridId(GridId gridId, int* xCol, int* yRow)
{
	*yRow = (gridId & 0xffff0000) >> 16;
	*xCol = gridId & 0x0000ffff;
}


GridId getGridIdfromRowCol(uint16 yRow, uint16 xCol)
{
	GridId id;
	id = (yRow << 16) | xCol;
	return id;
}

Rect getRectByGridId(GridId gridId, int32 gridSize)
{
	int32 xCol, yRow;
	getColRowByGridId(gridId, &xCol, &yRow);
	Rect rect;
	Rect_set(&rect, xCol*gridSize, yRow*gridSize, (xCol + 1)*gridSize, (yRow + 1)*gridSize);
	return rect;
}

GridId getGridIdFromPoint(const Point* point, int32 tileSize)
{
	uint16 gx, gy;
	gx = (uint16)divideRoundTowardNegativeInfinity(point->x, tileSize);
	gy = (uint16)divideRoundTowardNegativeInfinity(point->y, tileSize);
	return getGridIdfromRowCol(gy, gx);
}

LineSplitter::LineSplitter()
{
	m_tileSize = 0;
	m_points = NULL;
	m_pointsMaxSize = 0;
	m_originalPoints = NULL;
	m_originalPointsMaxSize = 0;
}

LineSplitter::~LineSplitter()
{
	free(m_points);
	free(m_originalPoints);
}

void LineSplitter::setTileSize(int32 tileSize)
{
	if (tileSize > 0)
		m_tileSize = tileSize;
}

Position LineSplitter::getCrosspoint(GridId gridId, const Point* p1, const Point* p2, Point* pOut)
{
	Position code1, code2;
	Rect rect = getRectByGridId(gridId, m_tileSize);
	code1 = Position_none;

	GridId newGridId = getGridIdFromPoint(p2, m_tileSize);
	code2 = getRelativeGridPos(newGridId, gridId);

	// p1点在矩形内部，而p2点在矩形外部
	//CQ_ASSERT(code2 != Position_none);

	if ((code2 & Position_left) != Position_none) //< 与左边界相交
	{
		pOut->x = rect.left;
		// 三角形相似原理
		if (p2->x == p1->x){
			//CQ_ASSERT(FALSE);
			pOut->y = (p1->y + p2->y) / 2;
		}
		else{
			pOut->y = p1->y + (int32)((double)(p2->y - p1->y) * (rect.left - p1->x) / (p2->x - p1->x));
		}

		if (pOut->y <= rect.bottom && pOut->y >= rect.top)
		{
			code1 = Position_left;
		}
	}
	else if ((code2 & Position_right) != Position_none) //< 与右边界相交
	{
		pOut->x = rect.right;
		if (p2->x == p1->x){
			//CQ_ASSERT(FALSE);
			pOut->y = (p1->y + p2->y) / 2;
		}
		else{
			pOut->y = p1->y + (int32)((double)(p2->y - p1->y) * (rect.right - p1->x) / (p2->x - p1->x));
		}

		if (pOut->y <= rect.bottom && pOut->y >= rect.top)
		{
			code1 = Position_right;
		}
	}

	if (code1 != Position_none)
		return code1;

	if ((code2 & Position_bottom) != Position_none) //< 与下边界相交
	{
		pOut->y = rect.bottom;
		if (p2->y == p1->y){
			//CQ_ASSERT(FALSE);
			pOut->x = (p1->x + p2->x) / 2;
		}
		else{
			pOut->x = p1->x + (int32)((double)(p2->x - p1->x) * (rect.bottom - p1->y) / (p2->y - p1->y));
		}
		if (pOut->x <= rect.right && pOut->x >= rect.left)
		{
			code1 = Position_bottom;
		}
	}
	else if ((code2 & Position_top) != Position_none) //< 与上边界相交
	{
		pOut->y = rect.top;
		if (p2->y == p1->y){
			pOut->x = (p1->x + p2->x) / 2;
		}
		else{
			pOut->x = p1->x + (int32)((double)(p2->x - p1->x) * (rect.top - p1->y) / (p2->y - p1->y));
		}

		if (pOut->x <= rect.right && pOut->x >= rect.left)
		{
			code1 = Position_top;
		}
	}

	if (code1 == Position_none)
	{
		printf("GridId: %d, p1(%d, %d), p2(%d, %d), p3(%d, %d), gridBBox:(%d,%d,%d,%d)\n"
			, gridId, p1->x, p1->y, p2->x, p2->y
			, pOut->x, pOut->y
			, rect.left, rect.top, rect.right, rect.bottom);
	}

	CQ_ASSERT(code1 != Position_none);
	CQ_ASSERT(pOut->x >= rect.left && pOut->y >= rect.top && pOut->x <= rect.right && pOut->y <= rect.bottom);
	return code1;
}

/** @brief
获取一个Grid相对于另一个Grid的位置，可用作获取Conhe-Sutherland算法所需的编码函数。
@param [in] targetGridId
需要求位置关系格子
@param [in] baseGridridId
基准格子
@return
相对位置关系
*/
Position LineSplitter::getRelativeGridPos(GridId targetGridId, GridId baseGridId)
{
	int xCol, yRow, newColX, newRowY;
	Position c = Position_none;
	getColRowByGridId(baseGridId, &xCol, &yRow);
	getColRowByGridId(targetGridId, &newColX, &newRowY);
	if (newColX < xCol)
	{
		c = (Position)(c | Position_left);
	}
	else if (newColX > xCol)
	{
		c = (Position)(c | Position_right);
	}

	if (newRowY < yRow)
	{
		c = (Position)(c | Position_top);
	}
	else if (newRowY > yRow)
	{
		c = (Position)(c | Position_bottom);
	}
	return c;
}

/*!
@brief
判断当前点是否在边界上
@param [in] p
待检测点
@param [in] rct
待检测矩形
@return
如果点不在矩形的边缘上, 返回Position_none, 否则返回对应的边或角
*/
Position LineSplitter::testPointOnEdge(const Point *p, GridId gridId)
{
	Rect rect = getRectByGridId(gridId, m_tileSize);
	Position c = Position_none;

	if (rect.top <= p->y && p->y <= rect.bottom)
	{
		if (p->x == rect.left)
		{
			c = (Position)(c | Position_left);
		}
		else if (p->x == rect.right)
		{
			c = (Position)(c | Position_right);
		}
	}

	if (p->x >= rect.left && p->x <= rect.right)
	{
		if (p->y == rect.top)
		{
			c = (Position)(c | Position_top);
		}
		else if (p->y == rect.bottom)
		{
			c = (Position)(c | Position_bottom);
		}
	}

	return c;
}

/*!
@brief
根据下一个点的位置来确定当前边界上点所归属的格子
@param [in] preGridId
当前点所在边界的上一个格子ID
@param [in] p
当前要判定的点
@param [in] rct
需要判定的格子的包络盒
@param [in] nextPointPosition
下一个点相对当前格子的位置
@return
经过调整的格子ID
*/
GridId LineSplitter::generateGridId(GridId currentGridId, const Point *p, Position nextPointPosition)
{
	Position position = testPointOnEdge(p, currentGridId);
	int xCol, yRow;
	getColRowByGridId(currentGridId, &xCol, &yRow);
	Position pos = (Position)(position & nextPointPosition);
	if ((pos & Position_left) == Position_left)
	{
		xCol -= 1;
	}
	else if ((pos & Position_right) == Position_right)
	{
		xCol += 1;
	}

	if ((pos & Position_top) == Position_top)
	{
		yRow -= 1;
	}
	else if ((pos & Position_bottom) == Position_bottom)
	{
		yRow += 1;
	}

	// @todo fix it: uint32 is too small for GridId, consider using struct GridId {int x; int y;}
	return getGridIdfromRowCol((uint16)yRow, (uint16)xCol);
}

// Use Conhe-Sutherland algorithm.
/*!
@brief
使用Conhe-Sutherland算法切分一条路段
每次执行完此函数，即将一条路段完全切分
算法基本流程：
1、判断线段是否跨格子。
2、如果跨格子，线段的起始点为路段第一个点且为交点，则根据第二个点来重新设置第一个点(交点)所在的格子，继续向后查找。
3、如果有交点且不为2中的情况，那么由此点将路段切分，切分出来的有效路段放入有效数据Map中，未检测的路段
重新当作新路段继续检测，将此打断的交点作为新路段的起点，重新从1开始执行，直到路段切分完成。
*/
bool LineSplitter::split(const Point* points, size_t n, void* userData, SplitResultCallback callback)
{
	if (m_tileSize == 0)
		return false;

	_reservePoints(n);

	if (m_originalPointsMaxSize < n)
	{ 
		m_originalPoints = (Point*)realloc(m_originalPoints, n * sizeof(Point));
		m_originalPointsMaxSize = n;
	}

	memcpy(m_originalPoints, points, n * sizeof(Point));

	GridId preGridId = 0;
	GridId curGridId = 0;
	bool isStartPoint = true;
	size_t beginIdx = 0;
	size_t curIdx = beginIdx + 1;
	bool isAllPointsInOneGrid = true;
	
	while (curIdx < n)
	{
		if (isStartPoint)
		{
			preGridId = getGridIdFromPoint(&m_originalPoints[beginIdx], m_tileSize);
		}

		curGridId = getGridIdFromPoint(&m_originalPoints[curIdx], m_tileSize);

		if (curGridId != preGridId)
		{
			// In different Grids, need to be cut.
			// 如果最后一个点落在边界上将会出现长度为0的线段
			// 起始点如果在边界上通过方向判断格子归属
			//        1        2        3
			//   +--------+--------+--------+
			//   |        |        |        |
			// 1 |        |        |        |
			//   |        | 1      |        |
			//   +--------+--o-----+--------+
			//   |        |        |        |
			// 2 |        |        |        |
			//   |        |        |        |
			//   +--------+--------+--------+
			//   |        |        |  2 o   |
			// 3 |        |        |        |
			//   |        |        |        |
			//   +--------+--------+--------+

			//////////////////////////////////////////////////////////////////////////
			// 判断起始点和终止点是否有存在界面上的情况
			// 如果起始点在边缘上，根据终止点的位置确定起始点所属格子，如上图，起始点1, 终止点为2
			// 所在格子被重置为2L2C格子，而不是根据约定计算出来的1L2C格子
			Point crossPoint;
			bool crossOnPrePoint;
			getCrosspoint(preGridId, &m_originalPoints[curIdx - 1], &m_originalPoints[curIdx], &crossPoint);
			crossOnPrePoint = Point_equal(&m_originalPoints[curIdx - 1], &crossPoint);

			// 起始点在边界的情况，将起始点根据下一个点的位置来重新分配格子
			if (crossOnPrePoint && isStartPoint)
			{
				Position nextPointPosition = getRelativeGridPos(curGridId, preGridId);
				preGridId = generateGridId(preGridId, &m_originalPoints[curIdx - 1], nextPointPosition);
				isStartPoint = false;
				continue;
			}

			isAllPointsInOneGrid = false;
			size_t resultSize = curIdx - beginIdx;
			if (!crossOnPrePoint)
				resultSize++;
			_reservePoints(resultSize);

			// 如果当前点没有落在边界上，添加交点生成新路段
			if (!Point_equal(&m_originalPoints[curIdx], &crossPoint))
			{
				for (size_t pIdx = beginIdx, newIdx = 0; pIdx < curIdx; pIdx++, newIdx++)
				{
					m_points[newIdx] = m_originalPoints[pIdx];
				}
				if (!crossOnPrePoint)
				{
					m_points[resultSize - 1] = crossPoint;
					m_originalPoints[curIdx - 1] = crossPoint;
				}
				curIdx--;               //回退一个点，以便下次遍历的时候可以从交点开始计算
			}
			else
			{
				for (size_t pIdx = beginIdx, newIdx = 0; pIdx <= curIdx; pIdx++, newIdx++)
				{
					m_points[newIdx] = m_originalPoints[pIdx];
				}
			}

			if (resultSize >= 2)
			{
				callback(m_points, resultSize, userData);
			}
			// 重置变量，重新遍历剩余路段
			beginIdx = curIdx;
			curIdx++;
			isStartPoint = true;
			isAllPointsInOneGrid = true;
			if (beginIdx >= n - 1)
			{
				isAllPointsInOneGrid = false;
				break;
			}
		}

		else
		{
			curIdx++;
			isStartPoint = false;
		}

	} // End while

	if (isAllPointsInOneGrid)         //收集最后剩余的一些点
	{
		size_t resultSize = curIdx - beginIdx;
		for (size_t i = 0; i < resultSize; i++)
		{
			m_points[i] = m_originalPoints[beginIdx++];
		}
		callback(m_points, resultSize, userData);
	}

	return true;
}

forceinline void LineSplitter::_reservePoints(size_t newSize)
{
	if (newSize > m_pointsMaxSize)
	{
		newSize += 1000;
		m_points = (Point*)realloc(m_points, newSize * sizeof(Point));
		m_pointsMaxSize = newSize;
	}
}
