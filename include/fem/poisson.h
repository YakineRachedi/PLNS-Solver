#pragma once

#include "array.h"

#if USE_FEM_MATRIX
	#include "fem_matrix.h"
#else
	#include "sparse_matrix.h"
#endif

#include "mesh.h"

/******************************************************************************
 * PoissonSolver : Data and routines used to solve a Poisson problem using
 *                 the finite element method.
 *
 * The FEM discretization leads to a linear system of the form
 *
 *                     A u = M f
 *
 * where A is the stiffness matrix, M is the mass matrix, f is the right-hand
 * side and u is the unknown solution vector.
 *
 * The linear system is solved using the Conjugate Gradient (CG) algorithm.
 * The solver stores both the FEM data (mesh, matrices and vectors) and the
 * internal vectors required by the CG iterations.
 *
 * The solver can be advanced by batches of iterations, which makes it
 * possible to monitor the convergence of the solution.
 *****************************************************************************/

struct PoissonSolver {

	/* Constructor */
	PoissonSolver(const Mesh & m);

	/* Reference to the mesh used to build the FEM system.
	 * The mesh is not owned or modified by the solver. */
	const Mesh & m;

	/* Number of degrees of freedom.
	 * For P1 finite elements, this is typically the number of vertices. */
	size_t N;

	/* Area of the domain.
	 * Used when enforcing a zero-mean condition on f and u. */
	double vol;

	/* Right-hand side and solution vectors. */
	TArray<double> f;
	TArray<double> u;

#if USE_FEM_MATRIX

	/* FEM matrices:
	 * A : stiffness matrix
	 * M : mass matrix */
	FEMatrix A;
	FEMatrix M;

#else

	/* Sparse matrix representation.
	 * P contains the common sparsity pattern of the matrices. */
	CSRPattern P;

	/* A : stiffness matrix
	 * M : mass matrix */
	CSRMatrix A;
	CSRMatrix M;

#endif

	/* Internal vectors used by the Conjugate Gradient solver.
	 *
	 * r  : current residual, r = Mf - Au
	 * p  : current CG search direction
	 * Ap : matrix-vector product A * p */
	TArray<double> r;
	TArray<double> p;
	TArray<double> Ap;

	/* Conjugate Gradient solver state.
	 *
	 * inited     : indicates whether the CG solver has been initialized
	 * iterate    : current number of CG iterations
	 * b2         : squared norm of the right-hand side, ||Mf||^2
	 * r2         : squared norm of the current residual, ||r||^2
	 * converged  : indicates whether the solver has converged
	 * rel_error  : current relative residual, ||r|| / ||Mf|| */
	bool inited;
	size_t iterate;
	double b2;
	double r2;
	bool converged;
	double rel_error;

	/* Reset the solution vector and the corresponding solver state. */
	void clear_solution();

	/* Initialize the Conjugate Gradient solver by computing the initial
	 * residual, search direction and error. */
	void init_cg();

	/* Remove the mean value of a vector so that its integral over the
	 * domain is zero. */
	void set_zero_mean(double *V);

	/* Perform Conjugate Gradient iterations until either the requested
	 * number of iterations has been reached or the desired tolerance
	 * has been achieved. */
	void do_iterate(size_t max_iter, double tol);
};