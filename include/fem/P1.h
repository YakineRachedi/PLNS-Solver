#pragma once

#include "fem_matrix.h"
#include "sparse_matrix.h"

/******************************************************************************
 *
 * P1 finite element matrix assembly
 *
 * This file provides functions used to assemble the global finite element
 * matrices associated with a triangular mesh and P1 finite elements.
 *
 * Two matrix storage formats are supported:
 *
 *     - CSRMatrix
 *         A generic sparse matrix representation using the Compressed Sparse
 *         Row (CSR) format.
 *
 *     - FEMatrix
 *         A specialized matrix representation that directly exploits the
 *         connectivity of the finite element mesh.
 *
 *****************************************************************************/


/******************************************************************************
 * CSR matrix variants
 *
 * The CSR representation requires two separate steps:
 *
 *     1. Build the sparsity pattern of the matrix.
 *     2. Assemble the numerical coefficients into this pattern.
 *
 *****************************************************************************/

/*
 * Build the sparsity pattern of a P1 finite element matrix.
 *
 * The pattern contains the row and column indices of all matrix entries that
 * can receive contributions during the finite element assembly.
 *
 * Since the mass and stiffness matrices have the same connectivity pattern,
 * the same CSRPattern can be reused for both matrices.
 */
void build_P1_CSRPattern(const Mesh & m, CSRPattern & P);

/*
 * Assemble the global P1 mass matrix using a previously constructed
 * CSR sparsity pattern.
 *
 * The resulting matrix M represents the finite element mass operator.
 */
void build_P1_mass_matrix(const Mesh & m, const CSRPattern & P, CSRMatrix & M);

/*
 * Assemble the global P1 stiffness matrix using a previously constructed
 * CSR sparsity pattern.
 *
 * The resulting matrix S represents the finite element stiffness operator.
 */
void build_P1_stiffness_matrix(const Mesh & m, const CSRPattern & P, CSRMatrix & S);


/******************************************************************************
 * FEMatrix variants
 *
 * These functions assemble the same finite element operators using the
 * specialized FEMatrix storage format.
 *
 * Unlike the CSR representation, FEMatrix directly stores the coefficients
 * associated with mesh elements and uses the mesh connectivity during
 * matrix-vector products.
 *
 *****************************************************************************/

/*
 * Assemble the global P1 mass matrix using the specialized FEMatrix format.
 */
void build_P1_mass_matrix(const Mesh & m, FEMatrix & M);

/*
 * Assemble the global P1 stiffness matrix using the specialized FEMatrix
 * format.
 */
void build_P1_stiffness_matrix(const Mesh & m, FEMatrix & S);