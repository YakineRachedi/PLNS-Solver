#pragma once

#include "mesh.h"

/******************************************************************************
 * Sphere mesh generation.
 *
 * The sphere is generated from a subdivided cube.
 *
 * First, a triangulated cube mesh is created. Each vertex position is then
 * projected onto the unit sphere by normalizing its position vector:
 *
 *     p <- p / ||p||
 *
 * The connectivity of the mesh remains unchanged.
 *****************************************************************************/

/******************************************************************************
 * Generate a triangulated unit sphere mesh.
 *
 * The sphere is obtained by generating a subdivided cube and projecting every
 * vertex onto the unit sphere.
 *
 * @param m
 *     Output mesh.
 *
 * @param subdiv
 *     Number of subdivisions used to generate each cube face before the
 *     projection onto the sphere.
 *
 * @return
 *     0 on success.
 *    -1 if the underlying cube generation fails.
 *****************************************************************************/
int load_sphere(Mesh &m, size_t subdiv);