#pragma once

#include <stdint.h>

#include "array.h"
#include "vec3.h"


/******************************************************************************
 * Mesh : 3D triangular mesh.
 *
 * A Mesh represents a geometric mesh made of vertices and triangles.
 *
 * The geometry is described using:
 *
 *   - positions : 3D coordinates of the vertices;
 *   - indices   : indices of the vertices forming each triangle;
 *   - attr      : additional floating-point attributes associated with
 *                the mesh.
 *
 * The mesh uses indexed triangles. Each triangle is represented by three
 * consecutive entries in the 'indices' array.
 *
 * Example:
 *
 *     positions:
 *
 *         0 -> (0, 0, 0)
 *         1 -> (1, 0, 0)
 *         2 -> (1, 1, 0)
 *         3 -> (0, 1, 0)
 *
 *     indices:
 *
 *         [0, 1, 2,  0, 2, 3]
 *
 *     This represents two triangles:
 *
 *         triangle 0 -> vertices 0, 1, 2
 *         triangle 1 -> vertices 0, 2, 3
 *
 * Using indices allows several triangles to share the same vertex instead
 * of storing the vertex coordinates multiple times.
 *
 *****************************************************************************/

struct Mesh {

    /*
     * 3D position of each vertex.
     *
     * positions[i] contains the coordinates of vertex i.
     */
    TArray<Vec3> positions;

    /*
     * Vertex indices defining the triangles.
     *
     * Every three consecutive indices define one triangle:
     *
     *     indices[3*k + 0]
     *     indices[3*k + 1]
     *     indices[3*k + 2]
     *
     * Each index refers to an entry in 'positions'.
     */
    TArray<uint32_t> indices;

    /*
     * Additional floating-point attributes associated with the mesh.
     *
     * The exact meaning of these attributes depends on how the Mesh
     * structure is used by the rest of the application.
     */
    TArray<float> attr;

    /*
     * Return the number of vertices in the mesh.
     *
     * There is one position for each vertex.
     */
    size_t vertex_count() const {
        return positions.size;
}

    /*
     * Return the total number of indices stored in the mesh.
     */
    size_t index_count() const {
        return indices.size;
    }

    /*
     * Return the number of triangles in the mesh.
     *
     * Each triangle is represented by exactly three indices.
     */
    size_t triangle_count() const {
        return indices.size / 3;
    }
};