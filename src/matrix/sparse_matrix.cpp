#include "sparse_matrix.h"

/******************************************************************************
 *
 * CSRMatrix implementation
 *
 * The CSR (Compressed Sparse Row) matrix stores only the non-zero entries
 * of a sparse matrix.
 *
 * For a given row i, its entries are stored in the range:
 *
 *     row_start[i] <= k < row_start[i + 1]
 *
 * For each storage index k:
 *
 *     col[k]  : column index of the entry
 *     data[k] : corresponding matrix coefficient
 *
 *****************************************************************************/


/******************************************************************************
 * Access a matrix coefficient.
 *
 * Searches for the coefficient A(i, j) in the sparse storage and returns
 * a reference to its value.
 *
 * Since the matrix only stores entries defined by its sparsity pattern,
 * accessing a coefficient that is not present in the pattern is invalid.
 *
 *****************************************************************************/
double &CSRMatrix::operator()(uint32_t i, uint32_t j) {
	static double dummy = 0.0;

	assert(i < rows);

	/* Get the range of non-zero entries stored for row i */
	size_t start = row_start[i];
	size_t stop = row_start[i + 1];

	/* Search for column j in row i */
	for (size_t k = start; k < stop; ++k) {
		if (col[k] == j)
			return data[k];
	}

	/* The requested coefficient is not part of the sparsity pattern */
	assert(false);

	/* Unreachable in normal execution, required to return a reference */
	return dummy;
}


/******************************************************************************
 * Compute the matrix-vector product:
 *
 *     y = A * x
 *
 * The multiplication is performed directly from the CSR representation,
 * without constructing a dense matrix.
 *
 * If the matrix is symmetric, only one triangular part may be stored.
 * The corresponding symmetric contributions must then also be added when
 * computing the product.
 *
 *****************************************************************************/
void CSRMatrix::mvp(const double *__restrict x, double *__restrict y) const {
	for (size_t i = 0; i < rows; ++i) {
		y[i] = 0;
		size_t start = row_start[i];
		size_t stop = row_start[i + 1];
		for (size_t k = start; k < stop; ++k) {
			assert(k < nnz);
			assert(col[k] < cols);
			y[i] += data[k] * x[col[k]];
		}
	}
	if (symmetric) {
		for (size_t i = 0; i < rows; ++i) {
			size_t start = row_start[i];
			/* stop before the diagonal */
			size_t stop = row_start[i + 1] - 1;
			for (size_t k = start; k < stop; ++k) {
				y[col[k]] += data[k] * x[i];
			}
		}
	}
}

/******************************************************************************
 * Compute the sum of all matrix coefficients.
 *
 * For a non-symmetric matrix, all stored coefficients are summed directly.
 *
 * For a symmetric matrix, only one triangular part is stored. The off-diagonal
 * coefficients therefore represent two matrix entries:
 *
 *     A(i, j) and A(j, i)
 *
 * The stored sum is first multiplied by two, then the diagonal coefficients
 * are subtracted once because they must only be counted once.
 *
 *****************************************************************************/
double CSRMatrix::sum() const {
	double res = 0.0;

	/* Sum all coefficients stored in the CSR arrays */
	for (size_t k = 0; k < nnz; k++) {
		res += data[k];
	}

	if (symmetric) {
		/*
		 * Only one triangular part is stored.
		 * Each off-diagonal coefficient therefore contributes twice
		 * to the full symmetric matrix.
		 */
		res *= 2;

		/*
		 * Diagonal coefficients were also multiplied by two, but they
		 * only exist once in the full matrix. Subtract them once.
		 *
		 * The sparsity pattern stores the diagonal entry as the last
		 * entry of each row.
		 */
		for (size_t k = 0; k < rows; k++) {
			assert(col[row_start[k + 1] - 1] == k);
			res -= data[row_start[k + 1] - 1];
		}
	}

	return res;
}