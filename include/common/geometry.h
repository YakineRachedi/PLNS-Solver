#pragma once

/******************************************************************************
 * Geometry : basic geometric objects and utility functions.
 *
 * This file defines:
 *
 *     - Ray    : a half-line;
 *     - Plane  : an infinite plane;
 *     - Sphere : a sphere;
 *
 * It also provides functions for:
 *
 *     - constructing a plane;
 *     - intersecting a ray with a plane;
 *     - computing rotations between vectors;
 *     - computing triangle normals.
 *
 * These objects are particularly useful for:
 *
 *     - 3D graphics;
 *     - cameras;
 *     - object picking;
 *     - collision detection;
 *     - mesh processing.
 *
 *****************************************************************************/

#include "quat.h"
#include "vec3.h"
#include "vec4.h"


/******************************************************************************
 * Ray.
 *
 * A ray is a half-line.
 *
 * It is defined by:
 *
 *     - a starting point;
 *     - a direction.
 *
 * Mathematically:
 *
 *     P(t) = start + t * dir
 *
 * with:
 *
 *     t >= 0
 *
 *
 * Example:
 *
 *
 *                    direction
 *
 *                       ---->
 *
 *     start ●────────────────────────────>
 *
 *
 *****************************************************************************/

template <typename T>
struct TRay {
	/* Starting point of the ray. */

	TVec3<T> start;


	/* Direction of the ray.
	 *
	 * Usually normalized, although this structure itself does not enforce it.
	 */

	TVec3<T> dir;
};


/* Ray using float components. */

typedef TRay<float> Ray;


/******************************************************************************
 * Plane.
 *
 * A plane is represented by the equation:
 *
 *     ax + by + cz + d = 0
 *
 * The vector:
 *
 *     (a, b, c)
 *
 * is the normal vector of the plane.
 *
 *
 * Example:
 *
 * The plane:
 *
 *     z = 0
 *
 * can be written:
 *
 *     0x + 0y + 1z + 0 = 0
 *
 * Therefore:
 *
 *     normal = (0, 0, 1)
 *     d      = 0
 *
 *
 * Geometrically:
 *
 *
 *                 normal
 *                    ↑
 *                    |
 *                    |
 *     --------------------------------
 *
 *                    plane
 *
 *****************************************************************************/

template <typename T> struct TPlane {
	union {

		/* Plane normal vector.
		 *
		 *     normal = (a, b, c)
		 */

		TVec3<T> normal;


		/* Alternative access to the normal components.
		 *
		 *     normal.x == a
		 *     normal.y == b
		 *     normal.z == c
		 */

		struct {
			T a;
			T b;
			T c;
		};
	};


	/* Constant coefficient of the plane equation.
	 *
	 *     ax + by + cz + d = 0
	 */

	T d;
};


/* Plane using float components. */

typedef TPlane<float> Plane;


/******************************************************************************
 * Sphere.
 *
 * A sphere is defined by:
 *
 *     - a center;
 *     - a radius.
 *
 *
 *                  .-''''-.
 *               .'         '.
 *              /             \
 *             |       ●       |
 *              \             /
 *               '.         .'
 *                  '-...-'
 *
 *                    center
 *
 *
 * Every point P on the sphere satisfies:
 *
 *     distance(P, center) = radius
 *
 *****************************************************************************/

template <typename T> struct TSphere {
	/* Center of the sphere. */

	TVec3<T> center;


	/* Radius of the sphere. */

	T radius;
};


/* Sphere using float components. */

typedef TSphere<float> Sphere;


/******************************************************************************
 * Create a plane from:
 *
 *     - a normal vector;
 *     - a point belonging to the plane.
 *
 *
 * Given:
 *
 *     normal = (a, b, c)
 *
 * and a point:
 *
 *     point = (x0, y0, z0)
 *
 * the plane equation is:
 *
 *     a(x - x0)
 *   + b(y - y0)
 *   + c(z - z0)
 *   = 0
 *
 * Expanding:
 *
 *     ax + by + cz
 *   - ax0 - by0 - cz0
 *   = 0
 *
 * Therefore:
 *
 *     d = -dot(normal, point)
 *
 *****************************************************************************/

template <typename T> TPlane<T> plane_from_normal_and_point(const TVec3<T> & normal, const TVec3<T> & point);


/******************************************************************************
 * Intersect a ray with a plane.
 *
 * Ray equation:
 *
 *     P(t) = start + t * dir
 *
 * Plane equation:
 *
 *     dot(normal, P) + d = 0
 *
 * Substituting the ray equation:
 *
 *     dot(normal, start + t * dir) + d = 0
 *
 * Therefore:
 *
 *     t =
 *
 *     -(d + dot(normal, start))
 *     -------------------------
 *          dot(normal, dir)
 *
 *
 * The returned Vec4 contains:
 *
 *     xyz : numerator information used to compute the intersection;
 *     w   : denominator information.
 *
 * The exact interpretation should be understood from the implementation.
 *
 *****************************************************************************/

template <typename T> TVec4<T> ray_plane_intersection(const TRay<T> & ray, const TPlane<T> & plane);


/******************************************************************************
 * Compute a quaternion rotation from one unit vector to another.
 *
 * Example:
 *
 *             to
 *             ↑
 *             |
 *
 * from ------>
 *
 *
 * The resulting quaternion represents the rotation that transforms:
 *
 *     from
 *
 * into:
 *
 *     to
 *
 *
 * Both vectors are expected to be normalized:
 *
 *     norm(from) == 1
 *     norm(to)   == 1
 *
 *****************************************************************************/

template <typename T> TQuat<T> great_circle_rotation(const TVec3<T> & from, const TVec3<T> & to);


/******************************************************************************
 * Compute the normal of a triangle.
 *
 * The triangle is defined by:
 *
 *
 *            v3
 *            ●
 *           / \
 *          /   \
 *         /     \
 *        ●-------●
 *       v1       v2
 *
 *
 * Two edges are constructed:
 *
 *     e1 = v2 - v1
 *
 *     e2 = v3 - v1
 *
 *
 * The cross product:
 *
 *     cross(e1, e2)
 *
 * is perpendicular to both edges.
 *
 * Therefore it is perpendicular to the triangle.
 *
 * The result is normalized before being returned.
 *
 *****************************************************************************/

template <typename T> TVec3<T> normal(const TVec3<T> & v1, const TVec3<T> & v2, const TVec3<T> & v3);



/******************************************************************************
 * Construct a plane from a normal and a point.
 *****************************************************************************/

template <typename T> TPlane<T> plane_from_normal_and_point(const TVec3<T> & normal, const TVec3<T> & point) {
	/*
	 * Plane equation:
	 *
	 *     dot(normal, X) + d = 0
	 *
	 * Since point belongs to the plane:
	 *
	 *     dot(normal, point) + d = 0
	 *
	 * Therefore:
	 *
	 *     d = -dot(normal, point)
	 */

	return {
		normal,
		-dot(normal, point)
	};
}


/******************************************************************************
 * Ray-plane intersection.
 *****************************************************************************/

template <typename T> TVec4<T> ray_plane_intersection(const TRay<T> & ray, const TPlane<T> & plane) {
	/*
	 * Compute:
	 *
	 *     alpha = dot(normal, direction)
	 *
	 * alpha tells us whether the ray direction has a component along the
	 * plane normal.
	 *
	 * If:
	 *
	 *     alpha == 0
	 *
	 * the ray is parallel to the plane.
	 */

	T alpha = dot(plane.normal, ray.dir);


	/*
	 * Compute:
	 *
	 *     beta = -d - dot(normal, start)
	 *
	 * The ray-plane intersection parameter satisfies:
	 *
	 *     alpha * t = beta
	 *
	 * Therefore:
	 *
	 *     t = beta / alpha
	 */

	T beta =
		-plane.d -
		dot(plane.normal, ray.start);


	/*
	 * The function returns:
	 *
	 *     (alpha * start + beta * dir, alpha)
	 *
	 * The first three coordinates correspond to:
	 *
	 *     alpha * intersection_point
	 *
	 * because:
	 *
	 *     intersection_point
	 *
	 *     = start + (beta / alpha) * dir
	 *
	 * Multiplying by alpha:
	 *
	 *     alpha * intersection_point
	 *
	 *     = alpha * start + beta * dir
	 *
	 *
	 * The fourth component contains alpha.
	 *
	 * This representation avoids performing the division immediately.
	 *
	 * To obtain the actual point:
	 *
	 *     intersection.xyz / intersection.w
	 */

	return {
		alpha * ray.start + beta * ray.dir,
		alpha
	};
}


/******************************************************************************
 * Compute the quaternion rotation from one vector to another.
 *****************************************************************************/

template <typename T> TQuat<T> great_circle_rotation(const TVec3<T> & from, const TVec3<T> & to) {
	/*
	 * This algorithm assumes both vectors are normalized.
	 */

	assert(approx_equal<T>(norm(from), 1));
	assert(approx_equal<T>(norm(to), 1));


	/*
	 * The dot product gives:
	 *
	 *     cos(angle)
	 */

	T cos_angle = dot(from, to);


	/*
	 * Special case:
	 *
	 *     cos(angle) = -1
	 *
	 * means:
	 *
	 *     angle = 180 degrees.
	 *
	 * The cross product is zero in this case, so we need to explicitly
	 * choose an arbitrary axis perpendicular to the vector.
	 */

	if (approx_equal<T>(cos_angle, -1)) {

		return {
			{ 0, 0, 1 },
			0
		};
	}


	/*
	 * Quaternion representation of a rotation:
	 *
	 *     q = (axis * sin(theta/2), cos(theta/2))
	 *
	 *
	 * Here:
	 *
	 *     cos(theta) = dot(from, to)
	 *
	 * and:
	 *
	 *     cos(theta/2)
	 *
	 * is computed using the half-angle identity:
	 *
	 *     cos(theta/2)
	 *
	 *     = sqrt((1 + cos(theta)) / 2)
	 */

	T cos_half_angle =
		sqrt((1.f + cos_angle) / 2.f);


	assert(cos_half_angle != 0);


	/*
	 * Quaternion scalar component.
	 */

	T w = cos_half_angle;


	/*
	 * The vector component is proportional to:
	 *
	 *     cross(from, to)
	 *
	 * which gives the rotation axis.
	 */

	TVec3<T> xyz =
		cross(from, to) *
		(0.5f / cos_half_angle);


	return { xyz, w };
}


/******************************************************************************
 * Compute a triangle normal.
 *****************************************************************************/

template <typename T> TVec3<T> normal(const TVec3<T> & v1, const TVec3<T> & v2, const TVec3<T> & v3) {
	/*
	 * Construct two triangle edges.
	 *
	 *        v3
	 *        ●
	 *       /|
	 *      / |
	 *     /  |
	 *
	 *   v1---● v2
	 *
	 *     e1 = v2 - v1
	 *
	 *     e2 = v3 - v1
	 */

	TVec3<T> e1 = v2 - v1;

	TVec3<T> e2 = v3 - v1;


	/*
	 * cross(e1, e2) is perpendicular to the triangle.
	 */

	TVec3<T> n = cross(e1, e2);


	/*
	 * Normalize the normal vector.
	 *
	 * The returned vector therefore has length 1.
	 */

	return normalized(n);
}