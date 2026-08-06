#pragma once
/******************************************************************************
 * TMat3 : Generic 3x3 matrix stored in column-major order.
 *
 * The matrix is internally represented as an array of three column vectors
 * (TVec3<T>). Each element can be accessed using the familiar mathematical
 * notation M(i, j), where:
 *
 *      i = row index    (0 <= i < 3)
 *      j = column index (0 <= j < 3)
 *
 * Although the storage is column-major, the access operator hides this
 * implementation detail so that users manipulate the matrix as a regular
 * mathematical object.
 *
 * Internal memory layout:
 *
 *      cols[0]        cols[1]        cols[2]
 *
 *      | m00 |        | m01 |        | m02 |
 *      | m10 |        | m11 |        | m12 |
 *      | m20 |        | m21 |        | m22 |
 *
 * Main features:
 *
 *   - operator()(i, j): access or modify the element at row i and column j.
 *   - operator()(j)   : access an entire column vector.
 *   - operator*       : matrix-matrix multiplication.
 *   - print()         : display the matrix in row-major mathematical form.
 *
 *****************************************************************************/
#include <assert.h>
#include <stdio.h>

#include "vec3.h"

template <typename T> struct TMat3 {
	TVec3<T> cols[3];

	/* Element accessor and mutator */
	const T & operator()(int i, int j) const;
	T & operator()(int i, int j);

	/* Column acessor and mutator */
	const TVec3<T> & operator()(int i) const;
	TVec3<T> & operator()(int i);
};

typedef TMat3<float> Mat3f;
typedef TMat3<double> Mat3d;
using Mat3 = Mat3f;

/* Free functions */

template <typename T> TMat3<T> operator*(const TMat3<T> & m1, const TMat3<T> & m2);
template <typename T> void print(const TMat3<T> & m);

/* Implementations */

template <typename T> inline const T & TMat3<T>::operator()(int i, int j) const {
    assert(i >= 0 && i <= 2 && j >= 0 && j <= 2);
	return cols[j][i];
}

template <typename T> inline T & TMat3<T>::operator()(int i, int j) {
	assert(i >= 0 && i <= 2 && j >= 0 && j <= 2);
	return cols[j][i];
}

template <typename T> inline const TVec3<T> & TMat3<T>::operator()(int j) const {
	assert(j >= 0 && j <= 2);
	return cols[j];
}

template <typename T> inline TVec3<T> &TMat3<T>::operator()(int j) {
	assert(j >= 0 && j <= 2);
	return cols[j];
}

template <typename T> TMat3<T> operator*(const TMat3<T> & A, const TMat3<T> & B) {
	TMat3<T> AB;

	AB(0) = A(0) * B(0, 0) + A(1) * B(1, 0) + A(2) * B(2, 0);
	AB(1) = A(0) * B(0, 1) + A(1) * B(1, 1) + A(2) * B(2, 1);
	AB(2) = A(0) * B(0, 2) + A(1) * B(1, 2) + A(2) * B(2, 2);

	return (AB);
}

template <typename T> void print(const TMat3<T> & m) {
	printf("%.3f %.3f %.3f\n", m(0, 0), m(0, 1), m(0, 2));
	printf("%.3f %.3f %.3f\n", m(1, 0), m(1, 1), m(1, 2));
	printf("%.3f %.3f %.3f\n", m(2, 0), m(2, 1), m(2, 2));
}