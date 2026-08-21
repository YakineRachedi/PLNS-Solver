#include "poisson.h"

#include "P1.h"
#include "array.h"
#include "conjugate_gradient.h"

#if USE_FEM_MATRIX
	#include "fem_matrix.h"
#else
	#include "sparse_matrix.h"
#endif

#include "mesh.h"
#include "tiny_blas.h"


/******************************************************************************
 *
 * PoissonSolver implementation
 *
 * The solver assembles the finite element mass and stiffness matrices from
 * the input mesh and solves the corresponding Poisson problem using the
 * Conjugate Gradient method.
 *
 * The linear system has the form:
 *
 *     A * u = M * f
 *
 * where:
 *
 *     A
 *         Stiffness matrix.
 *
 *     M
 *         Mass matrix.
 *
 *     f
 *         Discrete right-hand side.
 *
 *     u
 *         Discrete solution.
 *
 *****************************************************************************/


/******************************************************************************
 * Construct the Poisson solver.
 *
 * The constructor:
 *
 *     1. Stores the mesh and determines the number of degrees of freedom.
 *     2. Allocates the vectors required by the solver and the CG algorithm.
 *     3. Builds the finite element mass and stiffness matrices.
 *     4. Computes the total surface/volume associated with the mass matrix.
 *     5. Initializes the solver state.
 *
 *****************************************************************************/
PoissonSolver::PoissonSolver(const Mesh &m) : m(m), N(m.vertex_count()), f(N), u(N, 0.0), r(N), p(N), Ap(N) {
#if USE_FEM_MATRIX

	/*
	 * Assemble the finite element matrices using the specialized
	 * FEMatrix representation.
	 */
	build_P1_mass_matrix(m, M);
	build_P1_stiffness_matrix(m, A);

#else

	/*
	 * Build the common CSR sparsity pattern and use it to assemble
	 * the mass and stiffness matrices.
	 */
	build_P1_CSRPattern(m, P);
	build_P1_mass_matrix(m, P, M);
	build_P1_stiffness_matrix(m, P, A);

#endif

	/*
	 * Compute the total surface/volume represented by the mesh.
	 *
	 * This value is used when enforcing the zero-mean condition.
	 */
	vol = M.sum();

	/* Initialize the iterative solver state. */
	inited = false;
	iterate = 0;
	converged = false;
}


/******************************************************************************
 * Reset the current solution.
 *
 * The solution vector is set to zero and the Conjugate Gradient state is
 * reinitialized.
 *
 *****************************************************************************/
void PoissonSolver::clear_solution() {
	/* Reset the solution vector. */
	for (size_t i = 0; i < N; ++i) {
		u[i] = 0.0;
	}

	/* Recompute the initial CG state. */
	init_cg();

	iterate = 0;
	converged = false;
}


/******************************************************************************
 * Enforce a zero-mean condition on a discrete vector.
 *
 * The Poisson problem considered here requires the right-hand side and the
 * solution to satisfy a zero-mean condition.
 *
 * The mean is computed using the mass matrix:
 *
 *     s = sum(M * V)
 *
 * and the vector is corrected as:
 *
 *     V <- V - s / vol
 *
 *****************************************************************************/
void PoissonSolver::set_zero_mean(double *V) {
	/* Compute M * V. */
	M.mvp(V, Ap.data);

	/* Compute the integral/sum associated with V. */
	double s = blas_sum_in_place(Ap.data, N);

	/* Remove the mean value. */
	for (size_t i = 0; i < N; ++i) {
		V[i] -= s / vol;
	}
}


/******************************************************************************
 * Initialize the Conjugate Gradient algorithm.
 *
 * The linear system solved by the Poisson solver is:
 *
 *     A * u = M * f
 *
 * Therefore, the CG right-hand side is:
 *
 *     b = M * f
 *
 * The initial residual is:
 *
 *     r_0 = M * f - A * u_0
 *
 * The first search direction is initialized as:
 *
 *     p_0 = r_0
 *
 *****************************************************************************/
void PoissonSolver::init_cg() {
	double *F  = f.data;
	double *U  = u.data;
	double *R  = r.data;
	double *P  = p.data;
	double *AP = Ap.data;

	/*
	 * Enforce the zero-mean condition on the right-hand side and on
	 * the initial solution.
	 */
	set_zero_mean(F);
	set_zero_mean(U);

	/*
	 * Compute the CG right-hand side:
	 *
	 *     b = M * f
	 *
	 * R is temporarily used to store M * f.
	 */
	M.mvp(F, R);

	/* Compute ||M * f||^2 for the relative residual. */
	b2 = blas_dot(R, R, N);

	/*
	 * Compute:
	 *
	 *     AP = A * u
	 */
	A.mvp(U, AP);

	/*
	 * Compute the initial residual:
	 *
	 *     r_0 = M * f - A * u_0
	 */
	blas_axpy(-1, AP, R, N);

	/*
	 * Initialize the first CG search direction:
	 *
	 *     p_0 = r_0
	 */
	blas_copy(R, P, N);

	/* Compute ||r_0||^2. */
	r2 = blas_dot(R, R, N);

	/* Compute the initial relative residual. */
	rel_error = sqrt(r2 / b2);

	inited = true;
}


/******************************************************************************
 * Perform Conjugate Gradient iterations.
 *
 * The solver performs at most max_iter iterations and stops earlier when the
 * relative residual becomes smaller than the requested tolerance.
 *
 *****************************************************************************/
void PoissonSolver::do_iterate(size_t max_iter, double tol) {
	/* Initialize the CG state if this is the first call. */
	if (!inited) {
		init_cg();
	}

	double *U  = u.data;
	double *R  = r.data;
	double *P  = p.data;
	double *AP = Ap.data;

	/*
	 * Perform CG iterations until either:
	 *
	 *     - the maximum number of iterations is reached;
	 *     - the requested convergence tolerance is satisfied.
	 */
	while (max_iter-- && rel_error > tol) {

		/*
		 * Perform one Conjugate Gradient iteration and update:
		 *
		 *     u, r, p and Ap
		 */
		r2 = cg_iterate_once(A, U, R, P, AP, r2);

		iterate++;

		/* Update the relative residual. */
		rel_error = sqrt(r2 / b2);
	}

	/* Record convergence when the requested tolerance is reached. */
	if (rel_error <= tol) {
		converged = true;
	}
}