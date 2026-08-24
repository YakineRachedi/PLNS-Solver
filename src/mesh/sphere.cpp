#include <stdint.h>
#include <stdio.h>

#include "cube.h"
#include "mesh.h"
#include "vec3.h"


/******************************************************************************
 * Generate a triangulated unit sphere mesh.
 *
 * The sphere is constructed from a subdivided cube:
 *
 *     1. Generate a triangulated cube.
 *
 *     2. Project every vertex onto the unit sphere by normalizing its
 *        position:
 *
 *            p <- p / ||p||
 *
 * The mesh connectivity remains unchanged.
 *
 * @param m
 *     Output mesh.
 *
 * @param subdiv
 *     Number of subdivisions used for each cube face.
 *
 * @return
 *     0 on success.
 *    -1 if the cube mesh generation fails.
 *****************************************************************************/

int load_sphere(Mesh & m, size_t subdiv) {
	/*
	 * First generate the cube mesh.
	 */
	if (int res = load_cube(m, subdiv))
		return res;

	Vec3 *pos = m.positions.data;
	size_t vtx_count = m.positions.size;

	/*
	 * Project every cube vertex onto the unit sphere.
	 *
	 * The normalized vector has unit length:
	 *
	 *     ||normalized(p)|| = 1
	 */
	for (size_t i = 0; i < vtx_count; ++i) {
		pos[i] = normalized(pos[i]);
	}

	return 0;
}