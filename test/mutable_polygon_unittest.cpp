#include "gtest/gtest.h"
#include "mutable_polygon.h"

TEST(MutablePolygon, basic)
{
	MutablePolygon o;
	o.reserve(10);
	EXPECT_EQ(10, o.reservedSize());
	for (int i = 0; i < 10; i++)
	{
		o.addPoint(Point_make(0, 0));
		EXPECT_EQ(10, o.reservedSize());
	}
	
	o.addPoint(Point_make(0, 0));
	EXPECT_EQ(16, o.reservedSize());
}
