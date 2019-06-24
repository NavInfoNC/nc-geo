#include "stdafx.h"
#include "split_algorithms/polygon_marker.h"
#include "split_algorithms/static_polygon.h"

::testing::AssertionResult isNear(Point p1, Point expect, int epsilon)
{
	if (abs(p1.x - expect.x) <= epsilon && abs(p1.y - expect.y) <= epsilon)
		return ::testing::AssertionSuccess();
	else
		return ::testing::AssertionFailure() << "expect:" << expect.x << ',' << expect.y
		<< "actual: " << p1.x << ',' << p1.y;
}

TEST(PolygonMarkerTest, inscribedCircle)
{
	PolygonMarker* finder = new PolygonMarker();
	finder->setCentralPointPrecision(1);
	Point polygon[] = {
		{ 90000, 90000 },
		{ 100000, 90000 },
		{ 100000, 100000 },
		{ 90000, 100000 },
	};

	Point polygon1[] = {
		{ -100000, -100000 },
		{ 100000, -100000 },
		{ 100000, 200000 },
		{ 0, 100000 },
		{ -100000, 200000 },
	};

	Point polygon4[] = {
		{ 357, 84 }, { 280, 291 }, { 245, 476 },
		{ 238, 676 }, { 490, 673 }, { 507, 447 },
		{ 593, 435 }, { 593, 286 }, { 769, 265 },
		{762, 72}
	};

	StaticPolygon p, p1, p2, p3;
	p.initWithPointsNoCopy(polygon, 4);
	p1.initWithPointsNoCopy(polygon1, 5);
	p3.initWithPointsNoCopy(polygon4, 10);
	StaticPolygon *pp = &p, *pp1 = &p1, *pp3 = &p3;

	Point center;
	int radius;

	finder->findCentralMarkPoint(&pp, 1, &center, &radius);
	EXPECT_TRUE(isNear(center, Point_make(95000, 95000), 1));

	finder->findCentralMarkPoint(&pp1, 1, &center, &radius);
	EXPECT_TRUE(isNear(center, Point_make(0, 0), 1));

	finder->findCentralMarkPoint(&pp3, 1, &center, &radius);
	EXPECT_TRUE(isNear(center, Point_make(438, 307), 1));

	delete finder;
}

TEST(PolygonMarkerTest, PolygonWithHole)
{
	PolygonMarker* finder = new PolygonMarker();
	finder->setCentralPointPrecision(1);

	Point contour[] = {
		{0, 0},
		{210, 0},
		{210, 100},
		{0, 100}
	};
	Point hole1[] = {
		{ 100, 10 },
		{ 110, 10 },
		{ 110, 90 },
		{ 100, 90 }
	};
	Point hole2[] = {
		{ 10, 10 },
		{ 20, 10 },
		{ 20, 20 },
		{ 10, 20 }
	};

	StaticPolygon p1, p2, p3;
	p1.initWithPointsNoCopy(contour, 4);
	p2.initWithPointsNoCopy(hole1, 4);
	p3.initWithPointsNoCopy(hole2, 4);

	Point center;
	int radius;
	StaticPolygon* polys[] = { &p1, &p2, &p3 };
	finder->findCentralMarkPoint(polys, 3, &center, &radius);
	EXPECT_TRUE(isNear(center, Point_make(160, 50), 1));
	EXPECT_NEAR(radius, 50, 1);

	Point contour1[] = {
		{0, 0},
		{100, 0},
		{100, 100},
		{0, 100}
	};

	Point hole3[] = {
		{10, 10},
		{90, 10},
		{90, 90},
		{10, 90}
	};

	StaticPolygon p4, p5;
	p4.initWithPointsNoCopy(contour1, 4);
	p5.initWithPointsNoCopy(hole3, 4);

	StaticPolygon* polys1[] = { &p4, &p5 };
	finder->findCentralMarkPoint(polys1, 2, &center, &radius);
	EXPECT_EQ(radius, 5);

	delete finder;
}
