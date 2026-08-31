#include "test_meshes.h"
#include "test_utils.h"
#include "fem_matrix.h"

/******************************************************************************
 * Unit tests for FEMatrix (P1_cst, P1_sym, P1_gen).
 *
 * Mesh used for every test : two triangles sharing one edge.
 *
 *     positions : 4 vertices (coordinates are irrelevant here, mvp/sum
 *                 only use mesh connectivity)
 *     indices   : [0,1,2, 0,2,3]
 *
 * All expected values below were computed by hand.
 *****************************************************************************/


/******************************************************************************
 * P1_cst : one common off-diagonal coefficient per triangle.
 *
 * diag     = {1, 2, 3, 4}
 * off_diag = {10, 20}   (triangle 0 -> 10, triangle 1 -> 20)
 *
 * Resulting dense matrix :
 *
 *     A(0,0)=1  A(0,1)=10 A(0,2)=30 A(0,3)=20
 *     A(1,0)=10 A(1,1)=2  A(1,2)=10 A(1,3)=0
 *     A(2,0)=30 A(2,1)=10 A(2,2)=3  A(2,3)=20
 *     A(3,0)=20 A(3,1)=0  A(3,2)=20 A(3,3)=4
 *****************************************************************************/
static void test_P1_cst(const Mesh & mesh) {
    FEMatrix A;
    A.fem_type = FEMatrix::P1_cst;
    A.m = &mesh;
    A.rows = mesh.vertex_count();
    A.cols = mesh.vertex_count();

    A.diag.resize(4);
    A.diag[0] = 1.0; A.diag[1] = 2.0; A.diag[2] = 3.0; A.diag[3] = 4.0;

    A.off_diag.resize(2);
    A.off_diag[0] = 10.0;
    A.off_diag[1] = 20.0;

    /* mvp with x = e0 -> should return column 0 of A. */
    double x_e0[4] = {1.0, 0.0, 0.0, 0.0};
    double y[4];
    A.mvp(x_e0, y);

    check(almost_equal(y[0], 1.0) && almost_equal(y[1], 10.0) &&
          almost_equal(y[2], 30.0) && almost_equal(y[3], 20.0),
          "P1_cst: mvp(e0) matches expected column");

    /* sum() */
    double expected_sum = 190.0;
    check(almost_equal(A.sum(), expected_sum), "P1_cst: sum() matches expected total");

    /* Invariant : sum(A * ones) == sum(A) */
    double ones[4] = {1.0, 1.0, 1.0, 1.0};
    A.mvp(ones, y);
    double row_sum = y[0] + y[1] + y[2] + y[3];
    check(almost_equal(row_sum, expected_sum), "P1_cst: sum(mvp(ones)) == sum()");
}

/******************************************************************************
 * P1_sym : three symmetric off-diagonal coefficients per triangle.
 *
 * diag     = {1, 2, 3, 4}
 * off_diag = {5, 6, 7,   8, 9, 11}   (triangle 0, triangle 1)
 *
 * Resulting dense matrix :
 *
 *     A(0,0)=1  A(0,1)=5  A(0,2)=15 A(0,3)=11
 *     A(1,0)=5  A(1,1)=2  A(1,2)=6  A(1,3)=0
 *     A(2,0)=15 A(2,1)=6  A(2,2)=3  A(2,3)=9
 *     A(3,0)=11 A(3,1)=0  A(3,2)=9  A(3,3)=4
 *****************************************************************************/
static void test_P1_sym(const Mesh & mesh) {
    FEMatrix A;
    A.fem_type = FEMatrix::P1_sym;
    A.m = &mesh;
    A.rows = mesh.vertex_count();
    A.cols = mesh.vertex_count();

    A.diag.resize(4);
    A.diag[0] = 1.0; A.diag[1] = 2.0; A.diag[2] = 3.0; A.diag[3] = 4.0;

    A.off_diag.resize(6);
    A.off_diag[0] = 5.0;  A.off_diag[1] = 6.0;  A.off_diag[2] = 7.0;
    A.off_diag[3] = 8.0;  A.off_diag[4] = 9.0;  A.off_diag[5] = 11.0;

    double x_e0[4] = {1.0, 0.0, 0.0, 0.0};
    double y[4];
    A.mvp(x_e0, y);

    check(almost_equal(y[0], 1.0) && almost_equal(y[1], 5.0) &&
          almost_equal(y[2], 15.0) && almost_equal(y[3], 11.0),
          "P1_sym: mvp(e0) matches expected column");

    double expected_sum = 102.0;
    check(almost_equal(A.sum(), expected_sum), "P1_sym: sum() matches expected total");

    double ones[4] = {1.0, 1.0, 1.0, 1.0};
    A.mvp(ones, y);
    double row_sum = y[0] + y[1] + y[2] + y[3];
    check(almost_equal(row_sum, expected_sum), "P1_sym: sum(mvp(ones)) == sum()");
}

/******************************************************************************
 * P1_gen : six independent off-diagonal coefficients per triangle.
 *
 * diag     = {1, 2, 3, 4}
 * off_diag (triangle 0) = {1, 2, 3, 4, 5, 6}
 * off_diag (triangle 1) = {7, 8, 9, 10, 11, 12}
 *
 * Resulting (non-symmetric) dense matrix :
 *
 *     A(0,0)=1  A(0,1)=1  A(0,2)=13 A(0,3)=12
 *     A(1,0)=2  A(1,1)=2  A(1,2)=3  A(1,3)=0
 *     A(2,0)=13 A(2,1)=4  A(2,2)=3  A(2,3)=9
 *     A(3,0)=11 A(3,1)=0  A(3,2)=10 A(3,3)=4
 *****************************************************************************/
static void test_P1_gen(const Mesh & mesh) {
    FEMatrix A;
    A.fem_type = FEMatrix::P1_gen;
    A.m = &mesh;
    A.rows = mesh.vertex_count();
    A.cols = mesh.vertex_count();

    A.diag.resize(4);
    A.diag[0] = 1.0; A.diag[1] = 2.0; A.diag[2] = 3.0; A.diag[3] = 4.0;

    A.off_diag.resize(12);
    double coeffs[12] = {1,2,3,4,5,6, 7,8,9,10,11,12};
    for (int i = 0; i < 12; ++i) A.off_diag[i] = coeffs[i];

    /* mvp with x = e0 -> column 0 of A (matrix is NOT symmetric here). */
    double x_e0[4] = {1.0, 0.0, 0.0, 0.0};
    double y[4];
    A.mvp(x_e0, y);

    check(almost_equal(y[0], 1.0) && almost_equal(y[1], 2.0) &&
          almost_equal(y[2], 13.0) && almost_equal(y[3], 11.0),
          "P1_gen: mvp(e0) matches expected column");

    double expected_sum = 88.0;
    check(almost_equal(A.sum(), expected_sum), "P1_gen: sum() matches expected total");

    double ones[4] = {1.0, 1.0, 1.0, 1.0};
    A.mvp(ones, y);

    check(almost_equal(y[0], 27.0) && almost_equal(y[1], 7.0) &&
          almost_equal(y[2], 29.0) && almost_equal(y[3], 25.0),
          "P1_gen: mvp(ones) matches expected row sums");

    double row_sum = y[0] + y[1] + y[2] + y[3];
    check(almost_equal(row_sum, expected_sum), "P1_gen: sum(mvp(ones)) == sum()");
}

int main() {
    Mesh mesh;
    setup_unit_square_mesh(mesh);

    printf("FEMatrix unit tests\n");
    printf("-------------------\n");

    test_P1_cst(mesh);
    test_P1_sym(mesh);
    test_P1_gen(mesh);

    if (!g_all_passed) {
        printf("\nTEST FAILED\n");
        return 1;
    }

    printf("\nTEST PASSED\n");
    return 0;
}