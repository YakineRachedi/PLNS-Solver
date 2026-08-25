#pragma once

#include "mesh.h"

/******************************************************************************
 * test_meshes.h : Shared mesh fixtures for unit test executables.
 *
 * Each test_*.cpp file is compiled into its own standalone executable, so
 * including this header in multiple test files does not cause any ODR
 * violation at link time.
 *****************************************************************************/

/******************************************************************************
 * Unit square mesh divided into two triangles.
 *
 *        3 ----- 2
 *        |     / |
 *        |   /   |
 *        | /     |
 *        0 ----- 1
 *
 * positions:
 *
 *     0 = (0,0,0)
 *     1 = (1,0,0)
 *     2 = (1,1,0)
 *     3 = (0,1,0)
 *
 * triangles:
 *
 *     [0,1,2]
 *     [0,2,3]
 *
 * Area of the domain: 1.0
 *****************************************************************************/
inline void setup_unit_square_mesh(Mesh & m) {
    m.positions.resize(4);

    m.positions[0] = {0.0f, 0.0f, 0.0f};
    m.positions[1] = {1.0f, 0.0f, 0.0f};
    m.positions[2] = {1.0f, 1.0f, 0.0f};
    m.positions[3] = {0.0f, 1.0f, 0.0f};

    m.indices.push_back(0);
    m.indices.push_back(1);
    m.indices.push_back(2);

    m.indices.push_back(0);
    m.indices.push_back(2);
    m.indices.push_back(3);
}