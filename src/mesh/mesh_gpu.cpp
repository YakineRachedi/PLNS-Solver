#include <assert.h>

#include "mesh.h"
#include "mesh_gpu.h"


/******************************************************************************
 * Upload a mesh to GPU memory.
 *
 * The CPU mesh contains:
 *
 *     m->positions
 *         Vertex positions.
 *
 *     m->indices
 *         Triangle indices.
 *
 *     m->attr
 *         Optional scalar attributes.
 *
 * The function creates corresponding OpenGL buffers and configures a Vertex
 * Array Object describing the vertex layout.
 *
 *****************************************************************************/

void GPUMesh::upload() {

    // A GPUMesh without a source mesh cannot be uploaded.
    if (!m) return;


    /**************************************************************************
     * Position Buffer.
     *
     * Create a Vertex Buffer Object and upload all vertex positions.
     *
     * The data layout is:
     *
     *     x0 y0 z0
     *     x1 y1 z1
     *     x2 y2 z2
     *     ...
     *
     *************************************************************************/

    glGenBuffers(1, &pos_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, pos_vbo);


    /*
     * Allocate GPU memory and copy the vertex positions.
     *
     * GL_STATIC_DRAW indicates that the position buffer is expected to remain
     * mostly unchanged.
     */
    glBufferData(GL_ARRAY_BUFFER, m->positions.size * sizeof(m->positions[0]), m->positions.data, GL_STATIC_DRAW);

    // Unbind the current array buffer.

    glBindBuffer(GL_ARRAY_BUFFER, 0);


    /**************************************************************************
     * Index Buffer.
     *
     * The index buffer describes the triangles.
     *
     * Example:
     *
     *     0 1 2
     *     0 2 3
     *
     *************************************************************************/

    glGenBuffers(1, &idx_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idx_vbo);

    // Upload all triangle indices

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m->indices.size * sizeof(m->indices[0]), m->indices.data, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    /**************************************************************************
     * Attribute Buffer.
     *
     * The attribute is optional.
     *
     * For example:
     *
     *     m->attr[i] = solution[i]
     *
     * If attributes exist, create a buffer containing one float per vertex.
     *
     *************************************************************************/

    if (m->attr.size) {

        glGenBuffers(1, &attr_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, attr_vbo);
        glBufferData(GL_ARRAY_BUFFER, m->attr.size * sizeof(m->attr[0]), m->attr.data, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }


    /**************************************************************************
     * Vertex Array Object.
     *
     * The VAO stores the configuration of the vertex attributes.
     *************************************************************************/

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    /**************************************************************************
     * Vertex attribute 0: position.
     *
     * The corresponding shader input can be:
     *
     *     layout(location = 0) in vec3 position;
     *
     *************************************************************************/

    glBindBuffer(GL_ARRAY_BUFFER, pos_vbo);
    
    glVertexAttribPointer(
        0,                  /* Attribute location */
        3,                  /* Three components */
        GL_FLOAT,           /* float */
        GL_FALSE,           /* No normalization */
        3 * sizeof(float),  /* Distance between vertices */
        (void *)0           /* Offset */
    );


    glEnableVertexAttribArray(0);


    /**************************************************************************
     * Vertex attribute 1: scalar attribute.
     *
     * The corresponding shader input can be:
     *
     *     layout(location = 1) in float attr;
     *
     *************************************************************************/

    if (m->attr.size) {

        glBindBuffer(GL_ARRAY_BUFFER, attr_vbo);


        glVertexAttribPointer(
            1,                  /* Attribute location */
            1,                  /* One scalar */
            GL_FLOAT,
            GL_FALSE,
            1 * sizeof(float),
            (void *)0
        );


        glEnableVertexAttribArray(1);
    }


    // Associate the index buffer with the VAO.
    // The element array buffer binding is stored inside the VAO state.

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idx_vbo);

    // Unbind the VAO.
    glBindVertexArray(0);

    // Unbind temporary buffers.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Verify that no OpenGL error occurred.
    assert(!glGetError());
}


/******************************************************************************
 * Draw the GPU mesh.
 *
 * The mesh is rendered using indexed triangles:
 *
 *     GL_TRIANGLES
 *
 * The indices are stored as uint32_t:
 *
 *     GL_UNSIGNED_INT
 *
 *****************************************************************************/

void GPUMesh::draw() const {
    /*
     * Activate the VAO.
     *
     * This automatically restores:
     *
     *     - vertex position attribute;
     *     - scalar attribute;
     *     - index buffer.
     */
    glBindVertexArray(vao);


    /*
     * Draw all mesh triangles.
     * The number of elements is equal to the number of indices.
     */
    glDrawElements(GL_TRIANGLES, m->indices.size, GL_UNSIGNED_INT, (void *)0);

    // Unbind the VAO.
    glBindVertexArray(0);

    // Verify that no OpenGL error occurred.
    assert(!glGetError());
}


/******************************************************************************
 * Update the scalar attribute buffer.
 *
 * This function is used when:
 *
 *     m->attr
 *
 * has changed after the initial upload.
 *
 * For example, during the Poisson solver:
 *
 *     solver.u
 *
 * changes after Conjugate Gradient iterations.
 *
 * The new values are copied into:
 *
 *     mesh.attr
 *
 * and this function transfers the updated values to the existing GPU buffer.
 *
 *****************************************************************************/

void GPUMesh::update_attr() {

    // Bind the attribute buffer.
    glBindBuffer(GL_ARRAY_BUFFER, attr_vbo);


    /*
     * First allocate/reallocate the buffer.
     *
     * Passing NULL means that no initial data is copied.
     */
    glBufferData(GL_ARRAY_BUFFER, m->attr.size * sizeof(m->attr[0]), NULL, GL_STATIC_DRAW);

    // Upload the updated attribute values.
    glBufferData(GL_ARRAY_BUFFER, m->attr.size * sizeof(m->attr[0]), m->attr.data, GL_STATIC_DRAW);

    // Verify that no OpenGL error occurred.
    assert(!glGetError());
}