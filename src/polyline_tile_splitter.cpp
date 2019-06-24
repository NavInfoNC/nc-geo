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

	// p1���ھ����ڲ�����p2���ھ����ⲿ
	//CQ_ASSERT(code2 != Position_none);

	if ((code2 & Position_left) != Position_none) //< ����߽��ཻ
	{
		pOut->x = rect.left;
		// ����������ԭ��
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
	else if ((code2 & Position_right) != Position_none) //< ���ұ߽��ཻ
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

	if ((code2 & Position_bottom) != Position_none) //< ���±߽��ཻ
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
	else if ((code2 & Position_top) != Position_none) //< ���ϱ߽��ཻ
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
��ȡһ��Grid�������һ��Grid��λ�ã���������ȡConhe-Sutherland�㷨����ı��뺯����
@param [in] targetGridId
��Ҫ��λ�ù�ϵ����
@param [in] baseGridridId
��׼����
@return
���λ�ù�ϵ
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
�жϵ�ǰ���Ƿ��ڱ߽���
@param [in] p
������
@param [in] rct
��������
@return
����㲻�ھ��εı�Ե��, ����Position_none, ���򷵻ض�Ӧ�ı߻��
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
������һ�����λ����ȷ����ǰ�߽��ϵ��������ĸ���
@param [in] preGridId
��ǰ�����ڱ߽����һ������ID
@param [in] p
��ǰҪ�ж��ĵ�
@param [in] rct
��Ҫ�ж��ĸ��ӵİ����
@param [in] nextPointPosition
��һ������Ե�ǰ���ӵ�λ��
@return
���������ĸ���ID
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
ʹ��Conhe-Sutherland�㷨�з�һ��·��
ÿ��ִ����˺���������һ��·����ȫ�з�
�㷨�������̣�
1���ж��߶��Ƿ����ӡ�
2���������ӣ��߶ε���ʼ��Ϊ·�ε�һ������Ϊ���㣬����ݵڶ��������������õ�һ����(����)���ڵĸ��ӣ����������ҡ�
3������н����Ҳ�Ϊ2�е��������ô�ɴ˵㽫·���з֣��зֳ�������Ч·�η�����Ч����Map�У�δ����·��
���µ�����·�μ�����⣬���˴�ϵĽ�����Ϊ��·�ε���㣬���´�1��ʼִ�У�ֱ��·���з���ɡ�
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
			// ������һ�������ڱ߽��Ͻ�����ֳ���Ϊ0���߶�
			// ��ʼ������ڱ߽���ͨ�������жϸ��ӹ���
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
			// �ж���ʼ�����ֹ���Ƿ��д��ڽ����ϵ����
			// �����ʼ���ڱ�Ե�ϣ�������ֹ���λ��ȷ����ʼ���������ӣ�����ͼ����ʼ��1, ��ֹ��Ϊ2
			// ���ڸ��ӱ�����Ϊ2L2C���ӣ������Ǹ���Լ�����������1L2C����
			Point crossPoint;
			bool crossOnPrePoint;
			getCrosspoint(preGridId, &m_originalPoints[curIdx - 1], &m_originalPoints[curIdx], &crossPoint);
			crossOnPrePoint = Point_equal(&m_originalPoints[curIdx - 1], &crossPoint);

			// ��ʼ���ڱ߽�����������ʼ�������һ�����λ�������·������
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

			// �����ǰ��û�����ڱ߽��ϣ���ӽ���������·��
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
				curIdx--;               //����һ���㣬�Ա��´α�����ʱ����Դӽ��㿪ʼ����
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
			// ���ñ��������±���ʣ��·��
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

	if (isAllPointsInOneGrid)         //�ռ����ʣ���һЩ��
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
