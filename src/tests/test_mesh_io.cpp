#include "test_utils.h"

#include "mesh.h"
#include "mesh_io.h"


/******************************************************************************
 * Test loading a simple triangle.
 *
 * OBJ file:
 *
 *     v 0 0 0
 *     v 1 0 0
 *     v 0 1 0
 *
 *     f 1 2 3
 *
 * The resulting mesh must contain:
 *
 *     - 3 vertices
 *     - 3 indices
 *     - 1 triangle
 *****************************************************************************/


/******************************************************************************
 * Build an absolute path to a file located in the test data directory.
 *
 * TEST_DATA_DIR is defined by CMake as an absolute path to the project's
 * data/ folder, so tests work regardless of the current working directory
 * from which ctest is invoked.
 *****************************************************************************/
static std::string data_path(const char *filename) {
    return std::string(TEST_DATA_DIR) + "/" + filename;
}

static void test_load_triangle() {
    
    Mesh mesh;
    int result = load_obj(data_path("simple_triangle.obj").c_str(), mesh);

    check(result == EXIT_SUCCESS, "Mesh IO: OBJ file loads successfully");
    check(mesh.vertex_count() == 3, "Mesh IO: correct vertex count");
    check(mesh.indices.size == 3, "Mesh IO: correct index count");
    check(mesh.triangle_count() == 1, "Mesh IO: correct triangle count");
}


/******************************************************************************
 * Test vertex positions.
 *
 * The positions stored in the OBJ file must be correctly transferred
 * into the Mesh structure.
 *****************************************************************************/

static void test_vertex_positions() {
    
    Mesh mesh;
    int result = load_obj(data_path("simple_triangle.obj").c_str(), mesh);

    if (result != EXIT_SUCCESS) return;

    check(almost_equal(mesh.positions[0].x, 0.0), "Mesh IO: vertex 0 x");
    check(almost_equal(mesh.positions[0].y, 0.0), "Mesh IO: vertex 0 y");
    check(almost_equal(mesh.positions[1].x, 1.0), "Mesh IO: vertex 1 x");
    check(almost_equal(mesh.positions[1].y, 0.0), "Mesh IO: vertex 1 y");
    check(almost_equal(mesh.positions[2].x, 0.0), "Mesh IO: vertex 2 x");
    check(almost_equal(mesh.positions[2].y, 1.0), "Mesh IO: vertex 2 y");
}


/******************************************************************************
 * Test indices.
 *
 * All indices must reference valid vertices.
 *****************************************************************************/

static void test_indices_are_valid() {
    Mesh mesh;
    int result = load_obj(data_path("simple_triangle.obj").c_str(), mesh);

    if (result != EXIT_SUCCESS)
        return;

    bool valid = true;

    for (size_t i = 0; i < mesh.indices.size; ++i) {

        if (mesh.indices[i] >= mesh.vertex_count()) {
            valid = false;
            break;
        }
    }

    check(valid, "Mesh IO: all indices reference valid vertices");
}


/******************************************************************************
 * Test invalid filename.
 *
 * Loading a non-existent file must fail.
 *****************************************************************************/

static void test_invalid_file() {
    Mesh mesh;
    int result = load_obj("this_file_does_not_exist.obj", mesh);
    check(result == EXIT_FAILURE, "Mesh IO: invalid file returns failure");
}

/******************************************************************************
 * Test quad triangulation.
 *
 * OBJ face:
 *
 *     f 1 2 3 4
 *
 * A quadrilateral must be triangulated into:
 *
 *     2 triangles
 *
 * Therefore:
 *
 *     index count    = 6
 *     triangle count = 2
 *****************************************************************************/

static void test_quad_triangulation() {
    Mesh mesh;

    int result = load_obj(data_path("quad.obj").c_str(), mesh);
    check(result == EXIT_SUCCESS, "Mesh IO: quad OBJ loads successfully");
    check(mesh.vertex_count() == 4, "Mesh IO: quad has four vertices");
    check(mesh.indices.size == 6, "Mesh IO: quad is converted to six indices");
    check(mesh.triangle_count() == 2, "Mesh IO: quad is triangulated into two triangles");
}

int main() {
    LOG_MSG("Mesh IO unit tests\n");
    LOG_MSG("------------------\n");

    test_load_triangle();
    test_vertex_positions();
    test_indices_are_valid();
    test_invalid_file();
    test_quad_triangulation();

    if (!g_all_passed) {

        LOG_MSG("\nTEST FAILED\n");

        return 1;
    }

    LOG_MSG("\nTEST PASSED\n");

    return 0;
}