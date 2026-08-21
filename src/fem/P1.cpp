#include <stdio.h>
#include <string.h>

#ifdef USE_OPENMP
#include <omp.h>
#endif

#include "P1.h"
#include "adjacency.h"
#include "fem_matrix.h"
#include "mass.h"
#include "mesh.h"
#include "sparse_matrix.h"
#include "stiffness.h"

/* CSRMatrix variants */

/*
 * Check whether a vertex index is already present in the current row.
 *
 * This is used while building the CSR sparsity pattern to avoid storing
 * the same matrix entry more than once.
 */
static bool find(uint32_t x, uint32_t *start, size_t count) {
	for (size_t i = 0; i < count; ++i) {
		if (start[i] == x)
			return true;
	}
	return false;
}

/******************************************************************************
 * Build the sparsity pattern of P1 finite element matrices in CSR format.
 *
 * Each row corresponds to a mesh vertex.
 *
 * For P1 finite elements, two vertices are connected in the global matrix
 * when they belong to the same triangle.
 *
 * Since the matrices assembled here are symmetric, only the lower triangular
 * part and the diagonal are stored.
 *
 * The resulting pattern can then be shared by several matrices, such as the
 * mass and stiffness matrices.
 *****************************************************************************/
void build_P1_CSRPattern(const Mesh & m, CSRPattern & P) {
	size_t vtx_count = m.vertex_count();
	size_t tri_count = m.triangle_count();

	/* The assembled P1 matrices are square and symmetric */
	P.symmetric = true;
	P.rows = P.cols = vtx_count;

	/*
	 * row_start[a] gives the beginning of row a in the CSR column array.
	 * One additional entry is required to mark the end of the last row.
	 */
	P.row_start.resize(vtx_count + 1);

	/*
	 * Build vertex-to-triangle adjacency information.
	 *
	 * This allows us to find all triangles connected to a given vertex.
	 */
	VTAdjacency adj(m);

	/*
	 * Allocate an upper bound for the number of stored matrix entries.
	 *
	 * Each triangle contributes at most three off-diagonal connections,
	 * while each vertex contributes its diagonal entry.
	 */
	size_t max_nnz = 3 * tri_count + vtx_count;
	P.col.resize(max_nnz);

	/*
	 * Build the CSR structure row by row.
	 *
	 * For each vertex a, collect vertices connected to a through a triangle.
	 * Only vertices b < a are stored, together with the diagonal entry a,
	 * because the matrix is symmetric.
	 */
	size_t nnz = 0;

	for (size_t a = 0; a < vtx_count; ++a) {
		P.row_start[a] = nnz;

		uint32_t *start = &P.col[nnz];
		size_t nnz_loc = 0;

		/* Range of triangles adjacent to vertex a */
		uint32_t kstart = adj.offset[a];
		uint32_t kstop = kstart + adj.degree[a];

		for (size_t k = kstart; k < kstop; ++k) {
			uint32_t b = adj.vtri[k].next;
			uint32_t c = adj.vtri[k].prev;

			/*
			 * Add connected vertices to the current row.
			 * The find() test prevents duplicate entries.
			 */
			if (b < a && !find(b, start, nnz_loc)) {
				P.col[nnz++] = b;
				nnz_loc++;
			}

			if (c < a && !find(c, start, nnz_loc)) {
				P.col[nnz++] = c;
				nnz_loc++;
			}
		}

		/* Store the diagonal entry */
		P.col[nnz++] = a;
	}

	/* End position of the last CSR row */
	P.row_start[vtx_count] = nnz;

	/* Remove unused capacity */
	P.col.resize(nnz);
	P.col.shrink_to_fit();

	/*
	 * Sort column indices inside each row.
	 *
	 * Vertex degrees are typically small, so insertion sort is sufficient.
	 */
	for (size_t a = 0; a < vtx_count; ++a) {
		uint32_t *__restrict to_sort = &P.col[P.row_start[a]];
		size_t count = P.row_start[a + 1] - P.row_start[a];

		for (size_t k = 1; k < count; ++k) {
			size_t j = k;

			while (j && to_sort[j - 1] > to_sort[j]) {
				uint32_t tmp = to_sort[j - 1];
				to_sort[j - 1] = to_sort[j];
				to_sort[j] = tmp;
				j--;
			}
		}
	}
}
/******************************************************************************
 * Assemble the global P1 mass matrix in CSR format.
 *
 * The CSR sparsity pattern must already have been built.
 *
 * For each triangle:
 *
 *     1. Compute the local mass matrix.
 *     2. Add its contributions to the corresponding entries of the global
 *        matrix.
 *
 * The global matrix is assembled by summing the contributions of all
 * triangles sharing the same vertices.
 *****************************************************************************/
void build_P1_mass_matrix(const Mesh & m, const CSRPattern & P, CSRMatrix & M) {
	size_t vtx_count = m.vertex_count();
	size_t tri_count = m.triangle_count();

	/* The pattern must contain one CSR row for each mesh vertex */
	assert(P.row_start.size == vtx_count + 1);

	/* Initialize the global matrix using the previously built pattern */
	M.symmetric = true;
	M.rows = M.cols = vtx_count;
	M.nnz = P.col.size;
	M.row_start = P.row_start.data;
	M.col = P.col.data;

	/* Allocate and initialize all matrix coefficients */
	M.data.resize(M.nnz);
	for (size_t i = 0; i < M.nnz; ++i) {
		M.data[i] = 0.0;
	}

	const TArray<uint32_t> &idx = m.indices;

	/*
	 * Assemble the global matrix triangle by triangle.
	 */
	for (size_t t = 0; t < tri_count; ++t) {
		/* Global indices of the three triangle vertices */
		uint32_t a = idx[3 * t + 0];
		uint32_t b = idx[3 * t + 1];
		uint32_t c = idx[3 * t + 2];

		/* Triangle geometry */
		Vec3f A = m.positions[a];
		Vec3f B = m.positions[b];
		Vec3f C = m.positions[c];

		/* Edge vectors used to compute the local element matrix */
		Vec3d AB = {
			(double)B[0] - (double)A[0],
			(double)B[1] - (double)A[1],
			(double)B[2] - (double)A[2]
		};

		Vec3d AC = {
			(double)C[0] - (double)A[0],
			(double)C[1] - (double)A[1],
			(double)C[2] - (double)A[2]
		};

		/*
		 * Compute the local P1 mass matrix.
		 *
		 * Due to symmetry, only two different coefficients are required:
		 *
		 *     Mloc[0] : diagonal coefficient
		 *     Mloc[1] : off-diagonal coefficient
		 */
		double Mloc[2];
		mass(AB, AC, Mloc);

		/*
		 * Assemble local diagonal contributions into the global matrix.
		 */
		M(a, a) += Mloc[0];
		M(b, b) += Mloc[0];
		M(c, c) += Mloc[0];

		/*
		 * Assemble the three off-diagonal vertex connections.
		 *
		 * Only the lower triangular part is stored because M is symmetric.
		 */
		M(a > b ? a : b, a > b ? b : a) += Mloc[1];
		M(b > c ? b : c, b > c ? c : b) += Mloc[1];
		M(c > a ? c : a, c > a ? a : c) += Mloc[1];
	}
}

/******************************************************************************
 * Assemble the global P1 stiffness matrix in CSR format.
 *
 * The CSR sparsity pattern must already have been built.
 *
 * Each triangle contributes a local stiffness matrix whose coefficients
 * depend on the triangle geometry.
 *
 * The local contributions are accumulated into the corresponding entries
 * of the global stiffness matrix.
 *****************************************************************************/
void build_P1_stiffness_matrix(const Mesh & m, const CSRPattern & P, CSRMatrix & S) {
	size_t vtx_count = m.vertex_count();
	size_t tri_count = m.triangle_count();

	assert(P.row_start.size == vtx_count + 1);

	/* Initialize the global matrix using the shared CSR pattern */
	S.symmetric = true;
	S.rows = S.cols = vtx_count;
	S.nnz = P.col.size;
	S.row_start = P.row_start.data;
	S.col = P.col.data;

	S.data.resize(S.nnz);
	for (size_t i = 0; i < S.nnz; ++i) {
		S.data[i] = 0.0;
	}

	const TArray<uint32_t> &idx = m.indices;

	/*
	 * Assemble the stiffness matrix by accumulating contributions from
	 * every triangle.
	 */
	for (size_t t = 0; t < tri_count; ++t) {
		/* Global indices of the triangle vertices */
		uint32_t a = idx[3 * t + 0];
		uint32_t b = idx[3 * t + 1];
		uint32_t c = idx[3 * t + 2];

		Vec3f A = m.positions[a];
		Vec3f B = m.positions[b];
		Vec3f C = m.positions[c];

		/* Triangle edge vectors */
		Vec3d AB = {
			(double)B[0] - (double)A[0],
			(double)B[1] - (double)A[1],
			(double)B[2] - (double)A[2]
		};

		Vec3d AC = {
			(double)C[0] - (double)A[0],
			(double)C[1] - (double)A[1],
			(double)C[2] - (double)A[2]
		};

		/*
		 * Compute the local P1 stiffness matrix.
		 *
		 * The symmetric 3x3 local matrix contains:
		 *
		 *     3 diagonal coefficients
		 *     3 off-diagonal coefficients
		 */
		double Sloc[6];
		stiffness(AB, AC, Sloc);

		/* Assemble diagonal contributions */
		S(a, a) += Sloc[0];
		S(b, b) += Sloc[1];
		S(c, c) += Sloc[2];

		/*
		 * Assemble off-diagonal contributions.
		 *
		 * Only one triangular part is stored in CSR format.
		 */
		S(a > b ? a : b, a > b ? b : a) += Sloc[3];
		S(b > c ? b : c, b > c ? c : b) += Sloc[4];
		S(c > a ? c : a, c > a ? a : c) += Sloc[5];
	}
}

/* FEMatrix variants */
/******************************************************************************
 * Build the P1 mass matrix using the FEM-specific matrix representation.
 *
 * Instead of assembling a global sparse matrix explicitly, the coefficients
 * are stored directly according to the mesh connectivity.
 *
 * The matrix-vector product is later computed by iterating over the mesh
 * vertices and triangles.
 *****************************************************************************/
void build_P1_mass_matrix(const Mesh & m, FEMatrix & M) {
	size_t vtx_count = m.vertex_count();
	size_t tri_count = m.triangle_count();

	/*
	 * For a P1 mass matrix, all off-diagonal coefficients of one triangle
	 * are identical.
	 */
	M.fem_type = FEMatrix::P1_cst;

	/* Keep a reference to the mesh connectivity */
	M.m = &m;
	M.rows = M.cols = vtx_count;

	/* One accumulated diagonal coefficient per vertex */
	M.diag.resize(vtx_count);
	memset(M.diag.data, 0, vtx_count * sizeof(double));

	/* One off-diagonal coefficient per triangle */
	M.off_diag.resize(tri_count);

	const TArray<uint32_t> &idx = m.indices;

	for (size_t t = 0; t < tri_count; ++t) {
		uint32_t a = idx[3 * t + 0];
		uint32_t b = idx[3 * t + 1];
		uint32_t c = idx[3 * t + 2];

		Vec3f A = m.positions[a];
		Vec3f B = m.positions[b];
		Vec3f C = m.positions[c];

		Vec3d AB = {
			(double)B[0] - (double)A[0],
			(double)B[1] - (double)A[1],
			(double)B[2] - (double)A[2]
		};

		Vec3d AC = {
			(double)C[0] - (double)A[0],
			(double)C[1] - (double)A[1],
			(double)C[2] - (double)A[2]
		};

		/* Compute the local mass matrix coefficients */
		double Mloc[2];
		mass(AB, AC, Mloc);

		/*
		 * Accumulate the diagonal contribution at each triangle vertex.
		 */
		M.diag[a] += Mloc[0];
		M.diag[b] += Mloc[0];
		M.diag[c] += Mloc[0];

		/*
		 * Store the common off-diagonal coefficient of this triangle.
		 */
		M.off_diag[t] = Mloc[1];
	}
}

/******************************************************************************
 * Build the P1 stiffness matrix using the FEM-specific matrix representation.
 *
 * The global matrix is not explicitly stored.
 *
 * For each triangle, its three diagonal contributions are accumulated at
 * the corresponding vertices, while its three symmetric off-diagonal
 * coefficients are stored directly per element.
 *****************************************************************************/
void build_P1_stiffness_matrix(const Mesh & m, FEMatrix & S) {
	size_t vtx_count = m.vertex_count();
	size_t tri_count = m.triangle_count();

	/*
	 * The P1 stiffness matrix has three different symmetric off-diagonal
	 * coefficients per triangle.
	 */
	S.fem_type = FEMatrix::P1_sym;

	/* Keep the mesh used to reconstruct matrix-vector interactions */
	S.m = &m;
	S.rows = S.cols = vtx_count;

	/* One accumulated diagonal coefficient per vertex */
	S.diag.resize(vtx_count);
	memset(S.diag.data, 0, vtx_count * sizeof(double));

	/* Three symmetric off-diagonal coefficients per triangle */
	S.off_diag.resize(3 * tri_count);

	const TArray<uint32_t> &idx = m.indices;

	for (size_t t = 0; t < tri_count; ++t) {
		uint32_t a = idx[3 * t + 0];
		uint32_t b = idx[3 * t + 1];
		uint32_t c = idx[3 * t + 2];

		Vec3f A = m.positions[a];
		Vec3f B = m.positions[b];
		Vec3f C = m.positions[c];

		Vec3d AB = {
			(double)B[0] - (double)A[0],
			(double)B[1] - (double)A[1],
			(double)B[2] - (double)A[2]
		};

		Vec3d AC = {
			(double)C[0] - (double)A[0],
			(double)C[1] - (double)A[1],
			(double)C[2] - (double)A[2]
		};

		/* Compute the six independent local stiffness coefficients */
		double Sloc[6];
		stiffness(AB, AC, Sloc);

		/* Accumulate diagonal coefficients */
		S.diag[a] += Sloc[0];
		S.diag[b] += Sloc[1];
		S.diag[c] += Sloc[2];

		/*
		 * Store the three off-diagonal interactions of the triangle:
		 *
		 *     a <-> b
		 *     b <-> c
		 *     c <-> a
		 */
		S.off_diag[3 * t + 0] = Sloc[3];
		S.off_diag[3 * t + 1] = Sloc[4];
		S.off_diag[3 * t + 2] = Sloc[5];
	}
}