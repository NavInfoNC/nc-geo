#include "gtest/gtest.h"
#include "gtest/gtest.h"
#include "polygon_merger.h"
#include "static_polygon.h"

class PolygonMergerTest: public ::testing::Test
{
public:
	void SetUp()
	{
		m_merger = new PolygonMerger();
	}
	void TearDown()
	{
		delete m_merger;
	}
protected:
	PolygonMerger* m_merger;
};

::testing::AssertionResult isPolygonEqual(StaticPolygon* poly, const Point* pts, int ptsCount)
{
	const Point* pts1 = poly->points();
	int ptsCount1 = poly->pointNumber();

	if (ptsCount1 != ptsCount)
		return ::testing::AssertionFailure();

	for (int i = 0; i < ptsCount; i++)
	{
		if (pts[i].x != pts1[i].x || pts[i].y != pts1[i].y)
			return ::testing::AssertionFailure();
	}

	return ::testing::AssertionSuccess();
}

TEST_F(PolygonMergerTest, basic)
{
	Point pts1[] = { 
		Point_make(100, 100), 
		Point_make(100, 200),
		Point_make(200, 200),
		Point_make(200, 100)
	};

	Point pts2[] = {
		Point_make(200, 100), 
		Point_make(200, 200),
		Point_make(300, 200),
		Point_make(300, 100)
	};

	StaticPolygon p1, p2;
	p1.initWithPointsNoCopy(pts1, 4);
	p2.initWithPointsNoCopy(pts2, 4);
	std::vector<StaticPolygon*> polys;
	polys.push_back(&p1);
	polys.push_back(&p2);

	int resultNum;
	StaticPolygon** result = m_merger->mergePolygons(polys, resultNum);
	EXPECT_EQ(resultNum, 1);

	Point ptsResult[] = {
		{100, 100},
		{100, 200},
		{200, 200},
		{300, 200},
		{300, 100},
		{200, 100},
	};
	EXPECT_TRUE(isPolygonEqual(result[0], ptsResult, 6));
}

TEST_F(PolygonMergerTest, withHole)
{
	Point pts1[] = { 
		Point_make(10, 20), 
		Point_make(30, 20),
		Point_make(25, 25),
		Point_make(30, 30),
		Point_make(30, 40),
		Point_make(10, 40)
	};

	Point pts2[] = {
		Point_make(30, 20), 
		Point_make(50, 20),
		Point_make(50, 40),
		Point_make(30, 40),
		Point_make(30, 30),
		Point_make(35, 25)
	};

	StaticPolygon p1, p2;
	p1.initWithPointsNoCopy(pts1, 6);
	p2.initWithPointsNoCopy(pts2, 6);
	std::vector<StaticPolygon*> polys;
	polys.push_back(&p1);
	polys.push_back(&p2);

	int resultNum;
	StaticPolygon** result = m_merger->mergePolygons(polys, resultNum);
	EXPECT_EQ(resultNum, 2);
	
	Point resultPts1[] = {
		{10, 20},
		{30, 20},
		{50, 20},
		{50, 40},
		{30, 40},
		{10, 40},
	};

	Point resultPts2[] = {
		{25, 25},
		{30, 30},
		{35, 25},
		{30, 20}
	};

	if (result[0]->pointNumber() == 4)
	{
		EXPECT_TRUE(isPolygonEqual(result[0], resultPts2, 4));
		EXPECT_TRUE(isPolygonEqual(result[1], resultPts1, 6));
	}
	else
	{
		EXPECT_TRUE(isPolygonEqual(result[1], resultPts2, 4));
		EXPECT_TRUE(isPolygonEqual(result[0], resultPts1, 6));
	}
}


TEST_F(PolygonMergerTest, complicate)
{
	Point pts1[] = { 
		Point_make(0, 0),
		Point_make(50, 0),
		Point_make(50, 25),
		Point_make(25, 50),
		Point_make(0, 50)
	};

	Point pts2[] = {
		Point_make(50, 25), 
		Point_make(50, 0),
		Point_make(100, 0),
		Point_make(100, 50),
		Point_make(75, 50),
	};

	Point pts3[] = {
		Point_make(75, 50), 
		Point_make(100, 50),
		Point_make(100, 100),
		Point_make(50, 100),
		Point_make(50, 75),
	};


	Point pts4[] = {
		Point_make(50, 75), 
		Point_make(50, 100),
		Point_make(100, 0),
		Point_make(0, 50),
		Point_make(25, 50),
	};

	StaticPolygon p1, p2, p3, p4;
	p1.initWithPointsNoCopy(pts1, 5);
	p2.initWithPointsNoCopy(pts2, 5);
	p3.initWithPointsNoCopy(pts3, 5);
	p4.initWithPointsNoCopy(pts4, 5);

	std::vector<StaticPolygon*> polys;
	polys.push_back(&p1);
	polys.push_back(&p2);
	polys.push_back(&p3);
	polys.push_back(&p4);

	int resultNum;
	StaticPolygon** result = m_merger->mergePolygons(polys, resultNum);
	
	EXPECT_EQ(resultNum, 2);
	Point resultPts1[] = {
		{0, 0},
		{50, 0},
		{100, 0},
		{100, 50},
		{100, 100},
		{50, 100},
		{100, 0},
		{0, 50}
	};

	Point resultPts2[] = {
		{50, 25},
		{25, 50},
		{50, 75},
		{75, 50}
	};

	EXPECT_TRUE(isPolygonEqual(result[0], resultPts1, 8));
	EXPECT_TRUE(isPolygonEqual(result[1], resultPts2, 4));
}