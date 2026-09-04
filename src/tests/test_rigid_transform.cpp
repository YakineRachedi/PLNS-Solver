#include "test_utils.h"

#include <cmath>

#include "transform.h"


static bool approx(float a, float b, float epsilon = 1e-5f) { return std::fabs(a - b) < epsilon ;}

static void test_identity() {
    RigT transform = RigT::Identity;
    check(transform.trans == Vec3::Zero, "RigT: identity translation");
    check(transform.rot.x == 0.0f && transform.rot.y == 0.0f && transform.rot.z == 0.0f && transform.rot.w == 1.0f, "RigT: identity rotation");
}


static void test_identity_matrix() {

    RigT transform = RigT::Identity;
    Mat4 M = transform.as_matrix();

    check(approx(M(0, 0), 1.0f), "RigT: identity matrix (0,0)");
    check(approx(M(1, 1), 1.0f), "RigT: identity matrix (1,1)");
    check(approx(M(2, 2), 1.0f), "RigT: identity matrix (2,2)");
    check(approx(M(3, 3), 1.0f), "RigT: identity matrix (3,3)");
}

static void test_translation_matrix() {

    RigT transform;
    transform.rot = Quat::Identity;
    transform.trans = Vec3(10.0f, 20.0f, 30.0f);
    Mat4 M = transform.as_matrix();

    check(approx(M(0, 3), 10.0f), "RigT: translation x");
    check(approx(M(1, 3), 20.0f), "RigT: translation y");
    check(approx(M(2, 3), 30.0f), "RigT: translation z");
}


/******************************************************************************
 * Test transform and inverse.
 *
 * Applying a transformation followed by its inverse
 * must recover the original point.
 *****************************************************************************/

static void test_inverse() {

    RigT T;
    T.rot = Quat::Identity;
    T.trans = Vec3(1.0f, 2.0f, 3.0f);
    RigT inverse = T.inv();

    check(approx(inverse.trans.x, -1.0f), "RigT: inverse translation x");
    check(approx(inverse.trans.y, -2.0f), "RigT: inverse translation y");
    check(approx(inverse.trans.z, -3.0f), "RigT: inverse translation z");
}


int main() {
    LOG_MSG("Rigid Transform tests\n");
    LOG_MSG("---------------------\n");

    test_identity();
    test_identity_matrix();
    test_translation_matrix();
    test_inverse();


    if (!g_all_passed) {

        LOG_MSG("\nTEST FAILED\n");

        return 1;
    }

    LOG_MSG("\nTEST PASSED\n");

    return 0;
}