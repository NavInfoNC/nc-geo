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
#include "polygon_merger.h"
#include "static_polygon.h"

StaticPolygon** PolygonMerger::mergePolygons(const std::vector<StaticPolygon*>& polygons, int& outPolygonCount)
{
	_reset();

	// add all segments into a segment set
	for (size_t i = 0; i < polygons.size(); i++)
	{
		_buildSegmentSet(polygons[i]);
	}
	_generateContours();

	outPolygonCount = (int)m_result.size();
	return m_result.data();
}

PolygonMerger::~PolygonMerger()
{
	_reset();
}

void PolygonMerger::_buildSegmentSet(StaticPolygon* poly)
{
	const Point* pts = poly->points();
	int ptsNum = poly->pointNumber();
	
	for (int i = ptsNum - 1, j = 0; j < ptsNum; i = j++)
	{
		PointSet& ptSet = m_segments[pts[j]];
		// if a duplicated edge (with inverse direction) exist, remove that edge
		if (ptSet.find(pts[i]) != ptSet.end())
		{
			ptSet.erase(pts[i]);
			if (ptSet.empty())
				m_segments.erase(pts[j]);
			continue;
		}
		m_segments[pts[i]].insert(pts[j]);
	}
}

void PolygonMerger::_generateContours()
{
	// traversal all unique segments, link them to generate contours of merge result
	SegmentSet::iterator segIter;
	PointSet::iterator ptIter;

	segIter = m_segments.begin();

	while (segIter != m_segments.end())
	{
		Contour c;
		c.push_back(segIter->first);
		while (true)
		{
			PointSet& ptSet = segIter->second;
			ptIter = ptSet.begin();
			c.push_back(*ptIter);

			ptSet.erase(ptIter);
			if (ptSet.empty())
				m_segments.erase(segIter);

			// if no following segments, a contour has completed
			if ((segIter = m_segments.find(c.back())) == m_segments.end())
			{
				StaticPolygon* poly = new StaticPolygon();
				poly->initWithPoints(c.data(), (int)c.size() - 1);
				m_result.push_back(poly);
				segIter = m_segments.begin();
				break;
			}
		}
	}
}

void PolygonMerger::_reset()
{
	for (StaticPolygon* poly : m_result)
		delete poly;
	m_result.clear();
	m_segments.clear();
}
