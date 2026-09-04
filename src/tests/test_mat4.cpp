#include "test_utils.h"

#include "mat4.h"


/******************************************************************************
 * Helper.
 *
 * Initialize a 4x4 identity matrix.
 *****************************************************************************/

static Mat4 identity_matrix() {
    Mat4 M{};

    M(0, 0) = 1.0f;
    M(1, 1) = 1.0f;
    M(2, 2) = 1.0f;
    M(3, 3) = 1.0f;

    return M;
}


/******************************************************************************
 * Test matrix element access.
 *
 * Mat4 stores its data in column-major order:
 *
 *     cols[0]
 *     cols[1]
 *     cols[2]
 *     cols[3]
 *
 * However:
 *
 *     M(i, j)
 *
 * always means:
 *
 *     row i, column j
 *****************************************************************************/

static void test_element_access() {
    Mat4 M{};

    M(0, 0) = 1.0f;
    M(0, 1) = 2.0f;
    M(0, 2) = 3.0f;
    M(0, 3) = 4.0f;

    M(1, 0) = 5.0f;
    M(1, 1) = 6.0f;
    M(1, 2) = 7.0f;
    M(1, 3) = 8.0f;


    check(M(0, 0) == 1.0f, "Mat4: element (0,0)");
    check(M(0, 1) == 2.0f, "Mat4: element (0,1)");
    check(M(0, 2) == 3.0f, "Mat4: element (0,2)");
    check(M(0, 3) == 4.0f, "Mat4: element (0,3)");

    check(M(1, 0) == 5.0f, "Mat4: element (1,0)");
    check(M(1, 1) == 6.0f, "Mat4: element (1,1)");
    check(M(1, 2) == 7.0f, "Mat4: element (1,2)");
    check(M(1, 3) == 8.0f, "Mat4: element (1,3)");
}

static void test_column_access() {
    Mat4 M{};

    M(0) = Vec4(1.0f, 2.0f, 3.0f, 4.0f);

    check(M(0).x == 1.0f, "Mat4: column access x");
    check(M(0).y == 2.0f, "Mat4: column access y");
    check(M(0).z == 3.0f, "Mat4: column access z");
    check(M(0).w == 4.0f, "Mat4: column access w");


    /*
     * Since columns are stored directly:
     *
     * M(0,0) = cols[0][0]
     * M(1,0) = cols[0][1]
     * M(2,0) = cols[0][2]
     * M(3,0) = cols[0][3]
     */

    check(M(0, 0) == 1.0f, "Mat4: column maps to row 0");
    check(M(1, 0) == 2.0f, "Mat4: column maps to row 1");
    check(M(2, 0) == 3.0f, "Mat4: column maps to row 2");
    check(M(3, 0) == 4.0f, "Mat4: column maps to row 3");
}


/******************************************************************************
 * Test multiplication by the identity matrix.
 *
 * A * I = A
 * I * A = A
 *****************************************************************************/

static void test_identity_multiplication() {
    Mat4 A{};

    A(0, 0) = 1.0f;
    A(0, 1) = 2.0f;
    A(0, 2) = 3.0f;
    A(0, 3) = 4.0f;

    A(1, 0) = 5.0f;
    A(1, 1) = 6.0f;
    A(1, 2) = 7.0f;
    A(1, 3) = 8.0f;

    A(2, 0) = 9.0f;
    A(2, 1) = 10.0f;
    A(2, 2) = 11.0f;
    A(2, 3) = 12.0f;

    A(3, 0) = 13.0f;
    A(3, 1) = 14.0f;
    A(3, 2) = 15.0f;
    A(3, 3) = 16.0f;


    Mat4 I = identity_matrix();


    Mat4 result1 = A * I;

    Mat4 result2 = I * A;


    bool correct1 = true;
    bool correct2 = true;


    for (int i = 0; i < 4; ++i) {

        for (int j = 0; j < 4; ++j) {

            if (result1(i, j) != A(i, j)) {
                correct1 = false;
            }

            if (result2(i, j) != A(i, j)) {
                correct2 = false;
            }
        }
    }


    check(correct1, "Mat4: A * Identity = A");

    check(correct2, "Mat4: Identity * A = A");
}


/******************************************************************************
 * Test matrix multiplication.
 *
 * Use simple diagonal matrices.
 *
 * A =
 *
 * [2 0 0 0]
 * [0 2 0 0]
 * [0 0 2 0]
 * [0 0 0 2]
 *
 * B =
 *
 * [3 0 0 0]
 * [0 3 0 0]
 * [0 0 3 0]
 * [0 0 0 3]
 *
 * Therefore:
 *
 * A * B =
 *
 * [6 0 0 0]
 * [0 6 0 0]
 * [0 0 6 0]
 * [0 0 0 6]
 *****************************************************************************/

static void test_matrix_multiplication() {
    Mat4 A{};
    Mat4 B{};


    for (int i = 0; i < 4; ++i) {

        A(i, i) = 2.0f;

        B(i, i) = 3.0f;
    }


    Mat4 C = A * B;


    bool correct = true;


    for (int i = 0; i < 4; ++i) {

        for (int j = 0; j < 4; ++j) {

            float expected = (i == j) ? 6.0f : 0.0f;

            if (C(i, j) != expected) {
                correct = false;
            }
        }
    }


    check(correct, "Mat4: matrix multiplication");
}

int main() {
    LOG_MSG("Mat4 tests\n");
    LOG_MSG("----------\n");

    test_element_access();

    test_column_access();

    test_identity_multiplication();

    test_matrix_multiplication();


    if (!g_all_passed) {

        LOG_MSG("\nTEST FAILED\n");

        return 1;
    }

    LOG_MSG("\nTEST PASSED\n");

    return 0;
}