#pragma once

#include "array.h"
#include "matrix.h"
#include "mesh.h"

/******************************************************************************
 *
 * FEMatrix : Matrix representation specialized for P1 finite element
 *            discretizations on a triangular mesh.
 *
 * Instead of storing the complete sparse matrix explicitly, FEMatrix stores
 * its coefficients according to the mesh connectivity.
 *
 * The diagonal coefficients are stored once per vertex:
 *
 *     diag[v]
 *
 * The off-diagonal coefficients are stored per triangle. Their number and
 * interpretation depend on the selected FEMType.
 *
 * This representation can reduce memory usage compared with a generic CSR
 * sparse matrix and allows the matrix-vector product to be computed directly
 * by traversing the mesh triangles.
 *
 *****************************************************************************/

struct FEMatrix : public Matrix {

	/******************************************************************************
	 *
	 * FEMType : Storage format used for the off-diagonal coefficients.
	 *
	 * For a P1 finite element discretization, each triangle connects three
	 * vertices. The corresponding local matrix therefore contains interactions
	 * between these vertices.
	 *
	 *****************************************************************************/
	enum FEMType {

		/* One identical off-diagonal coefficient per triangle. */
		P1_cst,

		/* Three symmetric off-diagonal coefficients per triangle:
		 *
		 *     (a,b), (b,c), (c,a)
		 */
		P1_sym,

		/* Six independent off-diagonal coefficients per triangle.
		 *
		 * No symmetry between A(i,j) and A(j,i) is assumed.
		 */
		P1_gen,

		/* Additional finite element matrix formats may be added later. */
	};

	/* Storage format used by this matrix. */
	FEMType fem_type;

	/* Mesh defining the connectivity between the matrix degrees of freedom. */
	const Mesh *m;

	/* Diagonal matrix coefficients, one coefficient per vertex. */
	TArray<double> diag;

	/* Off-diagonal coefficients stored according to fem_type. */
	TArray<double> off_diag;

	/******************************************************************************
	 *
	 * Compute the matrix-vector product:
	 *
	 *     y = A * x
	 *
	 * The implementation is selected according to fem_type.
	 *
	 *****************************************************************************/
	void mvp(const double *__restrict x,
		 double *__restrict y) const override final;

	/******************************************************************************
	 *
	 * Return the sum of all matrix coefficients.
	 *
	 * The implementation is selected according to fem_type.
	 *
	 *****************************************************************************/
	double sum() const override final;
};