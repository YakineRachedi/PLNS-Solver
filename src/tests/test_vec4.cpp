#include "test_utils.h"

#include "vec4.h"

static void test_constructors() {
    Vec4 v(1.0f, 2.0f, 3.0f, 4.0f);

    check(v.x == 1.0f, "Vec4: constructor x");
    check(v.y == 2.0f, "Vec4: constructor y");
    check(v.z == 3.0f, "Vec4: constructor z");
    check(v.w == 4.0f, "Vec4: constructor w");


    Vec3 xyz(10.0f, 20.0f, 30.0f);

    Vec4 v2(xyz, 40.0f);

    check(v2.x == 10.0f, "Vec4: Vec3 constructor x");
    check(v2.y == 20.0f, "Vec4: Vec3 constructor y");
    check(v2.z == 30.0f, "Vec4: Vec3 constructor z");
    check(v2.w == 40.0f, "Vec4: Vec3 constructor w");
}

static void test_array_constructor() {
    float data[] = {1.0f, 2.0f, 3.0f, 4.0f};

    Vec4 v(data);

    check(v.x == 1.0f, "Vec4: array constructor x");
    check(v.y == 2.0f, "Vec4: array constructor y");
    check(v.z == 3.0f, "Vec4: array constructor z");
    check(v.w == 4.0f, "Vec4: array constructor w");
}

static void test_index() {
    Vec4 v(10.0f, 20.0f, 30.0f, 40.0f);

    check(v[0] == 10.0f, "Vec4: index 0");
    check(v[1] == 20.0f, "Vec4: index 1");
    check(v[2] == 30.0f, "Vec4: index 2");
    check(v[3] == 40.0f, "Vec4: index 3");

    v[2] = 100.0f;

    check(v.z == 100.0f, "Vec4: index write");
}

static void test_equality() {
    Vec4 a(1.0f, 2.0f, 3.0f, 4.0f);
    Vec4 b(1.0f, 2.0f, 3.0f, 4.0f);
    Vec4 c(1.0f, 2.0f, 3.0f, 5.0f);

    check(a == b, "Vec4: equal vectors");

    check(!(a == c), "Vec4: different vectors");
}

static void test_add_subtract() {
    
    Vec4 a(1.0f, 2.0f, 3.0f, 4.0f);
    Vec4 b(10.0f, 20.0f, 30.0f, 40.0f);
    Vec4 sum = a + b;

    check(sum == Vec4(11.0f, 22.0f, 33.0f, 44.0f),
          "Vec4: addition");


    Vec4 diff = b - a;

    check(diff == Vec4(9.0f, 18.0f, 27.0f, 36.0f),
          "Vec4: subtraction");
}

static void test_negation() {
    
    Vec4 a(1.0f, -2.0f, 3.0f, -4.0f);
    Vec4 result = -a;

    check(result == Vec4(-1.0f, 2.0f, -3.0f, 4.0f),
          "Vec4: unary minus");
}

static void test_compound_operators() {
    Vec4 a(1.0f, 2.0f, 3.0f, 4.0f);
    Vec4 b(10.0f, 20.0f, 30.0f, 40.0f);

    a += b;

    check(a == Vec4(11.0f, 22.0f, 33.0f, 44.0f),
          "Vec4: operator +=");

    a -= b;

    check(a == Vec4(1.0f, 2.0f, 3.0f, 4.0f),
          "Vec4: operator -=");
}

static void test_scalar_multiplication() {
    Vec4 v(1.0f, 2.0f, 3.0f, 4.0f);

    Vec4 result1 = v * 2.0f;

    check(result1 == Vec4(2.0f, 4.0f, 6.0f, 8.0f),
          "Vec4: vector * scalar");


    Vec4 result2 = 3.0f * v;

    check(result2 == Vec4(3.0f, 6.0f, 9.0f, 12.0f),
          "Vec4: scalar * vector");
}


/******************************************************************************
 * Test *= and /=.
 *****************************************************************************/

static void test_scalar_compound() {
    Vec4 v(2.0f, 4.0f, 6.0f, 8.0f);

    v *= 2.0f;

    check(v == Vec4(4.0f, 8.0f, 12.0f, 16.0f),
          "Vec4: operator *=");


    v /= 2.0f;

    check(v == Vec4(2.0f, 4.0f, 6.0f, 8.0f),
          "Vec4: operator /=");
}


/******************************************************************************
 * Test dot product.
 *
 * dot(a, b) =
 *
 *     ax*bx + ay*by + az*bz + aw*bw
 *****************************************************************************/

static void test_dot() {
    
    Vec4 a(1.0f, 2.0f, 3.0f, 4.0f);
    Vec4 b(5.0f, 6.0f, 7.0f, 8.0f);
    float result = dot(a, b);

    /*
     * 1*5 + 2*6 + 3*7 + 4*8
     *
     * = 5 + 12 + 21 + 32
     *
     * = 70
     */

    check(result == 70.0f, "Vec4: dot product");
}

static void test_norm() {
    Vec4 v(1.0f, 2.0f, 2.0f, 0.0f);

    /*
     * ||v|| =
     *
     * sqrt(1² + 2² + 2²)
     *
     * = sqrt(9)
     *
     * = 3
     */

    check(norm(v) == 3.0f, "Vec4: norm");
}

int main()
{
    LOG_MSG("Vec4 tests\n");
    LOG_MSG("----------\n");

    test_constructors();
    test_array_constructor();

    test_index();

    test_equality();

    test_add_subtract();
    test_negation();
    test_compound_operators();

    test_scalar_multiplication();
    test_scalar_compound();

    test_dot();
    test_norm();


    if (!g_all_passed) {

        LOG_MSG("\nTEST FAILED\n");

        return 1;
    }

    LOG_MSG("\nTEST PASSED\n");

    return 0;
}