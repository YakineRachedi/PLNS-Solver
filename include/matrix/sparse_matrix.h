#pragma once

#include <stdint.h>

#include "array.h"
#include "matrix.h"


/******************************************************************************
 * CSRPattern : Compressed Sparse Row matrix structure.
 *
 * CSR (Compressed Sparse Row) is a sparse matrix storage format designed
 * to store only the non-zero entries of a matrix.
 *
 * Instead of storing every coefficient of a dense matrix, CSR stores:
 *
 *   - row_start : indices delimiting the entries belonging to each row;
 *   - col       : column index of each stored non-zero entry.
 *
 * The actual numerical values are stored separately in CSRMatrix::data.
 *
 * For a matrix with 'rows' rows and 'nnz' non-zero entries:
 *
 *   row_start has size rows + 1
 *   col       has size nnz
 *
 * For row i, the non-zero entries are stored at:
 *
 *   row_start[i] <= k < row_start[i + 1]
 *
 * and the corresponding column of entry k is:
 *
 *   col[k]
 *
 * Example:
 *
 *      Matrix:
 *
 *          [ 10   0   20   0 ]
 *          [  0  30    0  40 ]
 *          [ 50   0    0   0 ]
 *
 *      Non-zero values:
 *
 *          10, 20, 30, 40, 50
 *
 *      Column indices:
 *
 *          0, 2, 1, 3, 0
 *
 *      row_start:
 *
 *          [ 0, 2, 4, 5 ]
 *
 *      Therefore:
 *
 *          Row 0 -> entries k = 0 .. 1
 *          Row 1 -> entries k = 2 .. 3
 *          Row 2 -> entry  k = 4
 *
 * The 'symmetric' flag indicates whether the matrix is known to be
 * symmetric. A symmetric matrix satisfies:
 *
 *          A(i,j) = A(j,i)
 *
 * In that case, a solver may be able to exploit the symmetry and store
 * only part of the matrix.
 *
 *****************************************************************************/

struct CSRPattern {
    bool symmetric;
    size_t rows;
    size_t cols;
    size_t nnz;

    /*
     * For row i (0 <= i < rows), the corresponding non-zero entries
     * are stored at indices:
     *
     *     row_start[i] <= k < row_start[i + 1]
     *
     * The column index of entry k is stored in col[k].
     *
     * row_start has size rows + 1.
     */
    TArray<uint32_t> row_start;

    /*
     * Column index of each stored non-zero entry.
     *
     * col[k] gives the column index of the k-th stored entry.
     *
     * col has size nnz.
     */
    TArray<uint32_t> col;
};


/******************************************************************************
 * CSRMatrix : Numerical values associated with a CSR sparse matrix pattern.
 *
 * CSRMatrix stores the numerical coefficients of a sparse matrix using the
 * Compressed Sparse Row (CSR) format.
 *
 * The sparsity pattern (row_start and col) is provided by CSRPattern,
 * while the numerical values are stored in 'data'.
 *
 * The three arrays describe the matrix:
 *
 *     row_start : where each row starts and ends;
 *     col       : column index of each non-zero entry;
 *     data      : numerical value of each non-zero entry.
 *
 * The arrays 'row_start' and 'col' are pointers to the corresponding
 * arrays owned by the CSRPattern. CSRMatrix therefore does not allocate
 * separate storage for these two arrays.
 *
 * For a given row i:
 *
 *     for (k = row_start[i]; k < row_start[i + 1]; ++k)
 *     {
 *         column = col[k];
 *         value  = data[k];
 *     }
 *
 * This represents:
 *
 *     A(i, column) = value
 *
 * Main operations:
 *
 *     - mvp(x, y) :
 *         Compute the matrix-vector product
 *
 *             y = A * x
 *
 *     - sum() :
 *         Compute the sum of all stored matrix coefficients.
 *
 *     - operator()(i, j) :
 *         Access the coefficient A(i,j).
 *
 * The matrix inherits from Matrix, which provides the common matrix
 * interface used by the rest of the numerical code.
 *
 *****************************************************************************/

struct CSRMatrix : public Matrix {

    /*
     * Indicates whether the matrix is symmetric:
     *
     *     A(i,j) = A(j,i)
     */
    bool symmetric = false;

    /*
     * Number of stored non-zero entries.
     */
    size_t nnz;

    /*
     * Pointers to the CSR sparsity pattern.
     *
     * These arrays are owned by CSRPattern and are not duplicated here.
     *
     * row_start has size rows + 1.
     * col has size nnz.
     */
    uint32_t *row_start;
    uint32_t *col;

    /*
     * Numerical value associated with each non-zero entry.
     *
     * data[k] corresponds to the matrix coefficient located at:
     *
     *     row determined by row_start
     *     column = col[k]
     *
     * data has size nnz.
     */
    TArray<double> data;

    /*
     * Matrix-vector product:
     *
     *     y = A * x
     *
     * x is the input vector and y is the output vector.
     *
     * __restrict indicates that, for this function, x and y are assumed
     * not to refer to overlapping memory regions. This gives the compiler
     * additional information that can potentially be used for optimization.
     */
    void mvp(const double *__restrict x,
             double *__restrict y) const;

    /*
     * Return the sum of all stored matrix coefficients.
     */
    double sum() const;

    /*
     * Access the matrix coefficient A(i,j).
     *
     * The non-zero entries are searched in row i using the CSR structure.
     */
    double & operator()(uint32_t i, uint32_t j);
};