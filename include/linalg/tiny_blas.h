#pragma once

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef USE_OPENMP
    #include <omp.h>
#endif


/******************************************************************************
 * Tiny BLAS : basic vector operations.
 *
 * This file provides a small collection of vector operations commonly found
 * in the BLAS (Basic Linear Algebra Subprograms) interface.
 *
 * These functions do NOT call an external BLAS implementation such as
 * OpenBLAS, MKL or BLIS. The operations are implemented directly using
 * C/C++ loops and can optionally be parallelized with OpenMP.
 *
 * The main operations provided are:
 *
 *     blas_copy
 *         Copy one vector into another.
 *
 *     blas_axpy
 *         Compute:
 *
 *             y = a * x + y
 *
 *     blas_axpby
 *         Compute:
 *
 *             y = a * x + b * y
 *
 *     blas_dot
 *         Compute the dot product:
 *
 *             x^T * y
 *
 *     blas_sum
 *         Compute the sum of all elements of a vector.
 *
 *     blas_sum_in_place
 *         Compute a vector sum using an in-place reduction.
 *
 * The naming follows the usual BLAS terminology so that these routines can
 * easily be replaced by calls to an optimized BLAS implementation if needed or for further implementation (maybe !).
 *
 *****************************************************************************/


/******************************************************************************
 * Copy a vector.
 *
 * Compute:
 *
 *     dest = src
 *
 * The vectors must contain at least N double-precision values.
 *
 * This operation is conceptually equivalent to the BLAS DCOPY routine.
 *
 *****************************************************************************/
void inline blas_copy(const double *__restrict src, double *__restrict dest, size_t N) {memcpy(dest, src, N * sizeof(double));}

/******************************************************************************
 * Perform the AXPY operation.
 *
 * Compute:
 *
 *     y = a * x + y
 *
 * where:
 *
 *     x : input vector
 *     y : input/output vector
 *     a : scalar coefficient
 *
 * This operation is conceptually equivalent to the BLAS DAXPY routine.
 *
 * If OpenMP support is enabled, the vector operation is parallelized over
 * the N elements.
 *
 *****************************************************************************/
void inline blas_axpy(double a, const double *__restrict x, double *__restrict y, size_t N) {
#ifdef USE_OPENMP
    #pragma omp parallel for
#endif
    for (size_t i = 0; i < N; ++i) {
        y[i] += a * x[i];
    }
}


/******************************************************************************
 * Perform the AXPBY operation.
 *
 * Compute:
 *
 *     y = a * x + b * y
 *
 * where:
 *
 *     x : input vector
 *     y : input/output vector
 *     a : coefficient applied to x
 *     b : coefficient applied to y
 *
 * This operation is useful in several numerical algorithms, including the
 * Conjugate Gradient solver.
 *
 * If OpenMP support is enabled, the vector operation is parallelized over
 * the N elements.
 *
 *****************************************************************************/
void inline blas_axpby(double a, const double *__restrict x, double b, double *__restrict y, size_t N) {
#ifdef USE_OPENMP
    #pragma omp parallel for
#endif
    for (size_t i = 0; i < N; ++i) {
        y[i] = a * x[i] + b * y[i];
    }
}


/******************************************************************************
 * Compute the dot product of two vectors.
 *
 * Compute:
 *
 *     x^T * y = sum(x[i] * y[i])
 *
 * The result is a scalar containing the dot product of x and y.
 *
 * When OpenMP is enabled, the reduction is performed in parallel.
 *
 * This operation is conceptually equivalent to the BLAS DDOT routine.
 *
 *****************************************************************************/
double inline blas_dot_old(const double * x, const double * y, size_t N) {
    double res = 0.0;

#ifdef USE_OPENMP
    #pragma omp parallel for reduction(+ : res)
#endif
    for (size_t i = 0; i < N; ++i) {
        res += x[i] * y[i];
    }

    return res;
}


/******************************************************************************
 * Compute the sum of all elements of a vector.
 *
 * Compute:
 *
 *     sum = sum(x[i])
 *
 * The reduction can be parallelized with OpenMP.
 *
 * Note:
 *     This operation is not a standard BLAS level-1 routine, but is useful
 *     for vector and numerical computations.
 *
 *****************************************************************************/
double inline blas_sum(const double *x, size_t N) {
    double sum = 0.0;

#ifdef USE_OPENMP
    #pragma omp parallel for reduction(+ : sum)
#endif
    for (size_t i = 0; i < N; ++i) {
        sum += x[i];
    }

    return sum;
}


/******************************************************************************
 * Compute the sum of a vector using an in-place reduction.
 *
 * The function progressively combines elements of the vector until the
 * complete sum is stored in x[0].
 *
 * For example:
 *
 *     x = [1, 2, 3, 4]
 *
 * eventually gives:
 *
 *     x[0] = 10
 *
 * The input vector is modified by the operation.
 *
 * This reduction strategy can be useful when experimenting with parallel
 * reduction algorithms.
 *
 *****************************************************************************/
double inline blas_sum_in_place(double * x, size_t N) {
    size_t step = 1;

    while (step < N) {

        size_t i = step;

        while (i < N) {
            x[i - step] += x[i];
            i += 2 * step;
        }

        step *= 2;
    }

    return x[0];
}


/******************************************************************************
 * Alternative implementation of the dot product.
 *
 * The function first computes the element-wise products:
 *
 *     buf[i] = x[i] * y[i]
 *
 * and then reduces the temporary vector:
 *
 *     result = sum(buf[i])
 *
 * This implementation is kept as an alternative to blas_dot_old().
 *
 * It requires an additional temporary buffer of N double-precision values
 * and therefore introduces extra memory allocation and memory traffic.
 *
 *****************************************************************************/
double inline blas_dot_new(const double * x, const double * y, size_t N) {
    double *buf = (double *)malloc(N * sizeof(double));

    for (size_t i = 0; i < N; ++i) {
        buf[i] = x[i] * y[i];
    }

    double res = blas_sum_in_place(buf, N);

    free(buf);

    return res;
}


/******************************************************************************
 * Compute the dot product of two vectors.
 *
 * This is the public dot-product routine used by the numerical algorithms.
 *
 * The current implementation uses blas_dot_old(), which performs the
 * reduction directly and can optionally use OpenMP.
 *
 * This wrapper makes it possible to replace the implementation later with
 * an optimized BLAS routine such as:
 *
 *     cblas_ddot()
 *
 * without changing the code of the numerical solvers.
 *
 *****************************************************************************/
double inline blas_dot(const double *x, const double *y, size_t N) {return blas_dot_old(x, y, N);}