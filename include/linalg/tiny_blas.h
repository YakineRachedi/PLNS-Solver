#pragma once

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef USE_KOKKOS
    #include <Kokkos_Core.hpp>
#endif

#ifdef USE_OPENBLAS
    #include <cblas.h>
#endif

#ifdef USE_OPENMP
    #include <omp.h>
#endif

/******************************************************************************
 * Tiny BLAS : basic vector operations.
 *
 * This file provides a small collection of vector operations commonly found
 * in the BLAS interface.
 *
 * Four backends are supported, selected at compile time via preprocessor
 * definitions (in order of priority):
 *
 *     USE_KOKKOS   : parallel_for / parallel_reduce (portable CPU/GPU)
 *     USE_OPENBLAS : calls to an external OpenBLAS implementation
 *     USE_OPENMP   : #pragma omp parallel for
 *     (none)       : plain sequential C++ loops
 *
 * The naming follows the usual BLAS terminology so that these routines can
 * be used as drop-in replacements regardless of the active backend.
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
void inline blas_copy(const double *__restrict src, double *__restrict dest, size_t N) {

    #if defined(USE_KOKKOS)
        Kokkos::parallel_for("blas_copy", N, KOKKOS_LAMBDA(size_t i) { dest[i] = src[i] ;});
        Kokkos::fence();
    #elif defined(USE_OPENBLAS)
        cblas_dcopy((int)N, src, 1, dest, 1);
    #elif defined(USE_OPENMP)
        #pragma omp parallel for
            for (size_t i = 0; i < N; ++i) dest[i] = src[i];
    #else
        memcpy(dest, src, N * sizeof(double));

    #endif
}

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
 *
 *****************************************************************************/
void inline blas_axpy(double a, const double *__restrict x, double *__restrict y, size_t N) {

    #if defined(USE_KOKKOS)
        Kokkos::parallel_for("blas_axpy", N, KOKKOS_LAMBDA(const size_t i) {y[i] += a * x[i];});
        Kokkos::fence();
    #elif defined(USE_OPENBLAS)
        cblas_daxpy((int)N, a, x, 1, y, 1);
    #elif defined(USE_OPENMP)
        #pragma omp parallel for
            for (size_t i = 0; i < N; ++i) y[i] += a * x[i];
    #else
        for (size_t i = 0; i < N; ++i) y[i] += a * x[i];
    #endif
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
 * Useful in several numerical algorithms, including the Conjugate Gradient
 * solver. Not a standard BLAS Level-1 routine under this exact name, but
 * OpenBLAS/cblas exposes it as an extension (cblas_daxpby) on most builds.
 *
 *****************************************************************************/

void inline blas_axpby(double a, const double *__restrict x, double b, double *__restrict y, size_t N) {
    
    #if defined(USE_KOKKOS)
        Kokkos::parallel_for("blas_axpby", N, KOKKOS_LAMBDA(const size_t i) { y[i] = a * x[i] + b * y[i] ;});
        Kokkos::fence();
    #elif defined(USE_OPENBLAS)
        cblas_daxpby((int)N, a, x, 1, b, y, 1);

    #elif defined(USE_OPENMP)
        #pragma omp parallel for
            for (size_t i = 0; i < N; ++i) y[i] = a * x[i] + b * y[i];
    #else
        for (size_t i = 0; i < N; ++i) y[i] = a * x[i] + b * y[i];
    #endif
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
 * This operation is conceptually equivalent to the BLAS DDOT routine.
 *
 *****************************************************************************/
double inline blas_dot(const double *x, const double *y, size_t N) {
    #if defined(USE_KOKKOS)
        double res = 0.0;
        Kokkos::parallel_reduce("blas_dot", N, KOKKOS_LAMBDA(const size_t i, double &lsum) {lsum += x[i] * y[i] ;}, res);
    return res;

    #elif defined(USE_OPENBLAS)
    return cblas_ddot((int)N, x, 1, y, 1);

    #elif defined(USE_OPENMP)
        double res = 0.0;
        #pragma omp parallel for reduction(+ : res)
            for (size_t i = 0; i < N; ++i) res += x[i] * y[i];
    return res;
    #else
        double res = 0.0;
        for (size_t i = 0; i < N; ++i) res += x[i] * y[i];
    return res;

#endif
}


/******************************************************************************
 * Compute the sum of all elements of a vector.
 *
 * Compute:
 *
 *     sum = sum(x[i])
 *
 * Note:
 *     This operation is not a standard BLAS level-1 routine, but is useful
 *     for vector and numerical computations.
 *
 *****************************************************************************/
double inline blas_sum(const double *x, size_t N) {

    #if defined(USE_KOKKOS)
        double sum = 0.0;
        Kokkos::parallel_reduce("blas_sum", N, KOKKOS_LAMBDA(const size_t i, double &lsum) {lsum += x[i];}, sum);
    return sum;

    #elif defined(USE_OPENMP)
        double sum = 0.0;
        #pragma omp parallel for reduction(+ : sum)
            for (size_t i = 0; i < N; ++i) sum += x[i];
    return sum;

    #else
        double sum = 0.0;
        for (size_t i = 0; i < N; ++i) sum += x[i];
    return sum;

    #endif
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