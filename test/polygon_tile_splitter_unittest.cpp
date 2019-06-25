#include "gtest/gtest.h"
#include "polygon_tile_splitter.h"
#include <stdio.h>

TEST(PolygonTileSplitter, basic)
{
	PolygonTileSplitter* splitter = new PolygonTileSplitter();
	StaticPolygon** pieces;

	Point points[3] = { {0, 0}, { 100, 0 }, { 0, 100 } };
	Point results[3][7] = { { { 0, 0 }, { 50, 0 }, { 50, 50 }, { 0, 50 }, { 0, 0 }, { 50, 0 }, { 50, 50 } },
	{ { 50, 0 }, { 100, 0 }, { 50, 50 }, { 50, 0 }, { 100, 0 }},
	{ { 0, 50 }, { 50, 50 }, { 0, 100 }, { 0, 50 }, { 50, 50 } } };
	int resultLen[3] = { 4, 3, 3 };
	StaticPolygon polygon;
	polygon.initWithPointsNoCopy(points, 3);
	
	splitter->setTileSize(50);
	int num;
	pieces = splitter->split(&polygon, &num);

	EXPECT_EQ(3, num);
	if (num == 3)
	{
		int trueNum = 0;
		for (int i = 0; i < 3; ++i)
		{
			bool flag = false;
			for (int j = 0; j < 3; ++j)
			{
				if (pieces[j]->pointNumber() == resultLen[i])
				{
					for (int k = 0; k < resultLen[i]; ++k)
					{
						if (memcmp(results[i] + k, pieces[j]->points(), resultLen[i] * sizeof(Point)) == 0)
						{
							trueNum++;
							flag = true;
							break;
						}
					}
				}
				if (flag)
					break;
			}
		}
		EXPECT_EQ(trueNum, 3);
	}
	delete splitter;
}

TEST(PolygonTileSplitter, selfIntersectionCase)
{
	PolygonTileSplitter* splitter = new PolygonTileSplitter();
	StaticPolygon **pieces;

	Point points[5] = { { 40, 0 }, { 50, 80 }, { 80, 0 }, { 0, 30 }, { 100, 50 } };
	StaticPolygon polygon;
	int num;
	polygon.initWithPoints(points, 5);
	splitter->setTileSize(20);
	pieces = splitter->split(&polygon, &num);

	EXPECT_EQ(NULL, pieces);

	delete splitter;
}

TEST(PolygonTileSplitter, polygonNear180Degree)
{

	Point points[] = {
		{ 1762726281, 539058125 },
		{ 1753181909, 529712634 },
		{ 1762925162, 535180843 },
		{ 1772668295, 540648933 },
		{ 1776048554, 542438502 }
	};

	PolygonTileSplitter* splitter = new PolygonTileSplitter();
	StaticPolygon polygon;

	polygon.initWithPoints(points, element_of(points));

	int polygonNum;
	StaticPolygon** resultPolygons;

	splitter->setTileSize(536870912);
	resultPolygons = splitter->split(&polygon, &polygonNum);

	ASSERT_EQ(2, polygonNum);

	StaticPolygon* p0 = resultPolygons[0];
	StaticPolygon* p1 = resultPolygons[1];

	EXPECT_EQ(5, p0->pointNumber());
	EXPECT_EQ(4, p1->pointNumber());

	delete splitter;
}

TEST(PolygonTileSplitter, deadLoop)
{
	Point points[] = {
		{ 1682195525, -108964751 },
		{ 1566072494, -11532703 },
		{ 1564282805, -17299054 },
		{ 1573628415, -20082789 },
		{ 1579593648, -27042187 },
		{ 1674838484, -101806473 }
	};

	PolygonTileSplitter* splitter = new PolygonTileSplitter();
	StaticPolygon polygon;

	polygon.initWithPoints(points, element_of(points));

	int polygonNum;
	StaticPolygon** resultPolygons;

	splitter->setTileSize(536870912);
	resultPolygons = splitter->split(&polygon, &polygonNum);

	ASSERT_EQ(2, polygonNum);

	StaticPolygon* p0 = resultPolygons[0];
	StaticPolygon* p1 = resultPolygons[1];

	EXPECT_EQ(4, p0->pointNumber());
	EXPECT_EQ(6, p1->pointNumber());

	delete splitter;
}

//class PolygonTileSplitterPerformanceTest : public testing::Test
//{
//public:
//	virtual void SetUp() override
//	{
//		FILE* pF;
//		fopen_s(&pF, "data/china.mif", "r");
//
//		char buffer[128];
//		fgets(buffer, 128, pF);
//
//		int count = atoi(buffer);
//		Point* pts = new Point[count];
//
//		count = 0;
//		while (fgets(buffer, 128, pF))
//		{
//			pts[count].x = int(atof(buffer) * 1e5);
//			pts[count].y = int(atof(strchr(buffer, ' ') + 1) * 1e5);
//			count++;
//		}
//		m_polygon.initWithPoints(pts, count);
//		delete pts;
//		fclose(pF);
//	}
//	virtual void TearDown() override
//	{
//
//	}
//
//protected:
//	StaticPolygon m_polygon;
//};
//
//TEST_F(PolygonTileSplitterPerformanceTest, performance)
//{
//	PolygonTileSplitter* splitter = new PolygonTileSplitter();
//	splitter->setTileSize(1000);
//	int count;
//	splitter->split(&m_polygon, &count);
//
//	delete splitter;
//}