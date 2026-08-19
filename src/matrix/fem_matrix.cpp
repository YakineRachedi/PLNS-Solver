#include "fem_matrix.h"


/******************************************************************************
 *
 * Matrix-vector product implementations for the different FEM storage formats.
 *
 *****************************************************************************/

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


/******************************************************************************
 *
 * Matrix coefficient sum implementations for the different FEM formats.
 *
 *****************************************************************************/

static double sum_P1_cst(const FEMatrix & A);
static double sum_P1_sym(const FEMatrix & A);
static double sum_P1_gen(const FEMatrix & A);


/******************************************************************************
 *
 * Return the sum of all coefficients of the matrix.
 *
 * The actual computation depends on the storage format used by the matrix.
 *
 *****************************************************************************/

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

static void mvp_P1_cst(const FEMatrix & A, const double *x, double *y) {
	assert(A.fem_type == FEMatrix::P1_cst);

	size_t vtx_count = A.m->vertex_count();
	size_t tri_count = A.m->triangle_count();

	const TArray<uint32_t> &idx = A.m->indices;

	/* Initialize the result with the diagonal contribution. */
	for (size_t v = 0; v < vtx_count; ++v) {
		y[v] = A.diag[v] * x[v];
	}

	/* Add the off-diagonal contributions of every triangle. */
	for (size_t t = 0; t < tri_count; ++t) {

		uint32_t a = idx[3 * t + 0];
		uint32_t b = idx[3 * t + 1];
		uint32_t c = idx[3 * t + 2];

		/* One common coefficient for all off-diagonal interactions. */
		double mult = A.off_diag[t];

		y[a] += mult * x[b];
		y[b] += mult * x[a];

		y[b] += mult * x[c];
		y[c] += mult * x[b];

		y[c] += mult * x[a];
		y[a] += mult * x[c];
	}
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

static void mvp_P1_sym(const FEMatrix & A, const double *x, double *y) {
	assert(A.fem_type == FEMatrix::P1_sym);

	size_t vtx_count = A.m->vertex_count();
	size_t tri_count = A.m->triangle_count();

	const TArray<uint32_t> &idx = A.m->indices;

	/* Initialize the result with the diagonal contribution. */
	for (size_t v = 0; v < vtx_count; ++v) {
		y[v] = A.diag[v] * x[v];
	}

	/* Add the symmetric off-diagonal contributions. */
	for (size_t t = 0; t < tri_count; ++t) {

		uint32_t a = idx[3 * t + 0];
		uint32_t b = idx[3 * t + 1];
		uint32_t c = idx[3 * t + 2];

		/* Edge (a,b). */
		y[a] += A.off_diag[3 * t + 0] * x[b];
		y[b] += A.off_diag[3 * t + 0] * x[a];

		/* Edge (b,c). */
		y[b] += A.off_diag[3 * t + 1] * x[c];
		y[c] += A.off_diag[3 * t + 1] * x[b];

		/* Edge (c,a). */
		y[c] += A.off_diag[3 * t + 2] * x[a];
		y[a] += A.off_diag[3 * t + 2] * x[c];
	}
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

static void mvp_P1_gen(const FEMatrix & A, const double *x, double *y) {
	assert(A.fem_type == FEMatrix::P1_gen);

	size_t vtx_count = A.m->vertex_count();
	size_t tri_count = A.m->triangle_count();

	const TArray<uint32_t> &idx = A.m->indices;

	/* Initialize the result with the diagonal contribution. */
	for (size_t v = 0; v < vtx_count; ++v) {
		y[v] = A.diag[v] * x[v];
	}

	/* Add the six independent off-diagonal contributions. */
	for (size_t t = 0; t < tri_count; ++t) {

		uint32_t a = idx[3 * t + 0];
		uint32_t b = idx[3 * t + 1];
		uint32_t c = idx[3 * t + 2];

		y[a] += A.off_diag[6 * t + 0] * x[b];
		y[b] += A.off_diag[6 * t + 1] * x[a];

		y[b] += A.off_diag[6 * t + 2] * x[c];
		y[c] += A.off_diag[6 * t + 3] * x[b];

		y[c] += A.off_diag[6 * t + 4] * x[a];
		y[a] += A.off_diag[6 * t + 5] * x[c];
	}
}


/******************************************************************************
 *
 * Return the sum of all coefficients of a P1_cst matrix.
 *
 * The diagonal coefficients are stored once per vertex.
 *
 * Each triangle stores one off-diagonal coefficient which represents the six
 * off-diagonal interactions between its three vertices. Its contribution to
 * the total matrix sum is therefore:
 *
 *     6 * off_diag[t]
 *
 *****************************************************************************/

static double sum_P1_cst(const FEMatrix & A) {
	assert(A.fem_type == FEMatrix::P1_cst);

	size_t vtx_count = A.m->vertex_count();
	size_t tri_count = A.m->triangle_count();

	double sum1 = 0.0;

	/* Sum the diagonal coefficients. */
	for (size_t v = 0; v < vtx_count; ++v) {
		sum1 += A.diag[v];
	}

	double sum2 = 0.0;

	/* Sum the six off-diagonal contributions of every triangle. */
	for (size_t t = 0; t < tri_count; ++t) {
		sum2 += 6.0 * A.off_diag[t];
	}

	return sum1 + sum2;
}


/******************************************************************************
 *
 * Return the sum of all coefficients of a symmetric P1 matrix.
 *
 * Each triangle stores three off-diagonal coefficients. Since the matrix is
 * symmetric, each coefficient appears twice in the full matrix:
 *
 *     A(i,j) and A(j,i)
 *
 *****************************************************************************/

static double sum_P1_sym(const FEMatrix & A) {
	assert(A.fem_type == FEMatrix::P1_sym);

	size_t vtx_count = A.m->vertex_count();
	size_t tri_count = A.m->triangle_count();

	double sum1 = 0.0;

	/* Sum the diagonal coefficients. */
	for (size_t v = 0; v < vtx_count; ++v) {
		sum1 += A.diag[v];
	}

	double sum2 = 0.0;

	/* Each symmetric off-diagonal coefficient contributes twice. */
	for (size_t t = 0; t < tri_count; ++t) {
		sum2 += 2.0 * A.off_diag[3 * t + 0];
		sum2 += 2.0 * A.off_diag[3 * t + 1];
		sum2 += 2.0 * A.off_diag[3 * t + 2];
	}

	return sum1 + sum2;
}


/******************************************************************************
 *
 * Return the sum of all coefficients of a general P1 matrix.
 *
 * The six off-diagonal coefficients of every triangle are independent and
 * are therefore summed individually.
 *
 *****************************************************************************/

static double sum_P1_gen(const FEMatrix & A) {
	assert(A.fem_type == FEMatrix::P1_gen);

	size_t vtx_count = A.m->vertex_count();
	size_t tri_count = A.m->triangle_count();

	double sum1 = 0.0;

	/* Sum the diagonal coefficients. */
	for (size_t v = 0; v < vtx_count; ++v) {
		sum1 += A.diag[v];
	}

	double sum2 = 0.0;

	/* Sum the six independent off-diagonal coefficients. */
	for (size_t t = 0; t < tri_count; ++t) {
		sum2 += A.off_diag[6 * t + 0];
		sum2 += A.off_diag[6 * t + 1];
		sum2 += A.off_diag[6 * t + 2];
		sum2 += A.off_diag[6 * t + 3];
		sum2 += A.off_diag[6 * t + 4];
		sum2 += A.off_diag[6 * t + 5];
	}

	return sum1 + sum2;
}