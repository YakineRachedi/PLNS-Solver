#pragma once

#include "matrix.h"


/******************************************************************************
 * Conjugate Gradient (CG) solver
 *
 * The Conjugate Gradient method is an iterative algorithm used to solve
 * linear systems of the form:
 *
 *     A * x = b
 *
 * where A is a square Symmetric Positive Definite (SPD) matrix.
 *
 * The algorithm does not require the matrix A to be stored as a dense matrix.
 * It only needs to compute matrix-vector products:
 *
 *     A * p
 *
 * This makes CG particularly suitable for large sparse systems, such as the
 * linear systems obtained from the finite element discretization of Poisson
 * or other PDE problems.
 *
 * The implementation uses four auxiliary vectors:
 *
 *     x
 *         Current approximation of the solution.
 *
 *     r
 *         Residual vector:
 *
 *             r = b - A * x
 *
 *         It measures how far the current solution is from satisfying the
 *         linear system.
 *
 *     p
 *         Search direction used to update the solution.
 *
 *     Ap
 *         Temporary vector used to store the matrix-vector product:
 *
 *             Ap = A * p
 *
 * The caller is responsible for allocating the memory required by these
 * vectors.
 *
 *****************************************************************************/


/******************************************************************************
 * Perform one Conjugate Gradient iteration.
 *
 * This function updates the CG state:
 *
 *     - x  : current solution
 *     - r  : residual
 *     - p  : search direction
 *     - Ap : matrix-vector product
 *
 * The function returns the squared Euclidean norm of the updated residual:
 *
 *     r2 = ||r||^2
 *
 * The squared norm is used instead of the norm itself to avoid an unnecessary
 * square-root computation during the iterations.
 *
 * The matrix A must be square and Symmetric Positive Definite (SPD).
 *
 * The vectors x, r, p and Ap must already be allocated by the caller.
 *
 *****************************************************************************/
double cg_iterate_once(
    const Matrix &A,
    double *__restrict x,
    double *__restrict r,
    double *__restrict p,
    double *__restrict Ap,
    double r2);


/******************************************************************************
 * Solve a linear system using the Conjugate Gradient method.
 *
 * Solves:
 *
 *     A * x = b
 *
 * iteratively until one of the following conditions is reached:
 *
 *     - the relative residual is smaller than the requested tolerance;
 *     - the maximum number of iterations is reached.
 *
 * The vectors used internally by the algorithm are provided by the caller:
 *
 *     b
 *         Right-hand side of the linear system.
 *
 *     x
 *         Solution vector / current iterate.
 *
 *     r
 *         Residual vector.
 *
 *     p
 *         Search direction.
 *
 *     Ap
 *         Temporary vector used to store A * p.
 *
 *     rel_error
 *         Output value containing the relative residual/error obtained by
 *         the solver.
 *
 *     tol
 *         Convergence tolerance.
 *
 *     max_iter
 *         Maximum number of CG iterations.
 *
 *     inited
 *         Indicates whether the CG iteration state has already been
 *         initialized.
 *
 * If inited is false, the solver initializes the CG state from b and x
 * and starts a new CG solve.
 *
 * If inited is true, the existing values of r and p are preserved and the
 * solver continues from the current CG state. This makes it possible to
 * execute the algorithm in batches of iterations, for example when the
 * convergence history needs to be inspected between batches.
 *
 * The function returns the number of CG iterations performed.
 *
 *****************************************************************************/
size_t conjugate_gradient_solve(
    const Matrix &A,
    const double *__restrict b,
    double *__restrict x,
    double *__restrict r,
    double *__restrict p,
    double *__restrict Ap,
    double *__restrict rel_error,
    double tol,
    int max_iter,
    bool inited = false);