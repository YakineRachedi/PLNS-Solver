#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "conjugate_gradient.h"
#include "matrix.h"
#include "tiny_blas.h"


/******************************************************************************
 * Perform one iteration of the Conjugate Gradient (CG) algorithm.
 *
 * The goal is to solve the linear system:
 *
 *     A * x = b
 *
 * where A is a square Symmetric Positive Definite (SPD) matrix.
 *
 * The function updates the following vectors:
 *
 *     x
 *         Current approximation of the solution.
 *
 *     r
 *         Residual vector:
 *
 *             r = b - A * x
 *
 *     p
 *         Current conjugate search direction.
 *
 *     Ap
 *         Temporary vector used to store:
 *
 *             Ap = A * p
 *
 * The parameter r2 contains the squared norm of the current residual:
 *
 *     r2 = r^T * r
 *
 * The function performs one complete CG iteration and returns the squared
 * norm of the updated residual:
 *
 *     r2_new = r_new^T * r_new
 *
 *****************************************************************************/
double cg_iterate_once(
    const Matrix &A,
    double *__restrict x,
    double *__restrict r,
    double *__restrict p,
    double *__restrict Ap,
    double r2)
{
    size_t N = A.rows;

    /* The Conjugate Gradient method requires a square matrix. */
    assert(A.rows == A.cols);


    /**************************************************************************
     * Compute the matrix-vector product:
     *
     *     Ap = A * p
     *
     * The Matrix::mvp() function performs the matrix-vector multiplication.
     **************************************************************************/
    A.mvp(p, Ap);


    /**************************************************************************
     * Compute the step size:
     *
     *     alpha_k = (r_k^T * r_k) / (p_k^T * A * p_k)
     *
     * Since Ap = A * p, the denominator is computed as:
     *
     *     p^T * Ap
     **************************************************************************/
    double alpha = r2 / blas_dot(p, Ap, N);


    /**************************************************************************
     * Update the solution:
     *
     *     x_{k+1} = x_k + alpha_k * p_k
     **************************************************************************/
    blas_axpy(alpha, p, x, N);


    /**************************************************************************
     * Update the residual:
     *
     *     r_{k+1} = r_k - alpha_k * A * p_k
     *
     * Since Ap contains A * p, this becomes:
     *
     *     r = r - alpha * Ap
     **************************************************************************/
    blas_axpy(-alpha, Ap, r, N);


    /**************************************************************************
     * Compute the squared norm of the new residual:
     *
     *     r2_new = r_{k+1}^T * r_{k+1}
     **************************************************************************/
    double r2_new = blas_dot(r, r, N);


    /**************************************************************************
     * Compute the coefficient used to construct the next conjugate
     * search direction:
     *
     *     beta_k =
     *         (r_{k+1}^T * r_{k+1}) /
     *         (r_k^T * r_k)
     **************************************************************************/
    double beta = r2_new / r2;


    /**************************************************************************
     * Update the search direction:
     *
     *     p_{k+1} = r_{k+1} + beta_k * p_k
     *
     * blas_axpby() performs:
     *
     *     p = alpha * r + beta * p
     *
     * with alpha = 1 here.
     **************************************************************************/
    blas_axpby(1, r, beta, p, N);


    return r2_new;
}


/******************************************************************************
 * Solve a linear system using the Conjugate Gradient (CG) method.
 *
 * The system to solve is:
 *
 *     A * x = b
 *
 * where A is a square Symmetric Positive Definite (SPD) matrix.
 *
 * The function iteratively improves the solution x until either:
 *
 *     - the relative residual becomes smaller than 'tol';
 *     - the maximum number of iterations 'max_iter' is reached.
 *
 * The vectors x, r, p and Ap are allocated by the caller and are used by
 * the CG algorithm:
 *
 *     x
 *         Current solution.
 *
 *     r
 *         Residual vector:
 *
 *             r = b - A * x
 *
 *     p
 *         Conjugate search direction.
 *
 *     Ap
 *         Temporary vector used to store A * p.
 *
 * The parameter 'rel_error' is used to return the final relative residual:
 *
 *     ||r|| / ||b||
 *
 * The 'inited' parameter allows the function either to initialize a new
 * Conjugate Gradient solve or to continue an already initialized iteration.
 *
 * If inited == false:
 *
 *     r = b - A * x
 *     p = r
 *
 * If inited == true, the existing values of r and p are preserved and the
 * solver continues from the current CG state.
 *
 * The function returns the number of iterations performed.
 *
 *****************************************************************************/
size_t conjugate_gradient_solve(
    const Matrix &A,
    const double *__restrict b,
    double *__restrict x,
    double *__restrict r,
    double *__restrict p,
    double *__restrict Ap,
    double *rel_error,
    double tol,
    int max_iter,
    bool inited)
{
    size_t N = A.rows;

    /* The Conjugate Gradient method requires a square matrix. */
    assert(A.rows == A.cols);


    /**************************************************************************
     * Compute the squared norm of the right-hand side:
     *
     *     b2 = b^T * b = ||b||^2
     *
     * This value is used to compute the relative residual.
     **************************************************************************/
    double b2 = blas_dot(b, b, N);


    /**************************************************************************
     * Initialize the CG iteration if this is a new solve.
     *
     * The initial residual is:
     *
     *     r_0 = b - A * x_0
     **************************************************************************/
    if (!inited) {

        /* Compute A * x and temporarily store it in r. */
        A.mvp(x, r);

        /* Compute r = b - A * x. */
        blas_axpby(1, b, -1, r, N);

        /*
         * The first search direction is the initial residual:
         *
         *     p_0 = r_0
         */
        blas_copy(r, p, N);
    }


    /**************************************************************************
     * Compute the squared norm of the current residual:
     *
     *     r2 = r^T * r = ||r||^2
     **************************************************************************/
    double r2 = blas_dot(r, r, N);


    /**************************************************************************
     * Compute the relative residual:
     *
     *     ||r|| / ||b||
     *
     * This quantity is used as the convergence criterion.
     **************************************************************************/
    *rel_error = sqrt(r2 / b2);


    /**************************************************************************
     * Perform CG iterations until convergence or until the maximum number
     * of iterations is reached.
     **************************************************************************/
    int iter = 0;

    while ((iter < max_iter) && (*rel_error > tol)) {

        /*
         * Perform one complete CG iteration and obtain the squared norm
         * of the updated residual.
         */
        r2 = cg_iterate_once(A, x, r, p, Ap, r2);

        /*
         * Update the relative residual after the iteration.
         */
        *rel_error = sqrt(r2 / b2);

        ++iter;
    }


    /* Return the number of iterations performed. */
    return iter;
}