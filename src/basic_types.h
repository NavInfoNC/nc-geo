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

#	include <memory.h>
#	include <string.h>
#	include <stdlib.h>
#	include <stdio.h>
#	include <assert.h>
#	include <malloc.h>
#	include <math.h>
#	include <stddef.h>
#	include <stdarg.h>

typedef int int32;
typedef unsigned int uint32;
typedef short int16;
typedef unsigned short uint16;
typedef unsigned char uint8;
typedef signed char int8;

#	if defined(MAPBAR_WINCE_EVC)
typedef __int64 int64;
typedef unsigned __int64 uint64;
#else
typedef long long int64;
typedef unsigned long long uint64;
#endif

#if defined(_MSC_VER)
#	define forceinline __forceinline
#elif defined(__GNUC__)
#	define forceinline inline __attribute__((always_inline)) 
#endif

#ifndef INT_MIN
#	define INT_MIN (-2147483647 - 1)
#	define INT_MAX 2147483647
#endif

#ifndef UINT8_MAX
#	define UINT8_MAX 255
#endif

#ifndef UINT16_MAX
#	define UINT16_MAX 65535
#endif

#ifndef UINT_MAX
#	define UINT_MAX      0xffffffff    /* maximum unsigned int value */
#endif

#ifndef INT64_MIN
#	define INT64_MIN        (-9223372036854775807ll - 1)
#	define INT64_MAX        9223372036854775807ll
#endif

#ifndef UINT64_MAX
#	define UINT64_MAX       0xffffffffffffffffull
#endif

#define SHORT_MIN (-32768)
#define SHORT_MAX 32767

#ifndef UCHAR_MAX
#	define UCHAR_MIN	0
#	define UCHAR_MAX	255
#endif

#ifndef SIZE_T_MAX
#	define SIZE_T_MAX (size_t)(-1)
#endif

#define invalidIndex -1

#ifndef TRUE
#	define TRUE	1
#	define FALSE	0
#endif

#ifndef NULL
#	define NULL	((void*)0)
#endif

typedef struct Point {
	int32 x, y;
#ifdef __cplusplus
	forceinline void set(int x, int y) { this->x = x, this->y = y; }
	forceinline void invalidate() { this->x = INT_MAX, this->y = INT_MAX; }
	forceinline bool isValid() { return this->x != INT_MAX || this->y != INT_MAX; }
	forceinline void setZero() { this->x = this->y = 0; }
	forceinline bool isZero() { return this->x == 0 && this->y == 0; }
	forceinline void offset(int x, int y) { this->x += x; this->y += y; }
	forceinline void offset(Point offset) { this->x += offset.x; this->y += offset.y; }
#endif
} Point;

static forceinline Point Point_make(int x, int y) { Point pos; pos.x = x; pos.y = y; return pos; }

typedef struct Size {
	int32 width;
	int32 height;
} Size;


typedef struct Rect {
	int32 left, top, right, bottom;
#ifdef __cplusplus
	forceinline Size size() { Size o; o.width = right - left, o.height = bottom - top; return o; }
	forceinline int width() { return right - left; }
	forceinline int height() { return bottom - top; }
	forceinline Point center() { Point c; c.x = (left + right) / 2; c.y = (top + bottom) / 2; return c; }
	forceinline bool intersectsWithRect(Rect r) { return left < r.right && top < r.bottom && right > r.left && bottom > r.top; }
	forceinline bool wrapsRect(Rect r) { return r.left >= left && r.right <= right && r.top >= top && r.bottom <= bottom; }
	forceinline void invalidate() { left = top = INT_MAX, right = bottom = INT_MIN; }
	forceinline bool isValid() { return left <= right && top <= bottom; }
	forceinline void combinePoint(Point point) {
		if (point.x < this->left)
			this->left = point.x;
		if (point.y < this->top)
			this->top = point.y;
		if (point.x > this->right)
			this->right = point.x;
		if (point.y > this->bottom)
			this->bottom = point.y;
	}
	forceinline void combineRect(Rect rect) {
		combinePoint(Point_make(rect.left, rect.top));
		combinePoint(Point_make(rect.right, rect.bottom));
	}
	forceinline bool testPoint(Point pos) { return pos.x >= left && pos.y >= top && pos.x < right && pos.y < bottom; }
#endif
} Rect;

static forceinline Rect Rect_make(int l, int t, int r, int b) { Rect rct; rct.left = l, rct.top = t, rct.right = r, rct.bottom = b; return rct; }
static forceinline void Rect_set(Rect* o, int left, int top, int right, int bottom) { o->left = left; o->top = top; o->right = right; o->bottom = bottom; }

#define Rect_getWidth(rct) ((rct).right - (rct).left)
#define Rect_getHeight(rct) ((rct).bottom - (rct).top)
#define Rect_getCenter(rct, center) {(center)->x = (rct).left + ((rct).right - (rct).left) / 2; (center)->y = (rct).top + ((rct).bottom - (rct).top) / 2; }
#define Rect_isNegtiveMinimum(o) ((o)->left == INT_MAX && (o)->top == INT_MAX && (o)->right == INT_MIN && (o)->bottom == INT_MIN)
#define Rect_setAsNegtiveMinimum(o) (o)->left = (o)->top = INT_MAX; (o)->right = (o)->bottom = INT_MIN
#define Rect_invalidate(o) Rect_setAsNegtiveMinimum(o)
#define Rect_isValid(rct) ((rct)->left <= (rct)->right && (rct)->top <= (rct)->bottom)


#define Rect_combinePoint(o, point) \
	if ((point)->x < (o)->left)	\
		(o)->left = (point)->x;	\
	if ((point)->y < (o)->top)	\
		(o)->top = (point)->y;	\
	if ((point)->x > (o)->right)	\
		(o)->right = (point)->x;	\
	if ((point)->y > (o)->bottom)	\
		(o)->bottom = (point)->y;


#ifdef __cplusplus
extern "C++"
{
	forceinline bool operator == (Point l, Point r) {
		return l.x == r.x && l.y == r.y;
	}
	forceinline bool operator != (Point l, Point r) {
		return l.x != r.x || l.y != r.y;
	}
	forceinline bool operator == (Rect l, Rect r) {
		return l.left == r.left && l.top == r.top && l.right == r.right && l.bottom == r.bottom;
	}
	forceinline bool operator != (Rect l, Rect r) {
		return l.left != r.left || l.top != r.top || l.right != r.right || l.bottom != r.bottom;
	}
	forceinline bool operator == (Size l, Size r) {
		return l.width == r.width && l.height == r.height;
	}
	forceinline bool operator != (Size l, Size r) {
		return l.width != r.width || l.height != r.height;
	}
}
#endif

static forceinline Size Size_make(int w, int h) { Size size; size.width = w, size.height = h; return size; }

/**
@brief A property of a class.

@remarks

A property can have some attributes:

nonatomic(default): The property is not thread safe,
and it may return invalid object when used in multi-thread environment.

@code
forceinline NcString* name() { return m_name; }
void setName(NcString* name)
{
retain(name);
release(m_name);
m_name = name;
}
@endcode

atomic: The property is thread safe and it will never return invalid object.
autorelease is implied when atomic is used.

@code
NcString* name()
{
NcString* rtn = NULL;
synchronized(this) {
rtn = autorelease(retain(m_name));
}
return rtn;
}

void setName(NcString* name)
{
synchronized(this)
{
retain(name);
release(m_name);
m_name = name;
}
}
@endcode
*/
#define NC_PROPERTY(name, ...)

/**
@brief Add some additional attributes to a function

@remarks

autorelease: The return value of the function is an autorelease object.

@code
NC_ATTRIBUTE(autorelease);
NcString* NcString::componentsBy(Range range)
{
rtn = NcString::stringWithCharacters(m_str + range.location, range.length);
return rtn;
}
@endcode
*/
#define NC_ATTRIBUTES(...)

#define CQ_ASSERT(o) { if (!(o)) { *((int*)(0)) = 0; }}
#ifndef element_of
#define element_of(o) (sizeof(o) / sizeof((o)[0]))
#endif

// If b can be a negative number, or the round direction is unknown, please refer to the following url:
// http://bytes.com/topic/c/answers/200858-integer-division-towards-infinity
// ASSERT(b > 0 && round toward zero);
#define divideRoundTowardNegativeInfinity(a, b) ((a)/(b) - ((a)%(b)< 0 ? 1 : 0))
#define modRoundTowardNegativeInfinity(a, b) ((a)%(b) + ( (a)%(b)<0 ? (b) : 0))

#define cq_swap(value_type, a, b) {value_type tmp; tmp = a; a = b; b = tmp;}
#define cq_max(a, b) ((a) > (b)? (a) : (b))
#define cq_min(a, b) ((a) < (b)? (a) : (b))
#define Point_equal(l, r) ((l)->x == (r)->x && (l)->y == (r)->y)

forceinline bool cq_bitScanReverse(uint32* index, uint32 mask)
{
	for (int i = 31; i >= 0; i--)
	{
		if (mask & ((uint32)1 << i))
		{
			*index = i;
			return true;
		}
	}
	return false;
}
