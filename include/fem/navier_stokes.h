#pragma once

#include "array.h"

#if USE_FEM_MATRIX
	#include "fem_matrix.h"
#else
	#include "sparse_matrix.h"
#endif

#include "mesh.h"

/******************************************************************************
 * NavierStokesSolver : Data and routines used to solve the two-dimensional
 *                       Navier-Stokes equations using the finite element
 *                       method.
 *
 * The solver uses a stream-function / vorticity formulation. The main
 * variables are:
 *
 *   - omega : vorticity field
 *   - psi   : stream function
 *
 * The FEM discretization leads to linear systems involving the stiffness
 * matrix S and the mass matrix M. These matrices are assembled from the
 * underlying mesh and are used to compute the stream function and the
 * transport terms.
 *
 * The time-dependent problem is advanced using successive time steps.
 * The solver stores the vectors and temporary data required by the numerical
 * computations, as well as the parameters controlling the time integration
 * and convergence.
 *****************************************************************************/

struct NavierStokesSolver {

	/* Constructor */
	NavierStokesSolver(const Mesh & m);

	/* Reference to the mesh used to build the FEM system.
	 * The mesh is not owned or modified by the solver. */
	const Mesh & m;

	/* Number of degrees of freedom.
	 * For P1 finite elements, this is typically the number of vertices. */
	size_t N;

	/* Area of the domain.
	 * Used when enforcing a zero-mean condition on omega and psi. */
	double vol;

	/* Vorticity field.
	 * omega contains the current discrete vorticity values. */
	TArray<double> omega;

	/* Mass-matrix weighted vorticity.
	 * Stores the result of the mass matrix applied to omega. */
	TArray<double> Momega;

	/* Stream function.
	 * psi contains the current discrete stream-function values. */
	TArray<double> psi;

#if USE_FEM_MATRIX

	/* FEM matrices:
	 * S : stiffness matrix
	 * M : mass matrix */
	FEMatrix S;
	FEMatrix M;

#else

	/* Sparse matrix representation.
	 * P stores the common sparsity pattern of the matrices. */
	CSRPattern P;

	/* S : stiffness matrix
	 * M : mass matrix */
	CSRMatrix S;
	CSRMatrix M;

#endif

	/* Internal vectors used by the iterative linear solver.
	 *
	 * r  : current residual
	 * p  : current search direction
	 * Ap : matrix-vector product A * p */
	TArray<double> r;
	TArray<double> p;
	TArray<double> Ap;

	/* Indicates whether the iterative solver has been initialized. */
	bool inited;

	/* Maximum number of iterations used by the iterative solver. */
	size_t iter_max = 500;

	/* Convergence tolerance used by the iterative solver. */
	double tol = 1e-6;

	/* Current simulation time. */
	double t;

	/* Remove the mean value of a vector so that its integral over the
	 * domain is zero. */
	void set_zero_mean(double *V);

	/* Compute the stream function from the current vorticity field. */
	size_t compute_stream_function();

	/* Compute the transport term associated with the vorticity field. */
	void compute_transport(double *T);

	/* Advance the solution by one time step of size dt using viscosity nu. */
	void time_step(double dt, double nu);
};