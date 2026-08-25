#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "navier_stokes.h"

#include "P1.h"
#include "tiny_blas.h"


/******************************************************************************
 *
 * NavierStokesSolver implementation
 *
 * This solver implements a two-dimensional incompressible Navier-Stokes
 * formulation using:
 *
 *     - vorticity omega;
 *     - stream function psi.
 *
 * The velocity field does not appear explicitly. Instead, the solver uses:
 *
 *     Delta(psi) = omega
 *
 * to recover the stream function from the vorticity, then advances omega in
 * time according to the vorticity transport equation:
 *
 *     d(omega)/dt + u . grad(omega) = nu * Delta(omega)
 *
 * where the velocity is obtained from the stream function:
 *
 *     u = (d(psi)/dy, -d(psi)/dx)
 *
 * The spatial discretization uses P1 finite elements on a triangular mesh.
 *
 *****************************************************************************/


/******************************************************************************
 * Construct the Navier-Stokes solver.
 *
 * The constructor:
 *
 *     1. Stores the mesh and determines the number of degrees of freedom.
 *     2. Allocates the vectors required by the solver.
 *     3. Builds the common CSR sparsity pattern.
 *     4. Assembles the P1 finite element mass matrix M.
 *     5. Assembles the P1 finite element stiffness matrix S.
 *     6. Computes the total domain measure using the mass matrix.
 *     7. Initializes the simulation time.
 *
 *****************************************************************************/
NavierStokesSolver::NavierStokesSolver(const Mesh &m) : m(m), N(m.vertex_count()), omega(N), 
                                                                Momega(N), psi(N), r(N), p(N), Ap(N) {
	/*
	 * Build the common sparse pattern used by the finite element
	 * mass and stiffness matrices.
	 */
	build_P1_CSRPattern(m, P);

	/* Assemble the finite element mass matrix. */
	build_P1_mass_matrix(m, P, M);

	/* Assemble the finite element stiffness matrix. */
	build_P1_stiffness_matrix(m, P, S);

	/*
	 * Compute the total surface/volume represented by the mesh.
	 *
	 * For a constant vector equal to one:
	 *
	 *     1^T M 1 = vol
	 *
	 * This value is used to enforce the zero-mean condition.
	 */
	vol = M.sum();

	/* Initialize the solver state. */
	inited = false;
	t = 0;
}


/******************************************************************************
 * Enforce the zero-mean condition on a discrete vector.
 *
 * The mean value is computed using the finite element mass matrix.
 *
 * For a vector V, its discrete integral is:
 *
 *     integral(V) ~= 1^T M V
 *
 * The corresponding mean is therefore:
 *
 *     mean(V) = integral(V) / vol
 *
 * The vector is corrected as:
 *
 *     V <- V - mean(V)
 *
 * This is particularly important for variables associated with operators
 * having a constant null space.
 *
 *****************************************************************************/
void NavierStokesSolver::set_zero_mean(double *V) {
	/*
	 * Compute:
	 *
	 *     Ap = M * V
	 */
	M.mvp(V, Ap.data);

	/*
	 * Compute the discrete integral:
	 *
	 *     s = 1^T * M * V
	 */
	double s = blas_sum_in_place(Ap.data, N);

	/*
	 * Remove the mean value from every degree of freedom:
	 *
	 *     V_i <- V_i - s / vol
	 */
	for (size_t i = 0; i < N; ++i) {
		V[i] -= s / vol;
	}
}


/******************************************************************************
 * Compute the nonlinear transport term.
 *
 * The continuous vorticity equation contains the nonlinear term:
 *
 *     u . grad(omega)
 *
 * With the stream function representation:
 *
 *     u = (d(psi)/dy, -d(psi)/dx)
 *
 * the transport term can be written using the Jacobian:
 *
 *     J(psi, omega)
 *
 * where:
 *
 *     J(psi, omega)
 *         = d(psi)/dx * d(omega)/dy
 *           - d(psi)/dy * d(omega)/dx
 *
 * This function assembles a discrete finite element approximation of this
 * nonlinear transport term.
 *
 * The result is stored in:
 *
 *     T
 *
 * Each triangle contributes to the three vertices defining the element.
 *
 *****************************************************************************/
void NavierStokesSolver::compute_transport(double *T) {
	/* Initialize the transport vector to zero. */
	memset(T, 0, N * sizeof(double));

	/*
	 * Assemble the transport contribution triangle by triangle.
	 */
	for (size_t t = 0; t < m.triangle_count(); ++t) {

		/* Get the three vertices of the current triangle. */
		uint32_t a = m.indices[3 * t + 0];
		uint32_t b = m.indices[3 * t + 1];
		uint32_t c = m.indices[3 * t + 2];

		/* Check that all vertex indices are valid. */
		assert(a < N && b < N && c < N);

		/*
		 * Compute the sum of the nodal vorticity values on the
		 * current P1 element.
		 */
		double sum = omega[a] + omega[b] + omega[c];

		/*
		 * Add the local transport contributions.
		 *
		 * The cyclic differences:
		 *
		 *     psi[b] - psi[c]
		 *     psi[c] - psi[a]
		 *     psi[a] - psi[b]
		 *
		 * represent the discrete antisymmetric structure of the
		 * nonlinear transport operator.
		 */
		T[a] += sum * (psi[b] - psi[c]);
		T[b] += sum * (psi[c] - psi[a]);
		T[c] += sum * (psi[a] - psi[b]);
	}

	/*
	 * Apply the normalization factor of the local finite element
	 * formulation.
	 */
	for (size_t v = 0; v < N; ++v) {
		T[v] *= 1.0 / 6;
	}
}


/******************************************************************************
 * Compute the stream function from the current vorticity.
 *
 * The continuous relation between stream function and vorticity is:
 *
 *     Delta(psi) = omega
 *
 * In weak finite element form, this leads to:
 *
 *     S * psi = M * omega
 *
 * where:
 *
 *     S
 *         Stiffness matrix associated with the Laplacian.
 *
 *     M
 *         Mass matrix.
 *
 * The resulting linear system is solved using the Conjugate Gradient method.
 *
 * Returns:
 *
 *     Number of Conjugate Gradient iterations performed.
 *
 *****************************************************************************/
size_t NavierStokesSolver::compute_stream_function() {
	double b2, r2, rel_error;
	size_t iter;

	double *R   = r.data;
	double *P   = p.data;
	double *AP  = Ap.data;
	double *Om  = omega.data;
	double *MOm = Momega.data;
	double *Psi = psi.data;

	/*
	 * Form the right-hand side:
	 *
	 *     b = M * omega
	 */
	M.mvp(Om, MOm);

	/*
	 * Compute:
	 *
	 *     b2 = ||b||^2
	 *
	 * This value is used to compute the relative residual.
	 */
	b2 = blas_dot(MOm, MOm, N);

	/*
	 * Compute the initial matrix-vector product:
	 *
	 *     R = S * psi
	 */
	S.mvp(Psi, R);

	/*
	 * Form the initial residual:
	 *
	 *     R = b - S * psi
	 *
	 *     R <- M * omega - R
	 */
	blas_axpby(1, MOm, -1, R, N);

	/*
	 * Initialize the first Conjugate Gradient search direction:
	 *
	 *     P_0 = R_0
	 */
	blas_copy(R, P, N);

	/* Compute the squared residual norm. */
	r2 = blas_dot(R, R, N);

	/* Compute the initial relative residual. */
	rel_error = sqrt(r2 / b2);

	/*
	 * Iterate until the requested tolerance is reached or the maximum
	 * number of iterations is exceeded.
	 */
	iter = 0;
	while ((rel_error > tol) && (iter++ < iter_max)) {

		/*
		 * Compute:
		 *
		 *     AP = S * P
		 */
		S.mvp(P, AP);

		/*
		 * Compute the Conjugate Gradient step:
		 *
		 *               R^T R
		 *     alpha = --------
		 *               P^T A P
		 *
		 * Here A = S.
		 */
		double alpha = r2 / blas_dot(P, AP, N);

		/*
		 * Update the solution:
		 *
		 *     psi <- psi + alpha * P
		 */
		blas_axpy(alpha, P, Psi, N);

		/*
		 * Update the residual:
		 *
		 *     R <- R - alpha * AP
		 */
		blas_axpy(-alpha, AP, R, N);

		/*
		 * Save the inverse of the previous squared residual norm
		 * for the beta coefficient.
		 */
		double beta = 1.0 / r2;

		/* Compute the new squared residual norm. */
		r2 = blas_dot(R, R, N);

		/* Update the relative residual. */
		rel_error = sqrt(r2 / b2);

		/*
		 * Complete the computation:
		 *
		 *     beta = ||R_new||^2 / ||R_old||^2
		 */
		beta *= r2;

		/*
		 * Update the search direction:
		 *
		 *     P <- R + beta * P
		 */
		blas_axpby(1, R, beta, P, N);
	}

	return iter;
}


/******************************************************************************
 * Advance the Navier-Stokes solution by one time step.
 *
 * The algorithm performs the following operations:
 *
 *     1. Solve for the stream function psi from the current vorticity omega.
 *
 *     2. Compute the nonlinear transport term T(omega, psi).
 *
 *     3. Advance the vorticity using an implicit treatment of diffusion:
 *
 *         (M + nu * dt * S) * omega^(n+1)
 *             = M * omega^n + dt * T(omega^n, psi^n)
 *
 *     4. Enforce the zero-mean condition on the new vorticity.
 *
 *     5. Advance the simulation time:
 *
 *         t <- t + dt
 *
 *****************************************************************************/
void NavierStokesSolver::time_step(double dt, double nu) {
	double b2, r2, rel_error;

	size_t iter1, iter2;

	double *R   = r.data;
	double *P   = p.data;
	double *AP  = Ap.data;
	double *Om  = omega.data;
	double *MOm = Momega.data;

	/*
	 * Step 1:
	 *
	 * Solve:
	 *
	 *     S * psi = M * omega
	 *
	 * to recover the stream function associated with the current
	 * vorticity field.
	 */
	iter1 = compute_stream_function();


	/**********************************************************************
	 *
	 * Step 2:
	 *
	 * Solve the semi-implicit vorticity update:
	 *
	 *     (M + nu * dt * S) * omega^(n+1)
	 *         = M * omega^n + dt * T(omega^n, psi^n)
	 *
	 * The nonlinear transport term is evaluated explicitly at time n,
	 * while the diffusion term is treated implicitly.
	 *
	 *********************************************************************/

	/*
	 * Compute:
	 *
	 *     P = T(omega^n, psi^n)
	 */
	compute_transport(P);

	/*
	 * Add the previous vorticity contribution:
	 *
	 *     P = M * omega^n + dt * T(omega^n, psi^n)
	 *
	 * P now contains the right-hand side of the linear system.
	 */
	blas_axpby(1, MOm, dt, P, N);

	/*
	 * Compute the initial residual for the system:
	 *
	 *     A * omega^(n+1) = b
	 *
	 * with:
	 *
	 *     A = M + dt * nu * S
	 *
	 * and:
	 *
	 *     b = M * omega^n + dt * T
	 *
	 * The current omega is used as the initial guess.
	 */

    /*
    * Compute the squared norm of the right-hand side, used to
    * evaluate the relative residual during the CG iterations.
    */
    b2 = blas_dot(P, P, N);
    
	/*
	 * First compute:
	 *
	 *     R = S * omega
	 */
	S.mvp(Om, R);

	/*
	 * Compute:
	 *
	 *     R = M * omega + dt * nu * S * omega
	 *
	 * MOm already contains M * omega from the previous computation.
	 */
	blas_axpby(1, MOm, dt * nu, R, N);

	/*
	 * Form:
	 *
	 *     R = b - A * omega
	 */
	blas_axpby(1, P, -1, R, N);

	/*
	 * Initialize the first CG search direction:
	 *
	 *     P = R
	 *
	 * The previous right-hand side stored in P is no longer needed.
	 */
	blas_copy(R, P, N);

	/* Compute the initial squared residual norm. */
	r2 = blas_dot(R, R, N);

	/* Compute the relative residual. */
	rel_error = sqrt(r2 / b2);

	/*
	 * Solve the linear system using the Conjugate Gradient method.
	 *
	 * At least one iteration is performed.
	 */
	iter2 = 0;
	do {

		/*
		 * Compute the system matrix applied to the search direction:
		 *
		 *     AP = (M + dt * nu * S) * P
		 *
		 * First:
		 *
		 *     AP = S * P
		 */
		S.mvp(P, AP);

		/*
		 * Compute:
		 *
		 *     MOm = M * P
		 *
		 * Momega is reused here as temporary storage.
		 */
		M.mvp(P, MOm);

		/*
		 * Combine both contributions:
		 *
		 *     AP = M * P + dt * nu * S * P
		 */
		blas_axpby(1, MOm, dt * nu, AP, N);

		/*
		 * Compute the CG step:
		 *
		 *                R^T R
		 *     alpha = ------------
		 *              P^T AP
		 */
		double alpha = r2 / blas_dot(P, AP, N);

		/*
		 * Update the vorticity:
		 *
		 *     omega <- omega + alpha * P
		 */
		blas_axpy(alpha, P, Om, N);

		/*
		 * Update the residual:
		 *
		 *     R <- R - alpha * AP
		 */
		blas_axpy(-alpha, AP, R, N);

		/*
		 * Compute:
		 *
		 *     beta = ||R_new||^2 / ||R_old||^2
		 */
		double beta = 1.0 / r2;
		r2 = blas_dot(R, R, N);
		rel_error = sqrt(r2 / b2);
		beta *= r2;

		/*
		 * Update the CG search direction:
		 *
		 *     P <- R + beta * P
		 */
		blas_axpby(1, R, beta, P, N);

		/*
		 * Recompute:
		 *
		 *     Momega = M * omega
		 *
		 * This is required for the next time step.
		 */
		M.mvp(Om, MOm);

		iter2++;

	} while ((rel_error > tol) && (iter2 <= iter_max));

	/*
	 * Remove the mean value from the updated vorticity.
	 */
	set_zero_mean(omega.data);

	/* Advance the physical simulation time. */
	t += dt;

	/* Iteration counts are currently not exposed. */
	(void)iter1;
	(void)iter2;

	//printf("Iter 1 : %zu, Iter2 : %zu\n", iter1, iter2);
}