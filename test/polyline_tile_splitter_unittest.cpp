#include "gtest/gtest.h"
#include "polyline_tile_splitter.h"
#include "cq_vector.h"

struct Line
{
public:
	forceinline Line() : m_points(NULL) {}
	forceinline Line(const Line* r, const Point* points, size_t n) {
		m_type = r->m_type;
		size_t copySize = sizeof(Point) * n;
		m_points = (Point*)malloc(copySize);
		memcpy(m_points, points, copySize);
		m_pointNum = n;
	}
	forceinline ~Line() { free(m_points); }
	void setPoints(const Point* points, size_t n) {
		free(m_points);
		size_t copySize = sizeof(Point) * n;
		m_points = (Point*)malloc(copySize);
		memcpy(m_points, points, copySize);
		m_pointNum = n;
	}

	int m_type;
	Point* m_points;
	size_t m_pointNum;
};

TEST(LineSplitter, splitByTileSize)
{
	LineSplitter splitter;
	static Point points[2] = { { 0, 0 }, { 200, 200 } };
	static Point t1[] = { { 0, 0 }, { 100, 100 } };
	static Point t2[] = { { 100, 100 }, { 200, 200 } };
	static Point* ts[] = { t1, t2 };
	static size_t numbers[] = { element_of(t1), element_of(t2) };

	static size_t count = 0;

	splitter.setTileSize(100);
	splitter.split(points, element_of(points), NULL, [](const Point* points, size_t n, void*){
		EXPECT_EQ(numbers[count], n);
		EXPECT_TRUE(memcmp(points, ts[count], sizeof(Point) * numbers[count]) == 0);
		count++;
	});
}

struct UserData
{
	const Line* oriLine;
	Vector<Line*>* newLines;
};

void splitPolyline(LineSplitter* splitter, Line* polyline, Vector<Line*>* newLines)
{
	UserData userData;
	userData.oriLine = polyline;
	userData.newLines = newLines;
	splitter->split(polyline->m_points, polyline->m_pointNum, &userData, [](const Point* points, size_t n, void* userData){
		UserData* data = (UserData*)userData;
		const Line* oldLine = data->oriLine;
		Vector<Line*>* newLines = data->newLines;

		Line* newLine = new Line(oldLine, points, n);
		newLines->append(newLine);
	});
}

TEST(LineSplitter, splitByNdsTile)
{
	LineSplitter splitter;
	Point points[4] = { { 0, 0 }, { 100, 100 }, { 400, 400 }, { 2000, 2000 } };

	Line line;
	line.m_type = 5;
	line.setPoints(points, 4);

	splitter.setTileSize(100);

	Vector<Line*> newLines;
	splitPolyline(&splitter, &line, &newLines);

	EXPECT_EQ(20, newLines.size());
	for (size_t i = 0; i < newLines.size(); i++)
	{
		EXPECT_EQ(5, newLines[i]->m_type);
		EXPECT_EQ(2, newLines[i]->m_pointNum);
		EXPECT_EQ(i * 100, newLines[i]->m_points[0].x);
		EXPECT_EQ(i * 100, newLines[i]->m_points[0].y);
		EXPECT_EQ((i + 1) * 100, newLines[i]->m_points[1].x);
		EXPECT_EQ((i + 1) * 100, newLines[i]->m_points[1].y);
	}

	for each(Line* line in newLines)
	{
		delete line;
	}
}

TEST(LineSplitter, crashCase1)
{
	LineSplitter splitter;
	static Point points[2] = {
		{1388839714, 478815959},
		{1388835777, 478815840}
	};
	static size_t count = 0;
	splitter.setTileSize(262144); // 2^18, grid side length of NDS level 13
	splitter.split(points, element_of(points), NULL, [](const Point* newPoints, size_t n, void*){
		EXPECT_EQ(2, n);
		if (count == 0)
		{
			EXPECT_TRUE(Point_equal(&newPoints[0], &points[0]));
		}
		else
		{
			EXPECT_TRUE(Point_equal(&newPoints[1], &points[1]));
		}
		count++;
	});
}

TEST(LineSplitter, crashCase2)
{
	LineSplitter splitter;
	static Point points[] = {
		{1386474738, 479474759},
		{1386475335, 479469987},
		{1386479391, 479461516},
		{1386481181, 479457699},
		{1386481538, 479456506},
		{1386481777, 479455193},
		{1386482135, 479452568}
	};
	
	splitter.setTileSize(262144); // 2^18, grid side length of NDS level 13
	splitter.split(points, element_of(points), NULL, [](const Point*, size_t, void*){
	});
}

TEST(LineSplitter, crashCase3)
{
	LineSplitter splitter;
	Point points[4] = {
		{ 90, 90 },
		{ 110, 60 },
		{ 100, 50 },
		{ 90, 40 }
	};
	static Point t1[] = { { 90, 90 }, { 100, 75 } };
	static Point t2[] = { { 100, 75 }, { 110, 60 }, { 100, 50 } };
	static Point t3[] = { { 100, 50 }, { 90, 40 } };
	static Point* ts[] = { t1, t2, t3 };
	static size_t numbers[] = { element_of(t1), element_of(t2), element_of(t3) };

	static size_t count = 0;

	splitter.setTileSize(100);
	splitter.split(points, element_of(points), NULL, [](const Point* points, size_t n, void*){
		EXPECT_EQ(numbers[count], n);
		EXPECT_TRUE(memcmp(points, ts[count], sizeof(Point) * numbers[count]) == 0);
		count++;
	});
}
