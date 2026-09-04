#pragma once
/******************************************************************************
 * TMat4 : Generic 4x4 matrix stored in column-major order.
 *
 * The matrix is internally represented as an array of four column vectors
 * (TVec4<T>). Each element can be accessed using the familiar mathematical
 * notation M(i, j), where:
 *
 *      i = row index    (0 <= i < 4)
 *      j = column index (0 <= j < 4)
 *
 * Although the storage is column-major, the access operator hides this
 * implementation detail so that users manipulate the matrix as a regular
 * mathematical object.
 *
 * Internal memory layout:
 *
 *      cols[0]        cols[1]        cols[2]        cols[3]
 *
 *      | m00 |        | m01 |        | m02 |        | m03 |    
 *      | m10 |        | m11 |        | m12 |        | m13 |
 *      | m20 |        | m21 |        | m22 |        | m23 |
 *      | m30 |        | m31 |        | m32 |        | m33 |
 * 
 * Each column is represented by a Vec4
 *
 * Main features:
 *
 *   - operator()(i, j): access or modify the element at row i and column j.
 *   - operator()(j)   : access an entire column vector.
 *   - operator*       : matrix-matrix multiplication.
 *   - print()         : display the matrix in row-major mathematical form.
 * 
 * This matrix class is mainly used for 3D transformations:
 *
 *     - rotations;
 *     - translations;
 *     - camera transformations;
 *     - perspective projections.
 *
 *****************************************************************************/

#include <assert.h>
#include <stdio.h>

#include "vec4.h"

template <typename T> struct TMat4 {
	TVec4<T> cols[4];

	/* Element accessor and mutator */
	const T & operator()(int i, int j) const;
	T & operator()(int i, int j);

	/* Column acessor and mutator */
	const TVec4<T> & operator()(int i) const;
	TVec4<T> & operator()(int i);
};

typedef TMat4<float> Mat4;


template <typename T> TMat4<T> operator*(const TMat4<T> & m1, const TMat4<T> & m2);
template <typename T> void print(const TMat4<T> & m);
template <typename T> inline const T & TMat4<T>::operator()(int i, int j) const{
	assert(i >= 0 && i <= 3 && j >= 0 && j <= 3);
	return cols[j][i];
}

template <typename T> inline T & TMat4<T>::operator()(int i, int j) {
	assert(i >= 0 && i <= 3 && j >= 0 && j <= 3);
	return cols[j][i];
}

template <typename T> inline const TVec4<T> & TMat4<T>::operator()(int j) const {
	assert(j >= 0 && j <= 3);
	return cols[j];
}

template <typename T> inline TVec4<T> & TMat4<T>::operator()(int j) {
	assert(j >= 0 && j <= 3);
	return cols[j];
}

template <typename T> TMat4<T> operator*(const TMat4<T> & A, const TMat4<T> & B) {
	TMat4<T> AB;

	/* First column. */

	AB(0) =
		A(0) * B(0, 0) +
		A(1) * B(1, 0) +
		A(2) * B(2, 0) +
		A(3) * B(3, 0);


	/* Second column. */

	AB(1) =
		A(0) * B(0, 1) +
		A(1) * B(1, 1) +
		A(2) * B(2, 1) +
		A(3) * B(3, 1);


	/* Third column. */

	AB(2) =
		A(0) * B(0, 2) +
		A(1) * B(1, 2) +
		A(2) * B(2, 2) +
		A(3) * B(3, 2);


	/* Fourth column. */

	AB(3) =
		A(0) * B(0, 3) +
		A(1) * B(1, 3) +
		A(2) * B(2, 3) +
		A(3) * B(3, 3);

	return AB;
}

/******************************************************************************
 * Print the matrix.
 *
 * The matrix is printed in mathematical row-major form.
 *
 * Example:
 *
 *     | M00 M01 M02 M03 |
 *     | M10 M11 M12 M13 |
 *     | M20 M21 M22 M23 |
 *     | M30 M31 M32 M33 |
 *
 *****************************************************************************/

template <typename T> void print(const TMat4<T> & m) {
	printf("%.3f %.3f %.3f %.3f\n", m(0, 0), m(0, 1), m(0, 2), m(0, 3));
	printf("%.3f %.3f %.3f %.3f\n", m(1, 0), m(1, 1), m(1, 2), m(1, 3));
	printf("%.3f %.3f %.3f %.3f\n", m(2, 0), m(2, 1), m(2, 2), m(2, 3));
	printf("%.3f %.3f %.3f %.3f\n", m(3, 0), m(3, 1), m(3, 2), m(3, 3));
}