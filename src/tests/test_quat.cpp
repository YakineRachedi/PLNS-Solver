#include "test_utils.h"

#include <cmath>

#include "quat.h"

static bool approx(float a, float b, float epsilon = 1e-5f) { return std::fabs(a - b) < epsilon ;}

static void test_constructor() {
    Quat q(1.0f, 2.0f, 3.0f, 4.0f);

    check(q.x == 1.0f, "Quat: constructor x");
    check(q.y == 2.0f, "Quat: constructor y");
    check(q.z == 3.0f, "Quat: constructor z");
    check(q.w == 4.0f, "Quat: constructor w");
}

static void test_identity() {
    Quat q = Quat::Identity;

    check(q.x == 0.0f, "Quat: identity x");
    check(q.y == 0.0f, "Quat: identity y");
    check(q.z == 0.0f, "Quat: identity z");
    check(q.w == 1.0f, "Quat: identity w");
}

static void test_conjugate() {
    Quat q(1.0f, 2.0f, 3.0f, 4.0f);

    Quat result = q.conj();

    check(result.x == -1.0f, "Quat: conjugate x");
    check(result.y == -2.0f, "Quat: conjugate y");
    check(result.z == -3.0f, "Quat: conjugate z");
    check(result.w == 4.0f, "Quat: conjugate w");
}


static void test_norm() {
    Quat q(1.0f, 2.0f, 2.0f, 0.0f);
    check(approx(norm(q), 3.0f),
          "Quat: norm");
}

static void test_normalise() {
    Quat q(2.0f, 0.0f, 0.0f, 0.0f);
    q.normalise();
    check(approx(norm(q), 1.0f), "Quat: normalise produces unit quaternion");
}


/******************************************************************************
 * Test inverse.
 *
 * q * q^-1 = Identity
 *****************************************************************************/

static void test_inverse() {
    Quat q(1.0f, 2.0f, 3.0f, 4.0f);
    Quat inv = q.inv();
    Quat result = q * inv;

    check(approx(result.x, 0.0f), "Quat: inverse result x");
    check(approx(result.y, 0.0f), "Quat: inverse result y");
    check(approx(result.z, 0.0f), "Quat: inverse result z");
    check(approx(result.w, 1.0f), "Quat: inverse result w");
}

static void test_identity_multiplication() {
    Quat q(1.0f, 2.0f, 3.0f, 4.0f);

    Quat I = Quat::Identity;
    Quat result1 = q * I;
    Quat result2 = I * q;

    check(result1.x == q.x && result1.y == q.y && result1.z == q.z && result1.w == q.w, "Quat: q * Identity");
    check(result2.x == q.x && result2.y == q.y && result2.z == q.z && result2.w == q.w, "Quat: Identity * q");
}


/******************************************************************************
 * Test quaternion multiplication.
 *
 * The following two quaternions represent 180 degree rotations:
 *
 * qx = rotation around X
 *
 * qy = rotation around Y
 *
 * This also verifies that quaternion multiplication works.
 *****************************************************************************/

static void test_multiplication() {
    
    Quat qx(1.0f, 0.0f, 0.0f, 0.0f);
    Quat qy(0.0f, 1.0f, 0.0f, 0.0f);

    Quat result = qx * qy;


    /*
     * qx * qy = (0, 0, 1, 0)
     */

    check(approx(result.x, 0.0f), "Quat: multiplication x");
    check(approx(result.y, 0.0f), "Quat: multiplication y");
    check(approx(result.z, 1.0f), "Quat: multiplication z");
    check(approx(result.w, 0.0f), "Quat: multiplication w");
}

int main() {
    LOG_MSG("Quaternion tests\n");
    LOG_MSG("----------------\n");

    test_constructor();

    test_identity();

    test_conjugate();

    test_norm();

    test_normalise();

    test_inverse();

    test_identity_multiplication();

    test_multiplication();


    if (!g_all_passed) {

        LOG_MSG("\nTEST FAILED\n");

        return 1;
    }

    LOG_MSG("\nTEST PASSED\n");

    return 0;
}