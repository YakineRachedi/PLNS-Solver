#pragma once
/******************************************************************************
 * Quat : Quaternions.
 *
 * Quaternions are mostly used to represent rotations in 3D.
 *
 * A quaternion has four components:
 *
 *      q = (x, y, z, w)
 *
 * or equivalently:
 *
 *      q = (xyz, w)
 *
 * where:
 *
 *      xyz : vector part
 *      w   : scalar part
 *
 * For a rotation of angle theta around a unit axis u:
 *
 *      q.xyz = u * sin(theta / 2)
 *
 *      q.w   = cos(theta / 2)
 *
 * Unit quaternions are particularly useful because they:
 *
 *      - represent rotations compactly;
 *      - avoid Euler angle singularities;
 *      - compose rotations efficiently;
 *      - interpolate smoothly using SLERP.
 *
 *****************************************************************************/

#include <assert.h>

#include "math_utils.h"
#include "vec3.h"


/******************************************************************************
 * Quaternion
 *****************************************************************************/

template <typename T> struct alignas(4 * sizeof(T)) TQuat {
	/*
	 * The quaternion contains a vector part:
	 *
	 *      xyz = (x, y, z)
	 *
	 * and a scalar part:
	 *
	 *      w
	 *
	 * The union allows access either as:
	 *
	 *      q.xyz
	 *
	 * or individually:
	 *
	 *      q.x
	 *      q.y
	 *      q.z
	 */

	union {
		TVec3<T> xyz;

		struct {
			T x;
			T y;
			T z;
		};
	};

	/*
	 * Scalar component.
	 */

	T w;


	/* Constructors */

	TQuat() = default;

	/*
	 * Construct directly from four components.
	 */

	constexpr TQuat(T x, T y, T z, T w);

	/*
	 * Construct from:
	 *
	 *      vector part xyz
	 *
	 * and:
	 *
	 *      scalar part w
	 */

	constexpr TQuat(TVec3<T> xyz, T w);


	/******************************************************************************
	 * Identity quaternion
	 *
	 * The identity quaternion represents no rotation.
	 *
	 *      q = (0, 0, 0, 1)
	 *
	 * Applying this rotation to any vector leaves it unchanged.
	 *****************************************************************************/

	static inline TQuat Identity{TVec3<T>::Zero, T(1)};


	/******************************************************************************
	 * Operators and methods
	 *****************************************************************************/


	/*
	 * Quaternion multiplication.
	 *
	 * Multiplying two rotation quaternions composes two rotations.
	 */

	TQuat & operator*=(const TQuat & a);


	/*
	 * Multiply all components by a scalar.
	 */

	TQuat & operator*=(const T & t);


	/*
	 * Return the conjugate quaternion.
	 *
	 * For:
	 *
	 *      q = (x, y, z, w)
	 *
	 * the conjugate is:
	 *
	 *      q* = (-x, -y, -z, w)
	 *
	 * For a unit quaternion representing a rotation,
	 * the conjugate represents the inverse rotation.
	 */

	TQuat conj() const;


	/*
	 * Return the inverse quaternion.
	 *
	 * Mathematically:
	 *
	 *              q*
	 *      q^-1 = -----
	 *             |q|^2
	 *
	 * For a unit quaternion:
	 *
	 *      |q| = 1
	 *
	 * therefore:
	 *
	 *      q^-1 = q*
	 */

	TQuat inv() const;


	/*
	 * Normalize the quaternion.
	 *
	 * After normalization:
	 *
	 *      |q| = 1
	 *
	 * This is necessary when using a quaternion as a rotation.
	 */

	TQuat & normalise();
};

typedef TQuat<float> Quat;


/*
 * Unary minus.
 *
 * Important:
 *
 * In this code:
 *
 *      -q
 *
 * does NOT mean the usual arithmetic negation:
 *
 *      (-x, -y, -z, -w)
 *
 * Instead it returns the conjugate:
 *
 *      (-x, -y, -z, w)
 *
 * This convention is used because for a unit rotation quaternion:
 *
 *      q^-1 = q*
 *
 * Therefore:
 *
 *      -q
 *
 * represents the inverse rotation.
 */

template <typename T> inline TQuat<T> operator-(const TQuat<T> & a);


/*
 * Quaternion multiplication.
 *
 * If a and b represent rotations:
 *
 *      a * b
 *
 * represents a composition of rotations.
 *
 * Quaternion multiplication is not commutative:
 *
 *      a * b != b * a
 *
 * in general.
 *
 * Therefore rotation order matters.
 */

template <typename T> inline TQuat<T> operator*(const TQuat<T> & a, const TQuat<T> & b);
template <typename T> inline TQuat<T> operator*(const TQuat<T> & a, const T & t); // Multiply a quaternion by a scalar
template <typename T> inline TQuat<T> operator*(const T & t, const TQuat<T> & a); // Scalar multiplication with reversed operands


/*
 * Dot product of two quaternions.
 *
 * This is:
 *
 *      dot(a,b)
 *
 *    = ax*bx + ay*by + az*bz + aw*bw
 */

template <typename T> T dot(const TQuat<T> & a, const TQuat<T> & b);


/*
 * Quaternion norm.
 *
 * The norm is:
 *
 *                    ___________________
 *                   /
 *      |q| = sqrt( dot(q,q) )
 *
 * For a rotation quaternion:
 *
 *      |q| = 1
 */

template <typename T> inline T norm(const TQuat<T> & a);


/*
 * Raise a quaternion to a real power.
 *
 * This is mainly useful for interpolating rotations.
 */

template <typename T> TQuat<T> pow(TQuat<T> & q, T t);


/*
 * Spherical linear interpolation.
 *
 * SLERP smoothly interpolates between two rotations:
 *
 *      q0 -----> q1
 *
 * t = 0:
 *
 *      result = q0
 *
 * t = 1:
 *
 *      result = q1
 *
 * Intermediate values correspond to smooth rotations.
 */

template <typename T> TQuat<T> slerp(TQuat<T> & q0, TQuat<T> q1, T t); 
template <typename T> inline constexpr TQuat<T>::TQuat(T x, T y, T z, T w) : x{ x } , y{ y } , z{ z } , w{ w } {}
template <typename T> inline constexpr TQuat<T>::TQuat(TVec3<T> xyz, T w) : xyz{ xyz } , w{ w } {}


/******************************************************************************
 * Quaternion multiplication assignment
 *****************************************************************************/

template <typename T> inline TQuat<T> & TQuat<T>::operator*=(const TQuat & a) {
	/*
	 * Compute the quaternion product and store the result
	 * in the current quaternion.
	 */

	*this = *this * a;

	return *this;
}


/******************************************************************************
 * Scalar multiplication assignment
 *****************************************************************************/

template <typename T>
inline TQuat<T> &TQuat<T>::operator*=(const T &t)
{
	/*
	 * Multiply both:
	 *
	 *      xyz
	 *
	 * and:
	 *
	 *      w
	 */

	xyz *= t;
	w *= t;

	return *this;
}


/******************************************************************************
 * Quaternion conjugate
 *****************************************************************************/

template <typename T> inline TQuat<T> TQuat<T>::conj() const {
	/*
	 * For:
	 *
	 *      q = (x, y, z, w)
	 *
	 * the conjugate is:
	 *
	 *      q* = (-x, -y, -z, w)
	 */

	return { -xyz, w };
}


/******************************************************************************
 * Quaternion inverse
 *****************************************************************************/

template <typename T> inline TQuat<T> TQuat<T>::inv() const {
	/*
	 * General formula:
	 *
	 *             q*
	 *      q^-1 = -----
	 *             |q|^2
	 *
	 * dot(q,q) is equal to |q|^2.
	 */

	return this->conj() *= (1.f / dot(*this, *this));
}


/******************************************************************************
 * Quaternion normalization
 *****************************************************************************/

template <typename T> inline TQuat<T> & TQuat<T>::normalise() {
	/*
	 * Divide all components by the norm:
	 *
	 *      q_normalized = q / |q|
	 *
	 * After this operation:
	 *
	 *      |q| = 1
	 */

	*this *= (1.f / norm(*this));

	return *this;
}


/******************************************************************************
 * Unary minus: conjugate
 *****************************************************************************/

template <typename T> inline TQuat<T> operator-(const TQuat<T> & a) {
	/*
	 * Project-specific convention:
	 *
	 *      -q = conjugate(q)
	 *
	 * For unit quaternions used as rotations,
	 * this represents the inverse rotation.
	 */

	return a.conj();
}


/******************************************************************************
 * Quaternion multiplication
 *****************************************************************************/

template <typename T> inline TQuat<T> operator*(const TQuat<T> & a, const TQuat<T> & b) {
	/*
	 * Quaternion multiplication:
	 *
	 * Let:
	 *
	 *      a = (a.xyz, a.w)
	 *      b = (b.xyz, b.w)
	 *
	 * Then:
	 *
	 * vector part =
	 *
	 *      a.w * b.xyz
	 *    + b.w * a.xyz
	 *    + cross(a.xyz, b.xyz)
	 *
	 * scalar part =
	 *
	 *      a.w * b.w - dot(a.xyz, b.xyz)
	 *
	 * This multiplication allows quaternion rotations
	 * to be composed.
	 */

	return { a.w * b.xyz + a.xyz * b.w + cross(a.xyz, b.xyz), a.w * b.w - dot(a.xyz, b.xyz) } ;}

template <typename T> inline TQuat<T> operator*(const TQuat<T> & a, const T & t) { return {t * a.xyz, t * a.w} ;}
template <typename T> inline TQuat<T> operator*(const T & t, const TQuat<T> & a) { return a * t ;}
template <typename T> inline T dot(const TQuat<T> & a, const TQuat<T> & b) { return dot(a.xyz, b.xyz) + a.w * b.w ;}
template <typename T> inline T norm(const TQuat<T> & a) { return sqrt(dot(a, a)) ;}


/******************************************************************************
 * Quaternion power
 *****************************************************************************/

template <typename T> TQuat<T> pow(TQuat<T> & q, T t) {
	/*
	 * This function is mainly used by SLERP.
	 *
	 * For a unit quaternion representing:
	 *
	 *      q = (u * sin(omega), cos(omega))
	 *
	 * q^t represents a rotation whose angle has been
	 * multiplied by t.
	 *
	 * Therefore:
	 *
	 *      q^0 = identity
	 *
	 *      q^1 = q
	 *
	 *      q^0.5 represents approximately half the rotation.
	 */

	/*
	 * Clamp w to [-1, 1].
	 *
	 * acos() requires its argument to be in this range.
	 */

	T qw = q.w < -1 ? -1 :
	       q.w > 1 ? 1 :
	       q.w;

	T omega = acos(qw);

	T cto = cos(t * omega);
	T sto = sin(t * omega);

	T so = sin(omega);

	/*
	 * When sin(omega) is zero, the quaternion corresponds
	 * to a degenerate case. Use t as a safe fallback.
	 */

	T mult = (so != 0) ? sto / so : t;

	return {
		mult * q.xyz,
		cto
	};
}


/******************************************************************************
 * Spherical Linear Interpolation
 *****************************************************************************/

template <typename T> TQuat<T> slerp(TQuat<T> &q0, TQuat<T> q1, T t) {
	/*
	 * SLERP interpolates on the surface of the unit
	 * quaternion sphere.
	 *
	 * The expression:
	 *
	 *      q0^-1 * q1
	 *
	 * represents the rotation needed to go from q0 to q1.
	 *
	 * Raising it to the power t selects a fraction
	 * of this rotation.
	 *
	 * Finally:
	 *
	 *      q0 * (...)
	 *
	 * applies this partial rotation starting from q0.
	 *
	 * Therefore:
	 *
	 *      t = 0 -> q0
	 *
	 *      t = 1 -> q1
	 */

	return q0 * pow(q0.inv() * q1, t);
}