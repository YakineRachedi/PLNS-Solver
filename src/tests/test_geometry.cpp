#include "test_utils.h"

#include <cmath>

#include "geometry.h"

static bool approx(float a, float b, float epsilon = 1e-5f) { return std::fabs(a - b) < epsilon ;}


/******************************************************************************
 * Test plane construction.
 *
 * Create the plane:
 *
 *     z = 5
 *
 * Its equation is:
 *
 *     z - 5 = 0
 *
 * Therefore:
 *
 *     a = 0
 *     b = 0
 *     c = 1
 *     d = -5
 *****************************************************************************/

static void test_plane_from_normal_and_point() {
    Vec3 normal(0.0f, 0.0f, 1.0f);
    Vec3 point(0.0f, 0.0f, 5.0f);

    Plane plane = plane_from_normal_and_point(normal, point);

    check(plane.a == 0.0f, "Geometry: plane coefficient a");
    check(plane.b == 0.0f, "Geometry: plane coefficient b");
    check(plane.c == 1.0f, "Geometry: plane coefficient c");
    check(plane.d == -5.0f, "Geometry: plane coefficient d");
}


/******************************************************************************
 * Test ray-plane intersection.
 *
 * Plane:
 *
 *     z = 0
 *
 * Ray:
 *
 *     start = (0, 0, 1)
 *     dir   = (0, 0, -1)
 *
 * The ray intersects the plane at:
 *
 *     (0, 0, 0)
 *****************************************************************************/

static void test_ray_plane_intersection() {
    Plane plane;

    plane.normal = Vec3(0.0f, 0.0f, 1.0f);

    plane.d = 0.0f;


    Ray ray;

    ray.start = Vec3(0.0f, 0.0f, 1.0f);

    ray.dir = Vec3(0.0f, 0.0f, -1.0f);


    Vec4 result = ray_plane_intersection(ray, plane);


    /*
     * The function returns:
     *
     * result.xyz = intersection numerator
     *
     * result.w   = alpha
     *
     * The actual intersection point is obtained
     * by dividing xyz by w.
     */

    Vec3 intersection(result.x / result.w, result.y / result.w, result.z / result.w);

    check(approx(intersection.x, 0.0f), "Geometry: ray-plane intersection x");
    check(approx(intersection.y, 0.0f), "Geometry: ray-plane intersection y");
    check(approx(intersection.z, 0.0f), "Geometry: ray-plane intersection z");
}


/******************************************************************************
 * Test triangle normal.
 *
 * Triangle:
 *
 *      (0,1,0)
 *          *
 *          |
 *          |
 *          *
 *         /
 *       /
 *
 * (0,0,0) ---- (1,0,0)
 *
 * The normal is:
 *
 *     (0, 0, 1)
 *****************************************************************************/

static void test_triangle_normal() {
    Vec3 v1(0.0f, 0.0f, 0.0f);
    Vec3 v2(1.0f, 0.0f, 0.0f);
    Vec3 v3(0.0f, 1.0f, 0.0f);
    Vec3 n = normal(v1, v2, v3);

    check(approx(n.x, 0.0f), "Geometry: triangle normal x");
    check(approx(n.y, 0.0f), "Geometry: triangle normal y");
    check(approx(n.z, 1.0f), "Geometry: triangle normal z");
}


/******************************************************************************
 * Test great circle rotation.
 *
 * Rotate:
 *
 *     from = (1,0,0)
 *
 * to:
 *
 *     to = (0,1,0)
 *
 * The required rotation is 90 degrees around Z.
 *
 * Quaternion:
 *
 *     q = (0, 0, sin(pi/4), cos(pi/4))
 *
 *       = (0, 0, sqrt(2)/2, sqrt(2)/2)
 *****************************************************************************/

static void test_great_circle_rotation() {

    Vec3 from(1.0f, 0.0f, 0.0f);
    Vec3 to(0.0f, 1.0f, 0.0f);
    Quat q = great_circle_rotation(from, to);

    float expected = std::sqrt(0.5f);

    check(approx(q.x, 0.0f), "Geometry: great circle rotation x");
    check(approx(q.y, 0.0f), "Geometry: great circle rotation y");
    check(approx(q.z, expected), "Geometry: great circle rotation z");
    check(approx(q.w, expected), "Geometry: great circle rotation w");
}


int main() {
    LOG_MSG("Geometry tests\n");
    LOG_MSG("--------------\n");

    test_plane_from_normal_and_point();
    test_ray_plane_intersection();
    test_triangle_normal();
    test_great_circle_rotation();


    if (!g_all_passed) {

        LOG_MSG("\nTEST FAILED\n");

        return 1;
    }

    LOG_MSG("\nTEST PASSED\n");

    return 0;
}