#include <cstdio>
#include <cmath>

#include "matrix.h"
#include "conjugate_gradient.h"

/******************************************************************************
 * Basic Conjugate Gradient Test
 *
 * Solves a small symmetric positive definite linear system and compares
 * the computed solution with the known exact solution.
 *
 * This test does not require a mesh, OpenGL, or the FEM solver.
 * It is intended to verify that the linear solver works correctly.
 * 
 * 
 * TestMatrix : Small 2x2 dense matrix used to test the Conjugate Gradient
 *              solver.
 *
 * The matrix is:
 *
 *     [ 4  1 ]
 * A = [      ]
 *     [ 1  3 ]
 *
 *****************************************************************************/

struct TestMatrix : public Matrix {
    double data[4] = {
        4.0, 1.0,
        1.0, 3.0
    };

    TestMatrix() {rows = 2; cols = 2;}
    /* Matrix-vector product: y = A * x */
    void mvp(const double *__restrict x, double *__restrict y) const override {
        y[0] = 4.0 * x[0] + 1.0 * x[1];
        y[1] = 1.0 * x[0] + 3.0 * x[1];
    }

    /* Sum of all matrix coefficients */
    double sum() const override{return 4.0 + 1.0 + 1.0 + 3.0;}
};


/******************************************************************************
 * Test the Conjugate Gradient solver.
 *
 * Solve:
 *
 *     A * x = b
 *
 * with:
 *
 *     [ 4  1 ] [ x0 ]   [ 1 ]
 *     [ 1  3 ] [ x1 ] = [ 2 ]
 *
 * Expected solution:
 *
 *     x0 = 1 / 11 ~= 0.090909
 *     x1 = 7 / 11 ~= 0.636364
 *
 *****************************************************************************/

int main() {
    TestMatrix A;

    /* Right-hand side */
    double b[2] = {1.0,
                    2.0};

    /* Initial solution: x_0 = 0 */
    double x[2] = {0.0,
                    0.0};

    /* CG working vectors */
    double r[2]  = {0.0, 0.0};
    double p[2]  = {0.0, 0.0};
    double Ap[2] = {0.0, 0.0};

    double rel_error = 0.0;

    size_t iterations = conjugate_gradient_solve(
        A,
        b,
        x,
        r,
        p,
        Ap,
        &rel_error,
        1e-10,
        100,
        false
    );

    double expected_x0 = 1.0 / 11.0;
	double expected_x1 = 7.0 / 11.0;

	double error0 = std::abs(x[0] - expected_x0);
	double error1 = std::abs(x[1] - expected_x1);

    printf("CG test\n");
    printf("-------\n");

    printf("Iterations : %zu\n", iterations);
    printf("Relative error : %.15e\n", rel_error);

    printf("\nComputed solution:\n");
    printf("x[0] = %.15f\n", x[0]);
    printf("x[1] = %.15f\n", x[1]);

    printf("\nExpected solution:\n");
    printf("x[0] = %.15f\n", 1.0 / 11.0);
    printf("x[1] = %.15f\n", 7.0 / 11.0);

    const double tolerance = 1e-8;

	if (error0 > tolerance || error1 > tolerance) {
		printf("TEST FAILED\n");
		return 1;
	}

	printf("TEST PASSED\n");
    
    return 0;
}