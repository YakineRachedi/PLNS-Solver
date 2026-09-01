#pragma once

#include "gl_utils.h"

#include "mesh.h"


/******************************************************************************
 * GPUMesh : GPU representation of a mesh.
 *
 * This structure stores the OpenGL objects required to transfer a Mesh from
 * CPU memory to GPU memory.
 *
 * The GPU representation contains:
 *
 *   - pos_vbo  : vertex position buffer
 *   - idx_vbo  : triangle index buffer
 *   - attr_vbo : scalar vertex attribute buffer
 *   - vao      : vertex array object describing the vertex layout
 *
 *****************************************************************************/

struct GPUMesh {

    /* Mesh stored in CPU memory. */
    const Mesh *m;

    /* OpenGL vertex buffer containing vertex positions. */
    GLuint pos_vbo;

    /* OpenGL index buffer containing triangle indices. */
    GLuint idx_vbo;

    /* OpenGL vertex buffer containing scalar attributes. */
    GLuint attr_vbo;

    /* OpenGL vertex array object. */
    GLuint vao;

    /* Upload the mesh from CPU memory to GPU memory. */
    void upload();

    /* Update the scalar vertex attributes stored on the GPU. */
    void update_attr();

    /* Draw the mesh. */
    void draw() const;
};