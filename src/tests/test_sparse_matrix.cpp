#include "test_utils.h"
#include "sparse_matrix.h"

/******************************************************************************
 * Unit tests for CSRMatrix (non-symmetric and symmetric storage).
 *****************************************************************************/

/******************************************************************************
 * Non-symmetric CSR matrix, taken directly from the CSRPattern documentation
 * example :
 *
 *     [ 10   0   20   0 ]
 *     [  0  30    0  40 ]
 *     [ 50   0    0   0 ]
 *
 * rows = 3, cols = 4, nnz = 5
 *****************************************************************************/
static void test_non_symmetric() {
    CSRMatrix A;
    A.rows = 3;
    A.cols = 4;
    A.nnz  = 5;
    A.symmetric = false;

    /* row_start / col are normally owned by a CSRPattern; here we build the
     * arrays manually and point CSRMatrix at them, like production code
     * would after constructing the pattern. */
    static uint32_t row_start[4] = {0, 2, 4, 5};
    static uint32_t col[5]       = {0, 2, 1, 3, 0};

    A.row_start = row_start;
    A.col       = col;

    A.data.resize(5);
    A.data[0] = 10.0; A.data[1] = 20.0; A.data[2] = 30.0;
    A.data[3] = 40.0; A.data[4] = 50.0;

    /* operator()(i,j) on existing entries */
    check(almost_equal(A(0, 2), 20.0), "CSR non-sym: A(0,2) == 20");
    check(almost_equal(A(1, 3), 40.0), "CSR non-sym: A(1,3) == 40");
    check(almost_equal(A(2, 0), 50.0), "CSR non-sym: A(2,0) == 50");

    /* mvp with x = [1,1,1,1] */
    double x[4] = {1.0, 1.0, 1.0, 1.0};
    double y[3];
    A.mvp(x, y);

    check(almost_equal(y[0], 30.0) && almost_equal(y[1], 70.0) &&
          almost_equal(y[2], 50.0),
          "CSR non-sym: mvp(ones) matches expected result");

    /* sum() */
    double expected_sum = 150.0;
    check(almost_equal(A.sum(), expected_sum), "CSR non-sym: sum() == 150");
}

/******************************************************************************
 * Symmetric CSR matrix. Only the lower triangular part (including the
 * diagonal, stored as the LAST entry of each row) is kept :
 *
 * Full symmetric matrix represented :
 *
 *     [ 4  1  2 ]
 *     [ 1  5  3 ]
 *     [ 2  3  6 ]
 *
 * Stored data (row-major, diagonal last per row) :
 *
 *     row 0 : (col=0, val=4)                       <- diagonal only
 *     row 1 : (col=0, val=1), (col=1, val=5)        <- diagonal last
 *     row 2 : (col=0, val=2), (col=1, val=3), (col=2, val=6)
 *
 * nnz = 6
 *****************************************************************************/
static void test_symmetric() {
    CSRMatrix A;
    A.rows = 3;
    A.cols = 3;
    A.nnz  = 6;
    A.symmetric = true;

    static uint32_t row_start[4] = {0, 1, 3, 6};
    static uint32_t col[6]       = {0, 0, 1, 0, 1, 2};

    A.row_start = row_start;
    A.col       = col;

    A.data.resize(6);
    A.data[0] = 4.0;
    A.data[1] = 1.0; A.data[2] = 5.0;
    A.data[3] = 2.0; A.data[4] = 3.0; A.data[5] = 6.0;

    /* mvp with x = [1,1,1] */
    double x[3] = {1.0, 1.0, 1.0};
    double y[3];
    A.mvp(x, y);

    check(almost_equal(y[0], 7.0) && almost_equal(y[1], 9.0) &&
          almost_equal(y[2], 11.0),
          "CSR sym: mvp(ones) matches expected result");

    /* mvp with x = e0 -> should return column 0 of the full symmetric matrix */
    double x_e0[3] = {1.0, 0.0, 0.0};
    A.mvp(x_e0, y);

    check(almost_equal(y[0], 4.0) && almost_equal(y[1], 1.0) &&
          almost_equal(y[2], 2.0),
          "CSR sym: mvp(e0) matches expected column");

    /* sum() : full matrix sum is 4+1+2+1+5+3+2+3+6 = 27 */
    double expected_sum = 27.0;
    check(almost_equal(A.sum(), expected_sum), "CSR sym: sum() == 27");
}

int main() {
    printf("CSRMatrix unit tests\n");
    printf("--------------------\n");

    test_non_symmetric();
    test_symmetric();

    if (!g_all_passed) {
        printf("\nTEST FAILED\n");
        return 1;
    }

    printf("\nTEST PASSED\n");
    return 0;
}