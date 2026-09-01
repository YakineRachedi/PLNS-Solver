#pragma once

#include "aabb.h"
#include "mesh.h"


/******************************************************************************
 * Mesh bounds utilities.
 *
 * This file provides utilities for computing geometric information about a
 * mesh.
 *
 * In particular, compute_mesh_bounds() computes an axis-aligned bounding box
 * (AABB) containing all vertices of a mesh.
 *
 * The resulting bounding box is defined by:
 *
 *     min
 *         Minimum coordinates among all vertices.
 *
 *     max
 *         Maximum coordinates among all vertices.
 *
 * For every mesh vertex p:
 *
 *     min.x <= p.x <= max.x
 *     min.y <= p.y <= max.y
 *     min.z <= p.z <= max.z
 *
 *****************************************************************************/


/******************************************************************************
 * Compute the axis-aligned bounding box of a mesh.
 *
 * The returned AABB contains all vertices of the mesh.
 *
 * The bounds are computed as:
 *
 *     min[j] = min_i(positions[i][j])
 *
 *     max[j] = max_i(positions[i][j])
 *
 * for:
 *
 *     j = 0, 1, 2
 *
 * corresponding respectively to the x, y and z coordinates.
 *
 * If the mesh contains no vertices, the function returns a bounding box
 * reduced to the origin:
 *
 *     min = (0,0,0)
 *     max = (0,0,0)
 *
 *****************************************************************************/

Aabb compute_mesh_bounds(const Mesh & m);