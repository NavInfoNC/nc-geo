Introduction
------------

**nc-geo** is a collection of algorithms for geometry processing(Polygon splitting, finding the largest circle in a polygon, etc.) used in NaviCore team. 
We implement some generic containers to make it more efficient.

Features
--------

* Well designed and easy to use.
* Very efficient.

 	For instance. ``PolygonTileSplitter`` can split the whole China's polygon (19893 points) by approximate 1 square km tiles in lesss than 2 seconds, resulting more than 9500000 polygons. It's serveral hundred times faster than GPC.

* Highly readable source code.

In This Project
---------------

Basic gometry types and their operatoions including:

* Point
  	Representation of 2D integer point.
* Rect
	Representation of 2D integer rectangle.
* StaticPolygon
	A collection of ``Points`` to represent the contour of a polygon, inmutable after creation.
* MutablePolygon
	Similar to ``StaticPolygon``, but adding or removing points is allowed.

Polygon/polyline algorithms including:

* PolygonTileSplitter
	Split polygon by tiles with specified tile size.
* PolylineTileSplitter
	Split polyline into segments by tiles with specified tile size.
* PolylinePolygonSplitter
	Split polyline into segments with a given polygon.
* PolygonMarker
	Calculate the center and radius of largest inscribed circle in polygon.
* PolygonMerger
	Given a set of polygons, remove their common edges(exactly same edge).

Generic containers and util:

* Vector
* Hashmap
* SmallObjectAllocator

Usage
-----

Each algortihm has a coresponding unit test, you can get its usage by reading testing code. All unit tests of these classes are in ``test``. For instance ``PolygonTileSplitter``:

.. code-block:: cpp

	PolygonTileSplitter* splitter = new PolygonTileSplitter();

	Point points[3] = { {0, 0}, { 100, 0 }, { 0, 100 } };
	StaticPolygon polygon;
	polygon.initWithPointsNoCopy(points, 3);
	
	splitter->setTileSize(50);
	int num;
	StaticPolygon** pieces = splitter->split(&polygon, &num);

Just a few lines of code.