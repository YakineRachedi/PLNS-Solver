#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "cube.h"
#include "duplicate_verts.h"
#include "math_utils.h"
#include "mesh.h"
#include "sys_utils.h"
#include "vec3.h"


/******************************************************************************
 * Generate the vertices of the six cube faces.
 *
 * Each face is initially treated as an independent n x n grid, where:
 *
 *     n = subdiv + 1
 *
 * Therefore, vertices shared by two or three faces are initially duplicated.
 *
 * The duplicated vertices are merged later by remove_duplicate_vertices().
 *
 * @param pos
 *     Pre-allocated array receiving the generated vertex positions.
 *
 * @param subdiv
 *     Number of subdivisions along each edge of a cube face.
 *****************************************************************************/
static void load_cube_vertices(Vec3 *pos, size_t subdiv) {
	size_t n = subdiv + 1;

	/*
	 * Each of the six faces contains n * n vertices.
	 *
	 * voff[f] stores the current write position for face f.
	 */
	size_t voff[6];
	for (int f = 0; f < 6; ++f) {
		voff[f] = f * POW2(n);
	}

	/*
	 * Generate coordinates regularly distributed between -1 and +1.
	 *
	 * For example, for subdiv = 2:
	 *
	 *     dir = {-1, 0, +1}
	 *
	 * rev contains the same coordinates in reverse order and is used to
	 * preserve a consistent orientation of the different cube faces.
	 */
	float *dir = (float *)safe_malloc(n * sizeof(float));
	float *rev = (float *)safe_malloc(n * sizeof(float));

	for (size_t i = 0; i < n; ++i) {
		dir[i] = (2 * (float)i - subdiv) / subdiv;
	}

	for (size_t i = 0; i < n; ++i) {
		rev[i] = dir[subdiv - i];
	}

	/*
	 * Generate the six faces independently.
	 *
	 * Every face is represented by an n x n regular grid.
	 *
	 * Since the faces are generated independently, two faces sharing an edge
	 * initially contain separate vertices with identical coordinates.
	 */
	for (size_t y = 0; y < n; ++y) {
		for (size_t x = 0; x < n; ++x) {

			pos[voff[0]++] = { dir[x], -1, dir[y] }; /* Front  */
			pos[voff[1]++] = { rev[x], +1, dir[y] }; /* Back   */
			pos[voff[2]++] = { -1, rev[x], dir[y] }; /* Left   */
			pos[voff[3]++] = { +1, dir[x], dir[y] }; /* Right  */
			pos[voff[4]++] = { dir[x], rev[y], -1 }; /* Bottom */
			pos[voff[5]++] = { dir[x], dir[y], +1 }; /* Top    */
		}
	}

	free(dir);
	free(rev);
}


/******************************************************************************
 * Generate triangle connectivity for the six subdivided cube faces.
 *
 * Each face contains:
 *
 *     subdiv * subdiv
 *
 * quadrilateral cells.
 *
 * Each quadrilateral cell:
 *
 *     base -------- base + 1
 *       |          / |
 *       |        /   |
 *       |      /     |
 *       |    /       |
 *       |  /         |
 *       base + n ----+
 *
 * is split into two triangles:
 *
 *     (base, base + 1,     base + 1 + n)
 *     (base, base + 1 + n, base + n)
 *
 * @param idx
 *     Pre-allocated array receiving the triangle indices.
 *
 * @param subdiv
 *     Number of subdivisions along each face edge.
 *****************************************************************************/

static void load_cube_indices(uint32_t *idx, size_t subdiv) {
	size_t n = subdiv + 1;

	/*
	 * Generate connectivity independently for every cube face.
	 */
	for (int f = 0; f < 6; ++f) {

		/* First vertex index of the current face. */
		size_t offset = f * POW2(n);

		/*
		 * Iterate over the subdiv x subdiv quadrilateral cells.
		 */
		for (size_t i = 0; i < subdiv; ++i) {
			for (size_t j = 0; j < subdiv; ++j) {

				uint32_t base =
				    (uint32_t)(i * n + j + offset);

				/* First triangle of the cell. */
				*idx++ = base;
				*idx++ = base + 1;
				*idx++ = base + 1 + n;

				/* Second triangle of the cell. */
				*idx++ = base;
				*idx++ = base + 1 + n;
				*idx++ = base + n;
			}
		}
	}
}


/******************************************************************************
 * Generate a triangulated cube mesh.
 *
 * The generation process is:
 *
 *     1. Generate the six faces independently.
 *
 *     2. Generate the triangle connectivity of every face.
 *
 *     3. Merge vertices with identical positions along shared edges and
 *        corners.
 *
 * @param m
 *     Output mesh.
 *
 * @param subdiv
 *     Number of subdivisions along each cube face edge.
 *
 * @return
 *     0 on success.
 *    -1 if subdiv is outside the supported range.
 *****************************************************************************/

int load_cube(Mesh & m, size_t subdiv) {
	/*
	 * Reject invalid or excessively large meshes.
	 */
	if (subdiv <= 0 || subdiv > (1 << 14) /* 16K */) {
		return -1;
	}

	size_t n = subdiv + 1;

	/*
	 * Initially, each of the six faces contains n^2 independent vertices.
	 *
	 * Some of these vertices will later be merged because they represent the
	 * same positions along cube edges and corners.
	 */
	m.positions.resize(6 * POW2(n));

	/*
	 * There are:
	 *
	 *     6 faces
	 *     subdiv^2 cells per face
	 *     2 triangles per cell
	 *     3 indices per triangle
	 *
	 * Therefore:
	 *
	 *     6 * subdiv^2 * 2 * 3 = 36 * subdiv^2
	 *
	 * indices are required.
	 */
	m.indices.resize(36 * POW2(subdiv));

	/* Generate the vertex positions. */
	load_cube_vertices(m.positions.data, subdiv);

	/* Generate the triangle connectivity. */
	load_cube_indices(m.indices.data, subdiv);

	/*
	 * Merge identical vertices.
	 *
	 * The faces were generated independently, so vertices on common edges
	 * and corners currently exist multiple times.
	 */
	remove_duplicate_vertices(m);

	return 0;
}