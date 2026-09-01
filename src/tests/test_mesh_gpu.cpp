#include "test_utils.h"
#include "test_meshes.h"
#include "mesh.h"
#include "mesh_gpu.h"


/******************************************************************************
 * Initialize an OpenGL context for the GPU tests.
 *
 * GPUMesh uses OpenGL functions such as:
 *
 *     glGenBuffers()
 *     glBindBuffer()
 *     glBufferData()
 *     glGenVertexArrays()
 *
 * Therefore, a valid OpenGL context must exist before running the tests.
 *
 * An invisible GLFW window is created because no graphical display is needed
 * for these unit tests.
 *****************************************************************************/

static GLFWwindow *create_test_context() {
    
    // Initialize GLFW.
    if (!glfwInit())
        return nullptr;

    // The tests do not need a visible window.
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);


    /*
     * Request a modern OpenGL context.
     *
     * GPUMesh uses:
     *
     *     VBOs
     *     VAOs
     *
     * These features require a modern OpenGL version.
     */

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);


    /*
     * Create a small invisible window.
     * The window is only used to create an OpenGL context.
     */

    GLFWwindow *window = glfwCreateWindow(640, 480, "test_mesh_gpu", nullptr, nullptr);

    if (!window) {
        glfwTerminate();
        return nullptr;
    }


    // Make the OpenGL context current.
    glfwMakeContextCurrent(window);


    /*
     * Initialize the OpenGL functions.
     * init_gl() loads the OpenGL functions needed by the project.
     */

    if (!init_gl()) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return nullptr;
    }


    return window;
}


/******************************************************************************
 * Test GPU upload.
 *
 * After upload():
 *
 *     - a position VBO must exist;
 *     - an index VBO must exist;
 *     - an attribute VBO must exist;
 *     - a VAO must exist.
 *
 * The test uses the unit square mesh:
 *
 *        3 ----- 2
 *        |     / |
 *        |   /   |
 *        | /     |
 *        0 ----- 1
 *
 *****************************************************************************/

static void test_upload() {
    Mesh mesh;


    /*
     * Create the standard mesh used by the unit tests.
     */

    setup_unit_square_mesh(mesh);


    /*
     * Add one scalar attribute for every vertex.
     *
     * GPUMesh::upload() only creates attr_vbo if:
     *
     *     mesh.attr.size != 0
     */

    mesh.attr.resize(mesh.vertex_count());

    for (size_t i = 0; i < mesh.vertex_count(); ++i) {
        mesh.attr[i] = static_cast<float>(i);
    }


    /*
     * Create and upload the GPU mesh.
     */

    GPUMesh gpu_mesh{};

    gpu_mesh.m = &mesh;

    gpu_mesh.upload();


    /*
     * Check that OpenGL objects were created.
     */

    check(
        gpu_mesh.pos_vbo != 0,
        "GPU Mesh: position VBO created"
    );

    check(
        gpu_mesh.idx_vbo != 0,
        "GPU Mesh: index VBO created"
    );

    check(
        gpu_mesh.attr_vbo != 0,
        "GPU Mesh: attribute VBO created"
    );

    check(
        gpu_mesh.vao != 0,
        "GPU Mesh: VAO created"
    );


    /*
     * Check that no OpenGL error occurred.
     */

    check(
        glGetError() == GL_NO_ERROR,
        "GPU Mesh: upload produces no OpenGL error"
    );
}


/******************************************************************************
 * Test attribute update.
 *
 * The mesh attribute values are initially:
 *
 *     attr = {0, 1, 2, 3}
 *
 * They are uploaded to the GPU.
 *
 * The CPU values are then modified:
 *
 *     attr = {10, 20, 30, 40}
 *
 * update_attr() transfers the new values to the GPU.
 *
 * The test verifies that no OpenGL error occurs.
 *****************************************************************************/

static void test_update_attr() {
    
    Mesh mesh;
    setup_unit_square_mesh(mesh);


    /*
     * Allocate one scalar attribute per vertex.
     */

    mesh.attr.resize(mesh.vertex_count());


    /*
     * Initialize the attributes.
     */

    mesh.attr[0] = 0.0f;
    mesh.attr[1] = 1.0f;
    mesh.attr[2] = 2.0f;
    mesh.attr[3] = 3.0f;


    /*
     * Create and upload the GPU mesh.
     */

    GPUMesh gpu_mesh{};

    gpu_mesh.m = &mesh;

    gpu_mesh.upload();


    /*
     * Clear any previous OpenGL errors.
     */

    while (glGetError() != GL_NO_ERROR) {
    }


    /*
     * Modify the attributes on the CPU.
     */

    mesh.attr[0] = 10.0f;
    mesh.attr[1] = 20.0f;
    mesh.attr[2] = 30.0f;
    mesh.attr[3] = 40.0f;


    /*
     * Transfer the new attributes to the GPU.
     */

    gpu_mesh.update_attr();


    /*
     * update_attr() must not generate an OpenGL error.
     */

    check(
        glGetError() == GL_NO_ERROR,
        "GPU Mesh: attribute update produces no OpenGL error"
    );
}


/******************************************************************************
 * Test drawing.
 *
 * GPUMesh::draw() performs:
 *
 *     1. Bind the VAO.
 *     2. Execute glDrawElements().
 *     3. Unbind the VAO.
 *
 * The test verifies that the draw call does not generate an OpenGL error.
 *
 * Note:
 *
 * No visual validation is performed.
 *
 * The goal is only to verify that the OpenGL commands can be executed
 * correctly.
 *****************************************************************************/

static void test_draw() {
    
    Mesh mesh;
    setup_unit_square_mesh(mesh);

    // Add one scalar attribute per vertex.
    mesh.attr.resize(mesh.vertex_count());

    for (size_t i = 0; i < mesh.vertex_count(); ++i) {
        mesh.attr[i] = static_cast<float>(i);
    }

    // Create and upload the GPU mesh.
    GPUMesh gpu_mesh{};
    gpu_mesh.m = &mesh;
    gpu_mesh.upload();


    // Clear any previous OpenGL errors.
    while (glGetError() != GL_NO_ERROR) {
    }


    // Execute the draw call.
    gpu_mesh.draw();


    // Verify that the draw call did not generate an OpenGL error.
    check(
        glGetError() == GL_NO_ERROR,
        "GPU Mesh: draw produces no OpenGL error"
    );
}


/******************************************************************************
 * Main function.
 *
 * A valid OpenGL context must be created before executing the GPU tests.
 *****************************************************************************/

int main() {
    LOG_MSG("GPU Mesh tests\n");
    LOG_MSG("--------------\n");


    // Create an invisible OpenGL context.

    GLFWwindow *window = create_test_context();

    if (!window) {
        LOG_MSG("Failed to create OpenGL context\n");
        return 1;
    }


    // Run GPU mesh tests.

    test_upload();
    test_update_attr();
    test_draw();


    // Destroy the OpenGL context.
    glfwDestroyWindow(window);
    glfwTerminate();

    // Report the test result.

    if (!g_all_passed) {
        LOG_MSG("\nTEST FAILED\n");
        return 1;
    }

    LOG_MSG("\nTEST PASSED\n");

    return 0;
}