#include "sparse_matrix.h"

#include <cassert>
#include <cstddef>

#ifdef USE_OPENMP
	#include <omp.h>
#endif

#ifdef USE_KOKKOS
	#include <Kokkos_Core.hpp>
	#include <Kokkos_ScatterView.hpp>
#endif

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
	
	#if defined(USE_KOKKOS)
		/*
     	* Kokkos backend.
     	*
     	* For a symmetric CSR matrix, each stored coefficient contributes
     	* to two entries of y. Several rows may therefore update the same
     	* y entry, so ScatterView is used to avoid write conflicts.
     	*/

		Kokkos::View<const double*> x_view(x, cols);
    	Kokkos::View<double*> y_view(y, rows);
		Kokkos::deep_copy(y_view, 0.0);
		
		if (!symmetric) {

        Kokkos::parallel_for("CSR_mvp", rows, KOKKOS_LAMBDA(const size_t i) {
                double sum = 0.0;

                const size_t start = row_start[i];
                const size_t stop  = row_start[i + 1];

                for (size_t k = start; k < stop; ++k) {
                    assert(k < nnz);
                    assert(col[k] < cols);

                    sum += data[k] * x_view(col[k]);
                }

                y_view(i) = sum;
            }
        );

    	} else {

        Kokkos::Experimental::ScatterView<double*> y_scatter(y_view);
        Kokkos::parallel_for("CSR_mvp_symmetric", rows, KOKKOS_LAMBDA(const size_t i) {

			auto y_access = y_scatter.access();

			const size_t start = row_start[i];
			const size_t stop  = row_start[i + 1];

			for (size_t k = start; k < stop; ++k) {

				assert(k < nnz);
				assert(col[k] < cols);

				const size_t j = col[k];

				if (j == i) {
					// Diagonal contribution.
					y_access(i) += data[k] * x_view(i);
				} else {
					// A(i,j) contribution.
					y_access(i) += data[k] * x_view(j);

					// Symmetric A(j,i) contribution.
					y_access(j) += data[k] * x_view(i);
					}
				}
			}
		);

        Kokkos::fence();
        Kokkos::Experimental::contribute(y_view, y_scatter);
    }
	
    Kokkos::fence();
	
#elif defined(USE_OPENMP)

    // Initialize output vector.
    #pragma omp parallel for
    for (size_t i = 0; i < rows; ++i) {
        y[i] = 0.0;
    }

    if (!symmetric) {

        // Each row writes to a different y[i].
        #pragma omp parallel for
        for (size_t i = 0; i < rows; ++i) {

            double sum = 0.0;

            const size_t start = row_start[i];
            const size_t stop  = row_start[i + 1];

            for (size_t k = start; k < stop; ++k) {
                assert(k < nnz);
                assert(col[k] < cols);

                sum += data[k] * x[col[k]];
            }

            y[i] = sum;
        }

    } else {

        // Symmetric matrix:
        // each stored coefficient contributes to two entries
        // of the full matrix, except diagonal coefficients.
        #pragma omp parallel for
        for (size_t i = 0; i < rows; ++i) {

            const size_t start = row_start[i];
            const size_t stop  = row_start[i + 1];

            for (size_t k = start; k < stop; ++k) {

                assert(k < nnz);
                assert(col[k] < cols);

                const size_t j = col[k];
                const double a = data[k];

                if (j == i) {

                    // Diagonal contribution.
                    #pragma omp atomic
                    y[i] += a * x[i];

                } else {

                    // A(i,j) contribution.
                    #pragma omp atomic
                    y[i] += a * x[j];

                    // Symmetric A(j,i) contribution.
                    #pragma omp atomic
                    y[j] += a * x[i];
                }
            }
        }
    }

	#else

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
	#endif
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

	#if defined(USE_KOKKOS)

		double res = 0.0;
		// Sum of all stored coefficients.
		Kokkos::parallel_reduce("CSR_sum", nnz, KOKKOS_LAMBDA(const size_t k, double &local_sum) {local_sum += data[k] ;}, res);

		if (symmetric) {

			/*
			* Off-diagonal coefficients are stored once but represent
			* two entries in the full symmetric matrix.
			*/
			res *= 2.0;
			double diagonal_sum = 0.0;
			Kokkos::parallel_reduce("CSR_diagonal_sum", rows, KOKKOS_LAMBDA(const size_t i, double &local_sum) {
					const size_t index = row_start[i + 1] - 1; assert(col[index] == i); local_sum += data[index] ;},
															diagonal_sum
			);
			res -= diagonal_sum;
		}

		Kokkos::fence();
		return res;


	#elif defined(USE_OPENMP)

		double res = 0.0;
		// Sum of all stored coefficients.
		#pragma omp parallel for reduction(+:res)
		for (size_t k = 0; k < nnz; ++k) {res += data[k] ;}

		if (symmetric) {

			res *= 2.0;
			double diagonal_sum = 0.0;

			#pragma omp parallel for reduction(+:diagonal_sum)
			for (size_t i = 0; i < rows; ++i) {

				const size_t index = row_start[i + 1] - 1;
				assert(col[index] == i);
				diagonal_sum += data[index];
			}

			res -= diagonal_sum;
		}

		return res;

	#else
		// Sequential backend.

		double res = 0.0;
		for (size_t k = 0; k < nnz; ++k) res += data[k];

		if (symmetric) {
			res *= 2.0;
			for (size_t i = 0; i < rows; ++i) {

				const size_t index = row_start[i + 1] - 1;
				assert(col[index] == i);
				res -= data[index];
			}
		}

		return res;

	#endif
}