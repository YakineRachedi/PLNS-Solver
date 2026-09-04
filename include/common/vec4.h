#pragma once

/******************************************************************************
 * Vec4 : vector with four components.
 *
 * This class is mainly used for:
 *
 *     - homogeneous coordinates;
 *     - 3D transformations;
 *     - 4x4 matrix operations.
 *
 * A 3D point:
 *
 *     (x, y, z)
 *
 * is usually represented in homogeneous coordinates as:
 *
 *     (x, y, z, 1)
 *
 * while a direction/vector is represented as:
 *
 *     (x, y, z, 0)
 *
 * The fourth component w allows 4x4 matrices to represent both rotations
 * and translations.
 *****************************************************************************/

#include <assert.h>
#include <cmath>

#include "vec3.h"


/******************************************************************************
 * A four components vector.
 *
 * The vector can be accessed in two different ways:
 *
 *     v.x
 *     v.y
 *     v.z
 *     v.w
 *
 * or:
 *
 *     v.xyz
 *
 * The xyz member represents the first three components as a Vec3.
 *
 * Memory layout:
 *
 *     +-----+-----+-----+-----+
 *     |  x  |  y  |  z  |  w  |
 *     +-----+-----+-----+-----+
 *
 *****************************************************************************/

template <typename T> struct alignas(4 * sizeof(T)) TVec4 {
	/* Components
	 *
	 * The union allows accessing the first three components either as:
	 *
	 *     x, y, z
	 *
	 * or as:
	 *
	 *     xyz
	 */

	union {

		/* First three components as a Vec3. */

		TVec3<T> xyz;

		/* Individual components. */

		struct {
			T x;
			T y;
			T z;
		};
	};

	/* Fourth component.
	 *
	 * In homogeneous coordinates:
	 *
	 *     w = 1 -> point
	 *     w = 0 -> direction/vector
	 */

	T w;


	/* Constructors */

	/* Default constructor.
	 *
	 * The components are not explicitly initialized.
	 */

	constexpr TVec4() = default;


	/* Construct a vector from four components.
	 *
	 * Example:
	 *
	 *     Vec4 v(1, 2, 3, 4);
	 */

	constexpr TVec4(T x, T y, T z, T w);


	/* Construct a Vec4 from a Vec3 and a fourth component.
	 *
	 * Example:
	 *
	 *     Vec3 p(1, 2, 3);
	 *     Vec4 hp(p, 1);
	 *
	 * hp is the homogeneous representation of a point.
	 */

	constexpr TVec4(const TVec3<T> & xyz, T w);


	/* Construct a vector from an array of four values.
	 *
	 * Example:
	 *
	 *     float data[4] = {1, 2, 3, 4};
	 *     Vec4 v(data);
	 */

	explicit TVec4(const T *t);


	/* Index accessors.
	 *
	 *     v[0] -> x
	 *     v[1] -> y
	 *     v[2] -> z
	 *     v[3] -> w
	 */

	T & operator[](int n);

	const T & operator[](int n) const;


	/* Equality */

	bool operator==(const TVec4<T> & a) const;


	/* Vector space operations */

	/* Unary minus.
	 *
	 *     -v
	 */

	TVec4 operator-() const;


	/* Addition:
	 *
	 *     v += a
	 */

	TVec4 & operator+=(const TVec4 & a);


	/* Subtraction:
	 *
	 *     v -= a
	 */

	TVec4 & operator-=(const TVec4 & a);


	/* Multiplication by a scalar:
	 *
	 *     v *= t
	 */

	TVec4 & operator*=(const T & t);


	/* Division by a scalar:
	 *
	 *     v /= t
	 */

	TVec4 & operator/=(const T & t);


	/* Static vector equal to:
	 *
	 *     (0, 0, 0, 0)
	 */

	static const TVec4 Zero;
};


typedef TVec4<float> Vec4;

template <typename T> inline TVec4<T> operator+(const TVec4<T> & a, const TVec4<T> & b);
template <typename T> inline TVec4<T> operator-(const TVec4<T> & a, const TVec4<T> & b);template <typename T> inline TVec4<T> operator*(const TVec4<T> & a, const T & t);
template <typename T> inline TVec4<T> operator*(const T & t, const TVec4<T> & a);
template <typename T> inline T dot(const TVec4<T> & a, const TVec4<T> & b);
template <typename T> inline T norm(const TVec4<T> & a); // Euclidean norm
template <typename T> const TVec4<T> TVec4<T>::Zero{ 0, 0, 0, 0 };
template <typename T> inline constexpr TVec4<T>::TVec4(T x, T y, T z, T w) : x{ x } , y{ y } , z{ z }, w{ w } {}
template <typename T> inline constexpr TVec4<T>::TVec4(const TVec3<T> & xyz, T w) : xyz{ xyz } , w{ w } {}
template <typename T> inline TVec4<T>::TVec4(const T *t) : x{ t[0] } , y{ t[1] } , z{ t[2] } , w{ t[3] } {}

// Index access
template <typename T> inline const T & TVec4<T>::operator[](int n) const {
	assert(n >= 0 && n <= 3);

	/*
	 * x, y, z and w are stored consecutively in memory.
	 *
	 * Therefore:
	 *
	 *     (&x)[0] -> x
	 *     (&x)[1] -> y
	 *     (&x)[2] -> z
	 *     (&x)[3] -> w
	 */

	return (&x)[n];
}


template <typename T> inline T &TVec4<T>::operator[](int n) {assert(n >= 0 && n <= 3); return (&x)[n] ;}

// Equality
template <typename T> inline bool TVec4<T>::operator==(const TVec4<T> & a) const 
    { return (x == a.x && y == a.y && z == a.z && w == a.w) ;}

// Unary minus
template <typename T> inline TVec4<T> TVec4<T>::operator-() const { return { -x, -y, -z, -w } ;}

// In-place arithmetic operations
template <typename T> inline TVec4<T> & TVec4<T>::operator+=(const TVec4<T> & a) { x += a.x; y += a.y; z += a.z; w += a.w; return (*this) ;}
template <typename T> inline TVec4<T> & TVec4<T>::operator-=(const TVec4<T> & a) { x -= a.x; y -= a.y; z -= a.z; w -= a.w; return (*this) ;}
template <typename T> inline TVec4<T> & TVec4<T>::operator*=(const T & t) { x *= t; y *= t; z *= t; w *= t; return (*this) ;}
template <typename T> inline TVec4<T> & TVec4<T>::operator/=(const T & t) { x /= t; y /= t; z /= t; w /= t; return (*this) ;}

// arithmetic operators
template <typename T> inline TVec4<T> operator+(const TVec4<T> & a, const TVec4<T> & b)
    {return { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w } ;}
template <typename T> inline TVec4<T> operator-(const TVec4<T> & a, const TVec4<T> & b) 
    {return { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w } ;}
template <typename T> inline TVec4<T> operator*(const TVec4<T> & a, const T & t) 
    {return { a.x * t, a.y * t, a.z * t, a.w * t } ;}

template <typename T> inline TVec4<T> operator*(const T & t, const TVec4<T> & a){return a * t ;}

// Dot product
template <typename T> inline T dot(const TVec4<T> & a, const TVec4<T> & b) {
    return ( a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w ) ;}

// Euclidean norm
template <typename T> inline T norm(const TVec4<T> & a) {return sqrt(dot(a, a)) ;}