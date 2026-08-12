#pragma once

#include <stdint.h>

#include "array.h"
#include "mesh.h"


/******************************************************************************
 * VTAdjacency : Vertex-to-Triangle adjacency structure.
 *
 * The Mesh structure provides the connectivity from triangles to vertices:
 *
 *     triangle -> vertices
 *
 * through the 'indices' array.
 *
 * However, some algorithms also need the reverse information:
 *
 *     vertex -> triangles
 *
 * VTAdjacency stores this reverse connectivity. It is built from an existing
 * Mesh and allows the triangles incident to a given vertex to be accessed
 * efficiently.
 *
 * This information is particularly useful when constructing the sparsity
 * pattern of finite element matrices such as mass and stiffness matrices.
 *
 * For each vertex 'a', the incident triangles are represented by VTri
 * structures. If a triangle is oriented as:
 *
 *     a -> b -> c
 *
 * then the VTri associated with vertex 'a' stores:
 *
 *     next = b
 *     prev = c
 *
 * There is no need to store 'a' itself because the VTri structure is already
 * associated with vertex 'a'.
 *
 * The adjacency information is stored using three arrays:
 *
 *     degree
 *         Number of triangles incident to each vertex.
 *
 *     offset
 *         Starting position of the adjacency information of each vertex
 *         inside the 'vtri' array.
 *
 *     vtri
 *         Array containing the actual vertex-to-triangle adjacency
 *         information.
 *
 * For a vertex 'a', its VTri entries are located at:
 *
 *     offset[a] <= j < offset[a] + degree[a]
 *
 * Since each triangle has three vertices, each triangle contributes exactly
 * three VTri entries. Therefore:
 *
 *     vtri.size = 3 * number_of_triangles
 *
 *****************************************************************************/


/* Vertex-to-Triangle adjacency table */
struct VTAdjacency {

    /**************************************************************************
     * VTri : Adjacency information for one vertex in one triangle.
     *
     * If the triangle is oriented as:
     *
     *     a -> b -> c
     *
     * and this VTri belongs to vertex 'a', then:
     *
     *     next = b
     *     prev = c
     *
     * The current vertex 'a' does not need to be stored because its identity
     * is determined by the position of this VTri in the adjacency structure.
     **************************************************************************/

    struct VTri {
        uint32_t next;
        uint32_t prev;
    };


    /**************************************************************************
     * Number of triangles incident to each vertex.
     *
     * degree[a] = number of triangles containing vertex 'a'.
     *
     * The array has one entry per vertex:
     *
     *     degree.size = mesh.vertex_count()
     **************************************************************************/
    TArray<uint32_t> degree;


    /**************************************************************************
     * Offset of each vertex's adjacency information in the 'vtri' array.
     *
     * offset[a] gives the first position in 'vtri' associated with vertex 'a'.
     *
     * The adjacency entries for vertex 'a' are therefore:
     *
     *     vtri[offset[a]]
     *     ...
     *     vtri[offset[a] + degree[a] - 1]
     *
     * The offsets are computed using a prefix sum of the vertex degrees:
     *
     *     offset[0] = 0
     *
     *     offset[k] = sum(degree[i]) for i = 0 ... k-1
     **************************************************************************/
    TArray<uint32_t> offset;


    /**************************************************************************
     * Vertex-to-triangle adjacency information.
     *
     * The array contains one VTri entry for every vertex of every triangle.
     *
     * Therefore:
     *
     *     vtri.size = 3 * mesh.triangle_count()
     *
     * For a given vertex 'a', its entries are located between:
     *
     *     offset[a]
     *
     * and
     *
     *     offset[a] + degree[a]
     **************************************************************************/
    TArray<VTri> vtri;


    /**************************************************************************
     * Construct the vertex-to-triangle adjacency structure from a mesh.
     *
     * The constructor must analyze the triangle connectivity stored in the
     * Mesh and build the degree, offset and vtri arrays.
     **************************************************************************/
    VTAdjacency(const Mesh & m);
};