#include "mesh_bounds.h"

#include "aabb.h"
#include "mesh.h"
#include "vec3.h"


/******************************************************************************
 * Compute the axis-aligned bounding box of a mesh.
 *
 * The bounding box is defined by two vertices:
 *
 *     min = (xmin, ymin, zmin)
 *
 *     max = (xmax, ymax, zmax)
 *
 * where:
 *
 *     xmin = minimum x coordinate among all mesh vertices
 *     ymin = minimum y coordinate among all mesh vertices
 *     zmin = minimum z coordinate among all mesh vertices
 *
 * and:
 *
 *     xmax = maximum x coordinate among all mesh vertices
 *     ymax = maximum y coordinate among all mesh vertices
 *     zmax = maximum z coordinate among all mesh vertices
 *
 * The resulting box contains every mesh vertex.
 *
 *****************************************************************************/

Aabb compute_mesh_bounds(const Mesh & m) {
    /*
     * Handle the empty mesh case.
     *
     * A mesh with no vertices has no meaningful geometric extent.
     * We therefore return a degenerate bounding box located at the origin:
     *
     *     min = max = (0,0,0)
     */
    if (m.positions.size == 0) {
        return { Vec3::Zero, Vec3::Zero };
    }

    // Access the mesh vertices.
    const Vec3 *positions = m.positions.data;
    size_t vertex_count = m.positions.size;


    /*
     * Initialize the bounding box using the first vertex.
     *
     * Using the first vertex avoids having to initialize the bounds with
     * arbitrary large values such as:
     *
     *     +infinity
     *     -infinity
     */
    Vec3 min = positions[0];
    Vec3 max = positions[0];

    // Process all remaining vertices.

    for (size_t i = 1; i < vertex_count; ++i) {

        const Vec3 & pos = positions[i];


        /*
         * Update each coordinate independently.
         *
         * j = 0 : x coordinate
         * j = 1 : y coordinate
         * j = 2 : z coordinate
         */
        for (size_t j = 0; j < 3; ++j) {

            // Update the minimum coordinate
            min[j] = (pos[j] < min[j]) ? pos[j] : min[j];

            // Update the maximum coordinate.
            max[j] = (pos[j] > max[j]) ? pos[j] : max[j];
        }
    }

    // Return the axis-aligned bounding box.
    return { min, max };
}