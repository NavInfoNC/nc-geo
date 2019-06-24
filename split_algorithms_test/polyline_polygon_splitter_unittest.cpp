#include "stdafx.h"
#include "split_algorithms/polyline_polygon_splitter.h"
#include "split_algorithms/static_polygon.h"
#include "split_algorithms/cq_vector.h"

class Checker
{
public:
	Checker()
	{
		m_hasResult = false;
		m_testCount = 0;
	}

	void clear()
	{
		m_testCount = 0;
		m_expectedResult.clear();
		m_expectedResLen.clear();
	}

	void addResult(Point* p, int n)
	{
		m_expectedResult.append(p);
		m_expectedResLen.append(n);
	}

	static void check(const Point* result, size_t n, void* userData)
	{
		Checker *o = (Checker*)userData;
		o->m_hasResult = true;
		EXPECT_LT(o->m_testCount, o->m_expectedResLen.size());
		EXPECT_EQ(o->m_expectedResLen.at(o->m_testCount), n);
		for (int i = 0; i < n; ++i)
		{
			EXPECT_EQ(o->m_expectedResult.at(o->m_testCount)[i].x, result[i].x);
			EXPECT_EQ(o->m_expectedResult.at(o->m_testCount)[i].y, result[i].y);
		}
		o->m_testCount++;
	}

	bool m_hasResult;

private:
	Vector <Point*> m_expectedResult;
	Vector <int> m_expectedResLen;
	int m_testCount;
};

TEST(PolylinePolygonSplitter, basicTest1)
{
	static Point polyPoint[] = { { 0, 0 }, { 0, 2 }, { 2, 2 }, { 2, 0 } };
	StaticPolygon poly;
	poly.initWithPointsNoCopy(polyPoint, 4);
	
	Point lines[] = { { 1, 1 }, { 3, 1 } };
	static Point res[2] = { { 2, 1 }, { 3, 1 } };
	Checker checker;
	checker.addResult(res, 2);

	PolylinePolygonSplitter cutter;
	cutter.split(poly, lines, 2, false, &checker, Checker::check);
}

TEST(PolylinePolygonSplitter, basicTest2)
{
	static Point polyPoint[] = { { 0, 0 }, { 0, 2 }, { 2, 2 }, { 2, 0 } };
	StaticPolygon poly;
	poly.initWithPointsNoCopy(polyPoint, 4);

	Point lines[] = { { 1, 3 }, { 1, -1 } };
	static Point res[2][2] = { { { 1, 3 }, { 1, 2 } }, { { 1, 0 }, { 1, -1 } } };
	Checker checker;
	checker.addResult(res[0], 2);
	checker.addResult(res[1], 2);

	PolylinePolygonSplitter cutter;
	cutter.split(poly, lines, 2, false, &checker, Checker::check);
}

TEST(PolylinePolygonSplitter, meetVertex1)
{
	static Point polyPoint[] = { { -1, 0 }, { 0, -1 }, { 1, 0 }, { 0, 1 } };
	StaticPolygon poly;
	poly.initWithPointsNoCopy(polyPoint, 4);

	Point lines[] = { { 0, 0 }, { 2, 0 } };
	static Point res[2] = { { 1, 0 }, { 2, 0 } };
	Checker checker;
	checker.addResult(res, 2);

	PolylinePolygonSplitter cutter;
	cutter.split(poly, lines, 2, false, &checker, Checker::check);
}

TEST(PolylinePolygonSplitter, meetVertex2)
{
	static Point polyPoint[] = { { -1, 0 }, { 0, -1 }, { 1, 0 }, { 0, 1 } };
	StaticPolygon poly;
	poly.initWithPointsNoCopy(polyPoint, 4);

	Point lines[] = { { 0, -2 }, { 0, 2 } };
	static Point res[2][2] = { { { 0, -2 }, { 0, -1 } }, { { 0, 1 }, { 0, 2 } } };
	Checker checker;
	checker.addResult(res[0], 2);
	checker.addResult(res[1], 2);

	PolylinePolygonSplitter cutter;
	cutter.split(poly, lines, 2, false, &checker, Checker::check);
}

TEST(PolylinePolygonSplitter, polyLine1)
{
	static Point polyPoint[] = { { -2, -2 }, { 2, -2 }, { 2, 2 }, { -2, 2 } };
	StaticPolygon poly;
	poly.initWithPointsNoCopy(polyPoint, 4);

	Point lines[] = { { -3, 0 }, { 0, -3 }, { 3, 0 }, { 0, 3 }, { -3, 0 } };
	static Point res[5][3] = { { { -3, 0 }, { -2, -1 } }, 
		{ { -1, -2 }, { 0, -3 }, { 1, -2 } },
		{ { 2, -1 }, { 3, 0 }, { 2, 1 } }, 
		{ { 1, 2 }, { 0, 3 }, { -1, 2 } }, 
		{ { -2, 1 }, { -3, 0 } } };
	Checker checker;
	checker.addResult(res[0], 2);
	checker.addResult(res[1], 3);
	checker.addResult(res[2], 3);
	checker.addResult(res[3], 3);
	checker.addResult(res[4], 2);

	PolylinePolygonSplitter cutter;
	cutter.split(poly, lines, 5, false, &checker, Checker::check);
}

TEST(PolylinePolygonSplitter, polyLine2)
{
	static Point polyPoint[] = { { -1, -1 }, { 1, -1 }, { 1, 1 }, { -1, 1 } };
	StaticPolygon poly;
	poly.initWithPointsNoCopy(polyPoint, 4);

	Point lines[] = { { -2, 0 }, { 0, -2 }, { 2, 0 }, { 0, 2 }, { -2, 0 } };
	Checker checker;
	checker.addResult(lines, 5);

	PolylinePolygonSplitter cutter;
	cutter.split(poly, lines, 5, false, &checker, Checker::check);
}

TEST(PolylinePolygonSplitter, startFromVertex1)
{
	static Point polyPoint[] = { { 0, 0 }, { 0, 1 }, { 1, 1 }, { 1, 0 } };
	StaticPolygon poly;
	poly.initWithPointsNoCopy(polyPoint, 4);

	Point lines[] = { { 0, 1 }, { -1, 2 } };
	Checker checker;
	checker.addResult(lines, 2);

	PolylinePolygonSplitter cutter;
	cutter.split(poly, lines, 2, false, &checker, Checker::check);

	checker.clear();
	cutter.split(poly, lines, 2, true, &checker, Checker::check);
}

TEST(PolylinePolygonSplitter, startFromVertex2)
{
	static Point polyPoint[] = { { 0, 0 }, { 0, 1 }, { 1, 1 }, { 1, 0 } };
	StaticPolygon poly;
	poly.initWithPointsNoCopy(polyPoint, 4);

	Point lines[] = { { 0, 1 }, { 2, -1 } };
	static Point res[2][2] = { { { 1, 0 }, { 2, -1 } }, { { 0, 1 }, { 1, 0 } } };
	Checker checker;
	checker.addResult(res[0], 2);

	PolylinePolygonSplitter cutter;
	cutter.split(poly, lines, 2, false, &checker, Checker::check);

	checker.clear();
	checker.addResult(res[1], 2);
	cutter.split(poly, lines, 2, true, &checker, Checker::check);
}

TEST(PolylinePolygonSplitter, passEdge1)
{
	static Point polyPoint[] = { { 0, 0 }, { 0, 1 }, { 1, 1 }, { 1, 0 } };
	StaticPolygon poly;
	poly.initWithPointsNoCopy(polyPoint, 4);

	Point lines[] = { { 1, -1 }, { 1, 2 } };
	static Point res[2][2] = { { { 1, -1 }, { 1, 0 } }, { { 1, 1 }, { 1, 2 } } };
	Checker checker;
	checker.addResult(res[0], 2);
	checker.addResult(res[1], 2);

	PolylinePolygonSplitter cutter;
	cutter.split(poly, lines, 2, false, &checker, Checker::check);
}

TEST(PolylinePolygonSplitter, passEdge2)
{
	static Point polyPoint[] = { { 0, 0 }, { 0, 4 }, { 2, 4 }, { 2, 2 }, { 4, 2 }, { 4, 0 } };
	StaticPolygon poly;
	poly.initWithPointsNoCopy(polyPoint, 6);

	Point lines[] = { { 0, 2 }, { 6, 2 } };
	static Point res[2][2] = { { { 4, 2 }, { 6, 2 } }, { { 0, 2 }, { 4, 2 } } };
	Checker checker;
	checker.addResult(res[0], 2);

	PolylinePolygonSplitter cutter;
	cutter.split(poly, lines, 2, false, &checker, Checker::check);

	checker.clear();
	checker.addResult(res[1], 2);
	cutter.split(poly, lines, 2, true, &checker, Checker::check);
}

TEST(PolylinePolygonSplitter, startFromEdge1)
{
	static Point polyPoint[] = { { 0, 0 }, { 0, 4 }, { 2, 4 }, { 2, 2 }, { 4, 2 }, { 4, 0 } };
	StaticPolygon poly;
	poly.initWithPointsNoCopy(polyPoint, 6);

	Point lines[] = { { 3, 2 }, { -1, 2 } };
	static Point res[2][2] = { { { 0, 2 }, { -1, 2 } }, { { 3, 2 }, { 0, 2 } } };
	Checker checker;
	checker.addResult(res[0], 2);

	PolylinePolygonSplitter cutter;
	cutter.split(poly, lines, 2, false, &checker, Checker::check);

	checker.clear();
	checker.addResult(res[1], 2);
	cutter.split(poly, lines, 2, true, &checker, Checker::check);
}

TEST(PolylinePolygonSplitter, startFromEdge2)
{
	static Point polyPoint[] = { { 0, 0 }, { 0, 4 }, { 2, 4 }, { 2, 2 }, { 4, 2 }, { 4, 0 } };
	StaticPolygon poly;
	poly.initWithPointsNoCopy(polyPoint, 6);

	Point lines[] = { { 3, 2 }, { 6, 2 } };
	static Point res[2][2] = { { { 4, 2 }, { 6, 2 } }, { { 3, 2 }, { 4, 2 } } };
	Checker checker;
	checker.addResult(res[0], 2);

	PolylinePolygonSplitter cutter;
	cutter.split(poly, lines, 2, false, &checker, Checker::check);

	checker.clear();
	checker.addResult(res[1], 2);
	cutter.split(poly, lines, 2, true, &checker, Checker::check);
}

TEST(PolylinePolygonSplitter, withSelfIntersection)
{
	static Point polyPoint[] = { { -3, 0 }, { -5, -4 }, { 5, 4 }, { 3, 0 } };
	StaticPolygon poly;
	poly.initWithPointsNoCopy(polyPoint, 4);

	Point lines[] = { { -4, 0 }, { 8, 0 }, { 8, 4 }, { -8, -4 } };
	static Point res[3][4] = { { { -4, 0 }, { -3, 0 } }, 
		{ { 3, 0 }, { 8, 0 }, { 8, 4 }, { 4, 2 } }, 
		{ { -4, -2 }, { -8, -4 } } };
	static Point resInside[2][2] = { { { -3, 0 }, { 3, 0 } }, { { 4, 2 }, { -4, -2 } } };
	Checker checker;
	checker.addResult(res[0], 2);
	checker.addResult(res[1], 4);
	checker.addResult(res[2], 2);

	PolylinePolygonSplitter cutter;
	cutter.split(poly, lines, 4, false, &checker, Checker::check);

	checker.clear();
	checker.addResult(resInside[0], 2);
	checker.addResult(resInside[1], 2);
	cutter.split(poly, lines, 4, true, &checker, Checker::check);
}

TEST(PolylinePolygonSplitter, crashCase)
{
	static Point polyPoint[] = { { 1389318426, 476676233 },
	{ 1389300698, 476690200 },
	{ 1389315906, 476736884 },
	{ 1389320716, 476742774 },
	{ 1389382079, 476753522 },
	{ 1389394030, 476745459 },
	{ 1389396528, 476692876 },
	{ 1389389974, 476687333 },
	{ 1389318426, 476676233 } };
	StaticPolygon poly;
	poly.initWithPointsNoCopy(polyPoint, 9);

	Point lines[] = { { 1389331278, 476693665 },
	{ 1389329997, 476692502 },
	{ 1389328616, 476691248 },
	{ 1389327434, 476690175 },
	{ 1389326747, 476689552 },
	{ 1389326747, 476689552 },
	{ 1389325714, 476688614 },
	{ 1389323846, 476686919 },
	{ 1389321978, 476685223 },
	{ 1389320945, 476684284 },
	{ 1389320945, 476684284 },
	{ 1389320002, 476683422 },
	{ 1389318194, 476681767 },
	{ 1389316040, 476679796 },
	{ 1389314059, 476677983 } };

	PolylinePolygonSplitter cutter;
	cutter.split(poly, lines, 15, false, NULL, [](const Point*, size_t, void*){});
}

TEST(PolylinePolygonSplitter, allOutside)
{
	static Point polyPoint[] = { { 0, 0 }, { 2, 0 }, { 2, 2 }, { 0, 2 } };
	StaticPolygon poly;
	poly.initWithPointsNoCopy(polyPoint, 4);

	Point lines[] = { { 3, 4 }, { 4, 3 } };
	Checker checker;
	PolylinePolygonSplitter cutter;
	cutter.split(poly, lines, 2, true, &checker, Checker::check);
	EXPECT_FALSE(checker.m_hasResult);

	checker.addResult(lines, 2);
	cutter.split(poly, lines, 2, false, &checker, Checker::check);
	EXPECT_TRUE(checker.m_hasResult);
}

TEST(PolylinePolygonSplitter, allInside)
{
	static Point polyPoint[] = { { 0, 0 }, { 3, 0 }, { 3, 3 }, { 0, 3 } };
	StaticPolygon poly;
	poly.initWithPointsNoCopy(polyPoint, 4);

	Point lines[] = { { 1, 2 }, { 2, 1 } };
	Checker checker;
	PolylinePolygonSplitter cutter;
	cutter.split(poly, lines, 2, false, &checker, Checker::check);
	EXPECT_FALSE(checker.m_hasResult);

	checker.addResult(lines, 2);
	cutter.split(poly, lines, 2, true, &checker, Checker::check);
	EXPECT_TRUE(checker.m_hasResult);
}
