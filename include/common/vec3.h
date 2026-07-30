#pragma once
/******************************************************************************
 Template for a three components vector
 *****************************************************************************/
 


#include <assert.h>
#include <cmath>


template <typename T> struct TVec3 {
	/* Members */
	T x;
	T y;
	T z;

	/* Constructors */
	constexpr TVec3() = default;
	constexpr TVec3(T x, T y, T z);
	explicit TVec3(const T *t);

	/* Index Accessor */
	T &operator[](int n);
	const T &operator[](int n) const;
}
