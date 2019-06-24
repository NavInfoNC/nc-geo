Split Algorithms
========================

A collection of common polygon/polyline alogrithms used in NaviCore team. 
We implement our own generic containers algorithms, so this lib is independent with STL and more efficient.

In This Project
---------------

Basic gometry types and their operatoions including:

	* Point
	* Rect
	* StaticPolygon
	* MutablePolygon

Polygon/polyline algorithms including:

	* PolygonTileSplitter
	* PolylineTileSplitter
	* PolylinePolygonSplitter
	* PolygonMarker
	* PolygonMerger

Generic containers and algorithms including:

	* Vector
	* Hashmap
	* Heap
	* Common sorting and searching algorithms

And unit test of all classes in ``split_algorithms_test``.

Usage
-----

Each algortihm has a coresponding unit test, you can get its usage by reading testing code.