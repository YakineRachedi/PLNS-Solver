#include "fem_matrix.h"

#include <cassert>
#include <cstddef>

#ifdef USE_OPENMP
    #include <omp.h>
#endif

#ifdef USE_KOKKOS
    #include <Kokkos_Core.hpp>
    #include <Kokkos_ScatterView.hpp>
#endif

// ============================================================================
// Matrix-vector product implementations for the different FEM storage formats.
// ============================================================================

static void mvp_P1_cst(const FEMatrix & A, const double *x, double *y);
static void mvp_P1_sym(const FEMatrix & A, const double *x, double *y);
static void mvp_P1_gen(const FEMatrix & A, const double *x, double *y);


/******************************************************************************
 *
 * FEMatrix matrix-vector product.
 *
 * Dispatch the computation to the implementation corresponding to the finite
 * element matrix storage format.
 *
 *****************************************************************************/

void FEMatrix::mvp(const double *x, double *y) const {
    switch (fem_type) {

    case FEMatrix::P1_cst:
        mvp_P1_cst(*this, x, y);
        return;

    case FEMatrix::P1_sym:
        mvp_P1_sym(*this, x, y);
        return;

    case FEMatrix::P1_gen:
        mvp_P1_gen(*this, x, y);
        return;
    }
}


// ============================================================================
// Matrix coefficient sum implementations for the different FEM formats.
// ============================================================================

static double sum_P1_cst(const FEMatrix & A);
static double sum_P1_sym(const FEMatrix & A);
static double sum_P1_gen(const FEMatrix & A);

// ============================================================================
// Return the sum of all coefficients of the matrix.
// The actual computation depends on the storage format used by the matrix.
// ============================================================================

static double sum_P1_cst(const FEMatrix & A);
static double sum_P1_sym(const FEMatrix & A);
static double sum_P1_gen(const FEMatrix & A);

double FEMatrix::sum() const {
    switch (fem_type) {

    case FEMatrix::P1_cst:
        return sum_P1_cst(*this);

    case FEMatrix::P1_sym:
        return sum_P1_sym(*this);

    case FEMatrix::P1_gen:
        return sum_P1_gen(*this);

    default:
        return 0.0;
    }
}


/******************************************************************************
 *
 * Compute y = A * x for a P1 matrix with one constant off-diagonal
 * coefficient per triangle.
 *
 * For each vertex v, the computation starts with the diagonal contribution:
 *
 *     y[v] = A(v,v) * x[v]
 *
 * Each triangle (a,b,c) then contributes the same off-diagonal coefficient to
 * the six interactions between its three vertices:
 *
 *     a <-> b
 *     b <-> c
 *     c <-> a
 *
 *****************************************************************************/
static void mvp_P1_cst(const FEMatrix &A, const double *x, double *y) {

    assert(A.fem_type == FEMatrix::P1_cst);

    const size_t vtx_count = A.m->vertex_count();
    const size_t tri_count = A.m->triangle_count();

#if defined(USE_KOKKOS)

    Kokkos::View<const double*> x_view(x, vtx_count);
    Kokkos::View<double*> y_view(y, vtx_count);

    /*
     * IMPORTANT:
     * Do not capture FEMatrix A inside KOKKOS_LAMBDA.
     *
     * FEMatrix contains TArray objects, and TArray is non-copyable.
     * Extract raw pointers before entering the lambda.
     */
	const double* diag = A.diag.data;
	const uint32_t* indices = A.m->indices.data;
	const double* off_diag = A.off_diag.data;

    // ------------------------------------------------------------------------
    // Diagonal contribution
    // ------------------------------------------------------------------------

    Kokkos::parallel_for(
        "FEM_mvp_P1_cst_diag",
        vtx_count,
        KOKKOS_LAMBDA(const size_t v) {
            y_view(v) = diag[v] * x_view(v);
        }
    );

    Kokkos::fence();

    // ------------------------------------------------------------------------
    // Off-diagonal contributions
    // ------------------------------------------------------------------------

    Kokkos::Experimental::ScatterView<double*> y_scatter(y_view);

	Kokkos::parallel_for(
		"FEM_mvp_P1_cst",
		tri_count,
		KOKKOS_LAMBDA(const size_t t) {

			auto y_access = y_scatter.access();

			const uint32_t a = indices[3 * t + 0];
			const uint32_t b = indices[3 * t + 1];
			const uint32_t c = indices[3 * t + 2];

			const double mult = off_diag[t];

			y_access(a) += mult * x_view(b);
			y_access(b) += mult * x_view(a);

			y_access(b) += mult * x_view(c);
			y_access(c) += mult * x_view(b);

			y_access(c) += mult * x_view(a);
			y_access(a) += mult * x_view(c);
		}
	);

    Kokkos::fence();

    Kokkos::Experimental::contribute(y_view, y_scatter);

    Kokkos::fence();

#elif defined(USE_OPENMP)

    // ------------------------------------------------------------------------
    // Diagonal contribution
    // ------------------------------------------------------------------------

    #pragma omp parallel for
    for (size_t v = 0; v < vtx_count; ++v) {
        y[v] = A.diag[v] * x[v];
    }

    // ------------------------------------------------------------------------
    // Off-diagonal contributions
    //
    // Several triangles can contribute to the same vertex.
    // Atomics are therefore required.
    // ------------------------------------------------------------------------

    #pragma omp parallel for
    for (size_t t = 0; t < tri_count; ++t) {

        const uint32_t a = A.m->indices[3 * t + 0];
        const uint32_t b = A.m->indices[3 * t + 1];
        const uint32_t c = A.m->indices[3 * t + 2];

        const double mult = A.off_diag[t];

        #pragma omp atomic
        y[a] += mult * x[b];

        #pragma omp atomic
        y[b] += mult * x[a];

        #pragma omp atomic
        y[b] += mult * x[c];

        #pragma omp atomic
        y[c] += mult * x[b];

        #pragma omp atomic
        y[c] += mult * x[a];

        #pragma omp atomic
        y[a] += mult * x[c];
    }

#else

    // ------------------------------------------------------------------------
    // Sequential backend
    // ------------------------------------------------------------------------

    for (size_t v = 0; v < vtx_count; ++v) {
        y[v] = A.diag[v] * x[v];
    }

    for (size_t t = 0; t < tri_count; ++t) {

	const uint32_t a = A.m->indices[3 * t + 0];
	const uint32_t b = A.m->indices[3 * t + 1];
	const uint32_t c = A.m->indices[3 * t + 2];

        const double mult = A.off_diag[t];

        y[a] += mult * x[b];
        y[b] += mult * x[a];

        y[b] += mult * x[c];
        y[c] += mult * x[b];

        y[c] += mult * x[a];
        y[a] += mult * x[c];
    }

#endif
}


/******************************************************************************
 *
 * Compute y = A * x for a symmetric P1 matrix.
 *
 * Each triangle (a,b,c) stores three off-diagonal coefficients corresponding
 * to the three edges:
 *
 *     off_diag[3*t + 0] : interaction between a and b
 *     off_diag[3*t + 1] : interaction between b and c
 *     off_diag[3*t + 2] : interaction between c and a
 *
 * Since the matrix is symmetric, each coefficient contributes in both
 * directions:
 *
 *     A(i,j) = A(j,i)
 *
 *****************************************************************************/
static void mvp_P1_sym(const FEMatrix &A, const double *x, double *y) {

    assert(A.fem_type == FEMatrix::P1_sym);

    const size_t vtx_count = A.m->vertex_count();
    const size_t tri_count = A.m->triangle_count();

#if defined(USE_KOKKOS)

    Kokkos::View<const double*> x_view(x, vtx_count);
    Kokkos::View<double*> y_view(y, vtx_count);

    const double* diag = A.diag.data;
    const uint32_t* indices = A.m->indices.data;
    const double* off_diag = A.off_diag.data;

    // Diagonal contribution
    Kokkos::parallel_for(
        "FEM_mvp_P1_sym_diag",
        vtx_count,
        KOKKOS_LAMBDA(const size_t v) {
            y_view(v) = diag[v] * x_view(v);
        }
    );

    Kokkos::fence();

    // Off-diagonal contributions
    Kokkos::Experimental::ScatterView<double*> y_scatter(y_view);

    Kokkos::parallel_for(
        "FEM_mvp_P1_sym",
        tri_count,
        KOKKOS_LAMBDA(const size_t t) {

            auto y_access = y_scatter.access();

            const uint32_t a = indices[3 * t + 0];
            const uint32_t b = indices[3 * t + 1];
            const uint32_t c = indices[3 * t + 2];

            const double ab = off_diag[3 * t + 0];
            const double bc = off_diag[3 * t + 1];
            const double ca = off_diag[3 * t + 2];

            // Edge (a,b)
            y_access(a) += ab * x_view(b);
            y_access(b) += ab * x_view(a);

            // Edge (b,c)
            y_access(b) += bc * x_view(c);
            y_access(c) += bc * x_view(b);

            // Edge (c,a)
            y_access(c) += ca * x_view(a);
            y_access(a) += ca * x_view(c);
        }
    );

    Kokkos::fence();

    Kokkos::Experimental::contribute(y_view, y_scatter);

    Kokkos::fence();

#elif defined(USE_OPENMP)

    // Diagonal
    #pragma omp parallel for
    for (size_t v = 0; v < vtx_count; ++v) {
        y[v] = A.diag[v] * x[v];
    }

    // Off-diagonal
    #pragma omp parallel for
    for (size_t t = 0; t < tri_count; ++t) {

        const uint32_t a = A.m->indices[3 * t + 0];
        const uint32_t b = A.m->indices[3 * t + 1];
        const uint32_t c = A.m->indices[3 * t + 2];

        const double ab = A.off_diag[3 * t + 0];
        const double bc = A.off_diag[3 * t + 1];
        const double ca = A.off_diag[3 * t + 2];

        // Edge (a,b)
        #pragma omp atomic
        y[a] += ab * x[b];

        #pragma omp atomic
        y[b] += ab * x[a];

        // Edge (b,c)
        #pragma omp atomic
        y[b] += bc * x[c];

        #pragma omp atomic
        y[c] += bc * x[b];

        // Edge (c,a)
        #pragma omp atomic
        y[c] += ca * x[a];

        #pragma omp atomic
        y[a] += ca * x[c];
    }

#else

    // Diagonal
    for (size_t v = 0; v < vtx_count; ++v) {
        y[v] = A.diag[v] * x[v];
    }

    // Off-diagonal
    for (size_t t = 0; t < tri_count; ++t) {

        const uint32_t a = A.m->indices[3 * t + 0];
        const uint32_t b = A.m->indices[3 * t + 1];
        const uint32_t c = A.m->indices[3 * t + 2];

        const double ab = A.off_diag[3 * t + 0];
        const double bc = A.off_diag[3 * t + 1];
        const double ca = A.off_diag[3 * t + 2];

        // Edge (a,b)
        y[a] += ab * x[b];
        y[b] += ab * x[a];

        // Edge (b,c)
        y[b] += bc * x[c];
        y[c] += bc * x[b];

        // Edge (c,a)
        y[c] += ca * x[a];
        y[a] += ca * x[c];
    }

#endif
}


/******************************************************************************
 *
 * Compute y = A * x for a general P1 matrix.
 *
 * No symmetry is assumed. Each triangle therefore stores six independent
 * off-diagonal coefficients:
 *
 *     A(a,b), A(b,a)
 *     A(b,c), A(c,b)
 *     A(c,a), A(a,c)
 *
 *****************************************************************************/
static void mvp_P1_gen(const FEMatrix &A, const double *x, double *y) {

    assert(A.fem_type == FEMatrix::P1_gen);

    const size_t vtx_count = A.m->vertex_count();
    const size_t tri_count = A.m->triangle_count();

#if defined(USE_KOKKOS)

    Kokkos::View<const double*> x_view(x, vtx_count);
    Kokkos::View<double*> y_view(y, vtx_count);

    const double* diag = A.diag.data;
    const uint32_t* indices = A.m->indices.data;
    const double* off_diag = A.off_diag.data;

    // Diagonal
    Kokkos::parallel_for(
        "FEM_mvp_P1_gen_diag",
        vtx_count,
        KOKKOS_LAMBDA(const size_t v) {
            y_view(v) = diag[v] * x_view(v);
        }
    );

    Kokkos::fence();

    // Off-diagonal
    Kokkos::Experimental::ScatterView<double*> y_scatter(y_view);

    Kokkos::parallel_for(
        "FEM_mvp_P1_gen",
        tri_count,
        KOKKOS_LAMBDA(const size_t t) {

            auto y_access = y_scatter.access();

            const uint32_t a = indices[3 * t + 0];
            const uint32_t b = indices[3 * t + 1];
            const uint32_t c = indices[3 * t + 2];

            // A(a,b)
            y_access(a) += off_diag[6 * t + 0] * x_view(b);

            // A(b,a)
            y_access(b) += off_diag[6 * t + 1] * x_view(a);

            // A(b,c)
            y_access(b) += off_diag[6 * t + 2] * x_view(c);

            // A(c,b)
            y_access(c) += off_diag[6 * t + 3] * x_view(b);

            // A(c,a)
            y_access(c) += off_diag[6 * t + 4] * x_view(a);

            // A(a,c)
            y_access(a) += off_diag[6 * t + 5] * x_view(c);
        }
    );

    Kokkos::fence();

    Kokkos::Experimental::contribute(y_view, y_scatter);

    Kokkos::fence();

#elif defined(USE_OPENMP)

    // Diagonal
    #pragma omp parallel for
    for (size_t v = 0; v < vtx_count; ++v) {
        y[v] = A.diag[v] * x[v];
    }

    // Off-diagonal
    #pragma omp parallel for
    for (size_t t = 0; t < tri_count; ++t) {

        const uint32_t a = A.m->indices[3 * t + 0];
        const uint32_t b = A.m->indices[3 * t + 1];
        const uint32_t c = A.m->indices[3 * t + 2];

        // A(a,b)
        #pragma omp atomic
        y[a] += A.off_diag[6 * t + 0] * x[b];

        // A(b,a)
        #pragma omp atomic
        y[b] += A.off_diag[6 * t + 1] * x[a];

        // A(b,c)
        #pragma omp atomic
        y[b] += A.off_diag[6 * t + 2] * x[c];

        // A(c,b)
        #pragma omp atomic
        y[c] += A.off_diag[6 * t + 3] * x[b];

        // A(c,a)
        #pragma omp atomic
        y[c] += A.off_diag[6 * t + 4] * x[a];

        // A(a,c)
        #pragma omp atomic
        y[a] += A.off_diag[6 * t + 5] * x[c];
    }

#else

    // Diagonal
    for (size_t v = 0; v < vtx_count; ++v) {
        y[v] = A.diag[v] * x[v];
    }

    // Off-diagonal
    for (size_t t = 0; t < tri_count; ++t) {

        const uint32_t a = A.m->indices[3 * t + 0];
        const uint32_t b = A.m->indices[3 * t + 1];
        const uint32_t c = A.m->indices[3 * t + 2];

        // A(a,b)
        y[a] += A.off_diag[6 * t + 0] * x[b];

        // A(b,a)
        y[b] += A.off_diag[6 * t + 1] * x[a];

        // A(b,c)
        y[b] += A.off_diag[6 * t + 2] * x[c];

        // A(c,b)
        y[c] += A.off_diag[6 * t + 3] * x[b];

        // A(c,a)
        y[c] += A.off_diag[6 * t + 4] * x[a];

        // A(a,c)
        y[a] += A.off_diag[6 * t + 5] * x[c];
    }

#endif
}

/******************************************************************************
 *
 * Return the sum of all coefficients of a P1_cst matrix.
 *
 * The diagonal coefficients are stored once per vertex.
 *
 * Each triangle stores one off-diagonal coefficient which represents the six
 * off-diagonal interactions between its three vertices.
 *
 *****************************************************************************/

static double sum_P1_cst(const FEMatrix &A) {

    assert(A.fem_type == FEMatrix::P1_cst);

    const size_t vtx_count = A.m->vertex_count();
    const size_t tri_count = A.m->triangle_count();

    double sum1 = 0.0;
    double sum2 = 0.0;

#if defined(USE_KOKKOS)

    const double* diag = A.diag.data;
    const double* off_diag = A.off_diag.data;

    Kokkos::parallel_reduce(
        "FEM_sum_P1_cst_diag",
        vtx_count,
        KOKKOS_LAMBDA(const size_t v, double &local_sum) {
            local_sum += diag[v];
        },
        sum1
    );

    Kokkos::parallel_reduce(
        "FEM_sum_P1_cst_offdiag",
        tri_count,
        KOKKOS_LAMBDA(const size_t t, double &local_sum) {
            local_sum += 6.0 * off_diag[t];
        },
        sum2
    );

    Kokkos::fence();

#elif defined(USE_OPENMP)

    #pragma omp parallel for reduction(+:sum1)
    for (size_t v = 0; v < vtx_count; ++v) {
        sum1 += A.diag[v];
    }

    #pragma omp parallel for reduction(+:sum2)
    for (size_t t = 0; t < tri_count; ++t) {
        sum2 += 6.0 * A.off_diag[t];
    }

#else

    for (size_t v = 0; v < vtx_count; ++v) {
        sum1 += A.diag[v];
    }

    for (size_t t = 0; t < tri_count; ++t) {
        sum2 += 6.0 * A.off_diag[t];
    }

#endif

    return sum1 + sum2;
}


/******************************************************************************
 *
 * Return the sum of all coefficients of a symmetric P1 matrix.
 *
 * Each triangle stores three off-diagonal coefficients. Since the matrix is
 * symmetric, each coefficient appears twice in the full matrix.
 *
 *****************************************************************************/

 static double sum_P1_sym(const FEMatrix &A) {

    assert(A.fem_type == FEMatrix::P1_sym);

    const size_t vtx_count = A.m->vertex_count();
    const size_t tri_count = A.m->triangle_count();

    double sum1 = 0.0;
    double sum2 = 0.0;

#if defined(USE_KOKKOS)

    const double* diag = A.diag.data;
    const double* off_diag = A.off_diag.data;

    Kokkos::parallel_reduce(
        "FEM_sum_P1_sym_diag",
        vtx_count,
        KOKKOS_LAMBDA(const size_t v, double &local_sum) {
            local_sum += diag[v];
        },
        sum1
    );

    Kokkos::parallel_reduce(
        "FEM_sum_P1_sym_offdiag",
        tri_count,
        KOKKOS_LAMBDA(const size_t t, double &local_sum) {

            local_sum += 2.0 * off_diag[3 * t + 0];
            local_sum += 2.0 * off_diag[3 * t + 1];
            local_sum += 2.0 * off_diag[3 * t + 2];

        },
        sum2
    );

    Kokkos::fence();

#elif defined(USE_OPENMP)

    #pragma omp parallel for reduction(+:sum1)
    for (size_t v = 0; v < vtx_count; ++v) {
        sum1 += A.diag[v];
    }

    #pragma omp parallel for reduction(+:sum2)
    for (size_t t = 0; t < tri_count; ++t) {
        sum2 += 2.0 * A.off_diag[3 * t + 0];
        sum2 += 2.0 * A.off_diag[3 * t + 1];
        sum2 += 2.0 * A.off_diag[3 * t + 2];
    }

#else

    for (size_t v = 0; v < vtx_count; ++v) {
        sum1 += A.diag[v];
    }

    for (size_t t = 0; t < tri_count; ++t) {
        sum2 += 2.0 * A.off_diag[3 * t + 0];
        sum2 += 2.0 * A.off_diag[3 * t + 1];
        sum2 += 2.0 * A.off_diag[3 * t + 2];
    }

#endif

    return sum1 + sum2;
}


/******************************************************************************
 *
 * Return the sum of all coefficients of a general P1 matrix.
 *
 * The six off-diagonal coefficients of every triangle are independent.
 *
 *****************************************************************************/

static double sum_P1_gen(const FEMatrix &A) {

    assert(A.fem_type == FEMatrix::P1_gen);

    const size_t vtx_count = A.m->vertex_count();
    const size_t tri_count = A.m->triangle_count();

    double sum1 = 0.0;
    double sum2 = 0.0;

#if defined(USE_KOKKOS)

    const double* diag = A.diag.data;
    const double* off_diag = A.off_diag.data;

    Kokkos::parallel_reduce(
        "FEM_sum_P1_gen_diag",
        vtx_count,
        KOKKOS_LAMBDA(const size_t v, double &local_sum) {
            local_sum += diag[v];
        },
        sum1
    );

    Kokkos::parallel_reduce(
        "FEM_sum_P1_gen_offdiag",
        tri_count,
        KOKKOS_LAMBDA(const size_t t, double &local_sum) {

            local_sum += off_diag[6 * t + 0];
            local_sum += off_diag[6 * t + 1];
            local_sum += off_diag[6 * t + 2];
            local_sum += off_diag[6 * t + 3];
            local_sum += off_diag[6 * t + 4];
            local_sum += off_diag[6 * t + 5];

        },
        sum2
    );

    Kokkos::fence();

#elif defined(USE_OPENMP)

    #pragma omp parallel for reduction(+:sum1)
    for (size_t v = 0; v < vtx_count; ++v) {
        sum1 += A.diag[v];
    }

    #pragma omp parallel for reduction(+:sum2)
    for (size_t t = 0; t < tri_count; ++t) {
        sum2 += A.off_diag[6 * t + 0];
        sum2 += A.off_diag[6 * t + 1];
        sum2 += A.off_diag[6 * t + 2];
        sum2 += A.off_diag[6 * t + 3];
        sum2 += A.off_diag[6 * t + 4];
        sum2 += A.off_diag[6 * t + 5];
    }

#else

    for (size_t v = 0; v < vtx_count; ++v) {
        sum1 += A.diag[v];
    }

    for (size_t t = 0; t < tri_count; ++t) {
        sum2 += A.off_diag[6 * t + 0];
        sum2 += A.off_diag[6 * t + 1];
        sum2 += A.off_diag[6 * t + 2];
        sum2 += A.off_diag[6 * t + 3];
        sum2 += A.off_diag[6 * t + 4];
        sum2 += A.off_diag[6 * t + 5];
    }

#endif

    return sum1 + sum2;
}