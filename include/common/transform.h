#pragma once
/******************************************************************************
 * Some utility functions related to affine transforms in 3D.
 *
 * This file contains utilities to manipulate objects in 3D space:
 *
 *   - rotations using quaternions;
 *   - translations;
 *   - rigid transformations;
 *   - transformation matrices.
 *
 * A rigid transformation has the form:
 *
 *      p' = R * p + t
 *
 * where:
 *
 *      R : rotation
 *      t : translation
 *      p : original point
 *      p': transformed point
 *
 *****************************************************************************/

#include "vec3.h"
#include "vec4.h"
#include "quat.h"
#include "mat4.h"


/******************************************************************************
 * Rigid Transformation
 *
 * A rigid transformation represents a rotation followed by a translation.
 *
 * Mathematically:
 *
 *      p' = R(p) + t
 *
 * A rigid transformation does not deform an object:
 *
 *      - lengths are preserved;
 *      - angles are preserved;
 *      - shapes are preserved.
 *
 * Example:
 *
 *      A cube is rotated and moved somewhere else.
 *      The cube does not change its shape.
 *
 *****************************************************************************/

template <typename T> struct TRigT {
	/*
	 * Rotation component.
	 *
	 * The quaternion represents the orientation of the object.
	 */
	TQuat<T> rot;

	/*
	 * Translation component.
	 *
	 * This represents the displacement in 3D space.
	 */
	TVec3<T> trans;

	/*
	 * Return the inverse transformation.
	 *
	 * If the original transformation is:
	 *
	 *      p' = R(p) + t
	 *
	 * then the inverse must recover p from p':
	 *
	 *      p = R^-1(p' - t)
	 *
	 * For a unit quaternion:
	 *
	 *      R^-1 is represented by the conjugate quaternion.
	 */
	TRigT inv() const;

	/*
	 * Convert the rigid transformation into a 4x4 matrix.
	 *
	 * This is useful for graphics APIs such as OpenGL.
	 *
	 * The resulting matrix contains both:
	 *
	 *      - rotation;
	 *      - translation.
	 */
	TMat4<T> as_matrix() const;

	/*
	 * Identity transformation.
	 *
	 * The identity transformation does nothing:
	 *
	 *      p' = p
	 *
	 * It has:
	 *
	 *      - identity rotation;
	 *      - zero translation.
	 */
	static inline TRigT Identity = {TQuat<T>::Identity, TVec3<T>::Zero};
};

typedef TRigT<float> RigT;


/*
 * Rotate a vector using a quaternion.
 *
 * Conceptually:
 *
 *      v' = R(v)
 *
 * The quaternion q represents the rotation R.
 *
 * Example:
 *
 *      v = (1, 0, 0)
 *
 * A rotation of 90 degrees around the Z axis gives:
 *
 *      v' = (0, 1, 0)
 *
 * The quaternion must be normalized:
 *
 *      |q| = 1
 */
template <typename T> TVec3<T> rotate(const TVec3<T> & v, const TQuat<T> & q);


/*
 * Apply the inverse rotation.
 *
 * If:
 *
 *      v' = rotate(v, q)
 *
 * then:
 *
 *      v = unrotate(v', q)
 *
 * For a unit quaternion, the inverse rotation is represented
 * by the conjugate quaternion.
 */
template <typename T> TVec3<T> unrotate(const TVec3<T> & v, const TQuat<T> & q);


/*
 * Rotate a point around a pivot.
 *
 * Normally:
 *
 *      rotate(point, q)
 *
 * rotates the point around the origin.
 *
 * To rotate around another point called pivot:
 *
 *      1. Move the pivot to the origin:
 *
 *             point - pivot
 *
 *      2. Rotate:
 *
 *             rotate(point - pivot, q)
 *
 *      3. Move back:
 *
 *             rotate(...) + pivot
 *
 * Therefore:
 *
 *      p' = R(p - pivot) + pivot
 */
template <typename T> TVec3<T> orbit(const TVec3<T> & point, const TQuat<T> & q, const TVec3<T> & pivot);


/*
 * Compose two rotations.
 *
 * Quaternion multiplication represents the composition of
 * two rotations.
 *
 * Important:
 *
 *      q2 * q1
 *
 * means that q1 is applied first, then q2.
 */
template <typename T> TQuat<T> compose(const TQuat<T> & q1, const TQuat<T> & q2);


/*
 * Create a rigid transformation from a rotation followed
 * by a translation.
 *
 * The result represents:
 *
 *      p' = R(p) + v
 */
template <typename T> TRigT<T> compose(const TQuat<T> & q, const TVec3<T> & v);


/*
 * Create a rigid transformation from a translation and
 * a rotation.
 *
 * Here the translation is itself rotated.
 *
 * The order of transformations matters.
 */
template <typename T> TRigT<T> compose(const TVec3<T> & v, const TQuat<T> & q);


/*
 * Compose two rigid transformations.
 *
 * If:
 *
 *      A(p) = R_A(p) + t_A
 *
 * and:
 *
 *      B(p) = R_B(p) + t_B
 *
 * then:
 *
 *      B(A(p))
 *
 * gives:
 *
 *      R_B(R_A(p) + t_A) + t_B
 *
 * therefore:
 *
 *      R = R_B * R_A
 *
 *      t = R_B(t_A) + t_B
 */
template <typename T> TRigT<T> compose(const TRigT<T> & A, const TRigT<T> & B);


/*
 * Compose two 4x4 matrices.
 *
 * The convention used in this project is:
 *
 *      compose(A, B) = B * A
 *
 * Therefore A is applied first, then B.
 */
template <typename T> TMat4<T> compose(const TMat4<T> & A, const TMat4<T> & B);


/*
 * Transform a 3D point using a 4x4 homogeneous matrix.
 *
 * A Vec3 is implicitly converted into a homogeneous
 * 4D representation.
 *
 * After multiplication, perspective division is performed:
 *
 *      (x, y, z, w)
 *
 * becomes:
 *
 *      (x/w, y/w, z/w)
 */
template <typename T> TVec3<T> transform(const TMat4<T> & A, const TVec3<T> & v);


/*
 * Transform a complete 4D homogeneous vector.
 *
 * This directly computes:
 *
 *      result = A * v
 *
 * No division by w is performed.
 */
template <typename T>
TVec4<T> transform(const TMat4<T> & A, const TVec4<T> &v);


/******************************************************************************
 * Implementations
 *****************************************************************************/


/******************************************************************************
 * Inverse rigid transformation
 *****************************************************************************/

template <typename T> TRigT<T> TRigT<T>::inv() const {
	/*
	 * Original transformation:
	 *
	 *      p' = R(p) + t
	 *
	 * Inverse:
	 *
	 *      p = R^-1(p' - t)
	 *
	 *       = R^-1(p') - R^-1(t)
	 *
	 * Therefore:
	 *
	 *      inverse rotation    = R^-1
	 *      inverse translation = -R^-1(t)
	 *
	 * For a unit quaternion:
	 *
	 *      -rot
	 *
	 * returns the conjugate, which represents the inverse rotation.
	 */

	return { -rot, -unrotate(trans, rot) };
}


/******************************************************************************
 * Convert a rigid transformation to a 4x4 matrix
 *****************************************************************************/

template <typename T> TMat4<T> TRigT<T>::as_matrix() const {
	TMat4<T> M;

	/*
	 * Precompute products of quaternion components.
	 *
	 * This avoids repeating multiplications when constructing
	 * the rotation matrix.
	 */

	const T xx = rot.x * rot.x;
	const T xy = rot.x * rot.y;
	const T xz = rot.x * rot.z;
	const T xw = rot.x * rot.w;

	const T yy = rot.y * rot.y;
	const T yz = rot.y * rot.z;
	const T yw = rot.y * rot.w;

	const T zz = rot.z * rot.z;
	const T zw = rot.z * rot.w;

	/*
	 * Construct the 3x3 rotation matrix from the quaternion.
	 *
	 * The upper-left 3x3 part represents rotation.
	 */

	M(0, 0) = 1.f - 2.f * (yy + zz);
	M(1, 0) = 2.f * (xy + zw);
	M(2, 0) = 2.f * (xz - yw);
	M(3, 0) = 0.f;

	M(0, 1) = 2.f * (xy - zw);
	M(1, 1) = 1.f - 2.f * (xx + zz);
	M(2, 1) = 2.f * (yz + xw);
	M(3, 1) = 0.f;

	M(0, 2) = 2.f * (xz + yw);
	M(1, 2) = 2.f * (yz - xw);
	M(2, 2) = 1.f - 2.f * (xx + yy);
	M(3, 2) = 0.f;

	/*
	 * Last column contains the translation.
	 *
	 * Homogeneous coordinates allow us to represent:
	 *
	 *      rotation + translation
	 *
	 * using a single matrix multiplication.
	 */

	M(0, 3) = trans.x;
	M(1, 3) = trans.y;
	M(2, 3) = trans.z;
	M(3, 3) = 1.f;

	return M;
}


/******************************************************************************
 * Rotate a vector
 *****************************************************************************/

template <typename T> inline TVec3<T> rotate(const TVec3<T> & v, const TQuat<T> & q) {
	/*
	 * Rotation quaternions must have norm 1.
	 */

	assert(approx_equal<T>(norm(q), 1));

	/*
	 * This is an optimized formula for quaternion rotation.
	 *
	 * Mathematically, quaternion rotation is:
	 *
	 *      v' = q * v * q*
	 *
	 * where:
	 *
	 *      q  : rotation quaternion
	 *      q* : conjugate quaternion
	 *
	 * Instead of performing two quaternion multiplications,
	 * the formula below directly computes the resulting vector.
	 */

	return 2.f * dot(q.xyz, v) * q.xyz + (2.f * q.w * q.w - 1.f) * v + 2.f * q.w * cross(q.xyz, v);
}


/******************************************************************************
 * Apply inverse rotation
 *****************************************************************************/

template <typename T> inline TVec3<T> unrotate(const TVec3<T> & v, const TQuat<T> & q) {
	assert(approx_equal<T>(norm(q), 1));

	/*
	 * This corresponds to applying the inverse rotation.
	 *
	 * Compared to rotate(), the cross-product term changes sign.
	 */

	return 2.f * dot(q.xyz, v) * q.xyz + (2.f * q.w * q.w - 1.f) * v - 2.f * q.w * cross(q.xyz, v);
}


/******************************************************************************
 * Rotate a point around a pivot
 *****************************************************************************/

template <typename T> TVec3<T> orbit(const TVec3<T> & point, const TQuat<T> & q, const TVec3<T> & pivot) {
	/*
	 * Example:
	 *
	 * Suppose:
	 *
	 *      pivot = (10, 0, 0)
	 *
	 * To rotate a point around this pivot:
	 *
	 *      point - pivot
	 *
	 * moves the pivot to the origin.
	 *
	 * We rotate around the origin, then add pivot back.
	 */

	return rotate(point - pivot, q) + pivot;
}


/******************************************************************************
 * Compose rotations
 *****************************************************************************/

template <typename T> TQuat<T> compose(const TQuat<T> & q1, const TQuat<T> & q2) {
	/*
	 * q1 is applied first.
	 * q2 is applied second.
	 */

	return q2 * q1;
}


/******************************************************************************
 * Create rigid transformations
 *****************************************************************************/

template <typename T> TRigT<T> compose(const TQuat<T> & q, const TVec3<T> & v) {
	/*
	 * Rotation followed by translation:
	 *
	 *      p' = R(p) + v
	 */

	return { q, v };
}


template <typename T> TRigT<T> compose(const TVec3<T> & v, const TQuat<T> & q) {
	/*
	 * Translation followed by rotation:
	 *
	 *      p' = R(p + v)
	 *
	 *       = R(p) + R(v)
	 *
	 * Therefore the stored translation must also be rotated.
	 */

	return { q, rotate(v, q) };
}


/******************************************************************************
 * Compose rigid transformations
 *****************************************************************************/

template <typename T> TRigT<T> compose(const TRigT<T> & A, const TRigT<T> & B) {
	/*
	 * Apply A first:
	 *
	 *      p1 = R_A(p) + t_A
	 *
	 * Then apply B:
	 *
	 *      p2 = R_B(p1) + t_B
	 *
	 * Therefore:
	 *
	 *      p2 = R_B(R_A(p)) + R_B(t_A) + t_B
	 *
	 * Rotation:
	 *
	 *      R = R_B * R_A
	 *
	 * Translation:
	 *
	 *      t = R_B(t_A) + t_B
	 */

	return {B.rot * A.rot, rotate(A.trans, B.rot) + B.trans};
}


/******************************************************************************
 * Compose matrices
 *****************************************************************************/

template <typename T> TMat4<T> compose(const TMat4<T> & A, const TMat4<T> & B) {
	/*
	 * Matrix multiplication follows the same convention:
	 *
	 *      A first
	 *      B second
	 *
	 * Therefore:
	 *
	 *      B * A
	 */

	return B * A;
}


/******************************************************************************
 * Transform a 3D point
 *****************************************************************************/

template <typename T> TVec3<T> transform(const TMat4<T> & A, const TVec3<T> & v) {
	/*
	 * Matrix A is stored as four columns:
	 *
	 *      A(0), A(1), A(2), A(3)
	 *
	 * A 3D point is treated as the homogeneous vector:
	 *
	 *      (x, y, z, 1)
	 *
	 * Therefore:
	 *
	 *      A * (x, y, z, 1)
	 *
	 * becomes:
	 *
	 *      A(0) * x
	 *    + A(1) * y
	 *    + A(2) * z
	 *    + A(3) * 1
	 *
	 * The last column therefore contributes translation.
	 */

	TVec4<T> prod =
		A(0) * v[0] +
		A(1) * v[1] +
		A(2) * v[2] +
		A(3);

	/*
	 * Perspective division.
	 *
	 * A homogeneous vector:
	 *
	 *      (x, y, z, w)
	 *
	 * represents the 3D point:
	 *
	 *      (x/w, y/w, z/w)
	 *
	 * For ordinary affine transformations, w is normally 1.
	 */

	T inv_w = 1.f / prod.w;

	return {
		prod.x * inv_w,
		prod.y * inv_w,
		prod.z * inv_w
	};
}


/******************************************************************************
 * Transform a 4D vector
 *****************************************************************************/

template <typename T> TVec4<T> transform(const TMat4<T> & A, const TVec4<T> & v) {
	/*
	 * Direct matrix-vector multiplication:
	 *
	 *        | A00 A01 A02 A03 |   | x |
	 *        | A10 A11 A12 A13 | * | y |
	 *        | A20 A21 A22 A23 |   | z |
	 *        | A30 A31 A32 A33 |   | w |
	 *
	 * Because the matrix is stored by columns:
	 *
	 *      A * v =
	 *
	 *      A(0) * x +
	 *      A(1) * y +
	 *      A(2) * z +
	 *      A(3) * w
	 */

	return {
		A(0) * v[0] +
		A(1) * v[1] +
		A(2) * v[2] +
		A(3) * v[3]
	};
}