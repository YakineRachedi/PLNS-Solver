#pragma once

#include "mesh.h"


/******************************************************************************
 * Mesh file loading utilities.
 *
 * This file provides functions used to load meshes from external files.
 *
 * Currently, the supported format is:
 *
 *     Wavefront OBJ (.obj)
 *
 * The loaded geometry is converted into the internal Mesh representation.
 *
 *****************************************************************************/


/******************************************************************************
 * Load a Wavefront OBJ file.
 *
 * Parameters:
 *
 *     filename
 *         Path of the OBJ file to load.
 *
 *     mesh
 *         Destination mesh.
 *
 * On success:
 *
 *     EXIT_SUCCESS
 *
 * is returned.
 *
 * On failure:
 *
 *     EXIT_FAILURE
 *
 * is returned.
 *
 * The loaded mesh contains:
 *
 *     mesh.positions
 *         Unique vertex positions.
 *
 *     mesh.indices
 *         Triangle indices.
 *
 * Polygonal faces containing more than three vertices are triangulated.
 *
 *****************************************************************************/

int load_obj(const char *filename, Mesh & mesh);