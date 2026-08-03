#pragma once

#include "vec3.h"

/* Given a triangle ABC, computes the (symmetric) 3x3 mass M s.t.
 *
 *   M_{ij} := \int_{ABC} \phi_i \phi_j
 *
 * where \phi_0 := \phi_A, \phi_1 := \phi_B, \phi_2 := \phi_C
 * are the shape functions of the P1 Lagrange element associated
 * to ABC.
 *
 * Idea behind computation :
 * -------------------------
 *
 * We denote by \Psi the affine map
 *
 *    \Psi(s,t) = sB + tC + (1-s-t)A.
 *
 * Then \Psi maps the reference simplex in R^2 (we denote it by A'B'C')
 * onto ABC, and since \Psi is affine \phi_X = Psi \circ \phi_X' for
 * any X in {A, B, C}. Moreover by the change of variable formula, for
 * arbitrary X, Y in {A, B, C} :
 *
 *    \int_{ABC} \phi_X \phi_Y = \int_{A'B'C'} \phi_X' \phi_Y' |Jac(\Psi)|dsdt
 *
 * where the Jacobian |Jac(\Psi)| is constant equal to |ABC|/|A'B'C'| = 2|ABC|.
 *
 * Besides, elementary integration shows that
 *
 *               (2  1  1)
 * M' = (1/24) * (1  2  1)
 *               (1  1  2)
 *
 * We therefore only return |ABC|/6 and |ABC|/12, with |ABC| = |AB x AC| / 2.
 * 
 */
void inline mass(const Vec3d & AB, const Vec3d & AC, double *__restrict M) {
	M[0] = norm(cross(AB, AC)) / 12;
	M[1] = M[0] / 2;
}


/*
    * Détails :
     * The local mass matrix is defined by * * M_ij = ∫_K φ_i φ_j dx. 
     * On the reference triangle, the mass matrix is 
     * M_hat = 1/24 ( 2 1 1 ; 1 2 1 ; 1 1 2 ). 
     * For a physical triangle K, the change of variables introduces 
     * the constant Jacobian factor |det J| = 2|K|. 
     * Therefore, 
     * M = 2|K| M_hat * = |K|/12 ( 2 1 1 ; 1 2 1 ; 1 1 2 ). 
     * Hence, 
     * M = ( |K|/6 |K|/12 |K|/12 ; |K|/12 |K|/6 |K|/12 ; |K|/12 |K|/12 |K|/6 ). 
     * Due to the symmetry of the matrix, we only return the two distinct coefficients: 
     * |K|/6 for the diagonal entries and |K|/12 for the off-diagonal entries. */
