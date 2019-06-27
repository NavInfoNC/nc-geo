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
#pragma once

#include "static_polygon.h"

class LineCutterImple;

// Using polygon to cut lines, remains segments outside or inside the polygon.
//
// Can deal with polygons with self intersections. Using ray method to determine 
// whether the point is inside the polygon. But can't deal with polygons that 
// deteriorate to a line, cuz I think it is useless.
//
// The points on the edge of the polygon is considered to be inside the polygon.
class PolylinePolygonSplitter
{
public:
	PolylinePolygonSplitter();
	~PolylinePolygonSplitter();

	typedef void(*SplitResultCallback)(const Point* result, size_t n, void* userData);

	void split(StaticPolygon& poly, const Point* lines, size_t n, bool isInsideRemains, 
		void* userData, SplitResultCallback callback);

private:
	LineCutterImple* m_imple;
};
