#include "test_utils.h"

#include "mesh.h"
#include "mesh_bounds.h"


/******************************************************************************
 * Test an empty mesh.
 *
 * An empty mesh must return:
 *
 *     min = (0, 0, 0)
 *     max = (0, 0, 0)
 *****************************************************************************/

static void test_empty_mesh() {
    Mesh mesh;

    Aabb bounds = compute_mesh_bounds(mesh);
    check(almost_equal(bounds.min.x, 0.0), "Mesh bounds: empty mesh min.x");
    check(almost_equal(bounds.min.y, 0.0), "Mesh bounds: empty mesh min.y");
    check(almost_equal(bounds.min.z, 0.0), "Mesh bounds: empty mesh min.z");
    check(almost_equal(bounds.max.x, 0.0), "Mesh bounds: empty mesh max.x");
    check(almost_equal(bounds.max.y, 0.0), "Mesh bounds: empty mesh max.y");
    check(almost_equal(bounds.max.z, 0.0), "Mesh bounds: empty mesh max.z");
}


/******************************************************************************
 * Test a single vertex.
 *
 * For a mesh containing only one vertex:
 *
 *     min = max = vertex position
 *****************************************************************************/

static void test_single_vertex() {
    Mesh mesh;

    mesh.positions.resize(1);
    mesh.positions[0] = Vec3(1.0f, 2.0f, 3.0f);

    Aabb bounds = compute_mesh_bounds(mesh);
    check(almost_equal(bounds.min.x, 1.0), "Mesh bounds: single vertex min.x");
    check(almost_equal(bounds.min.y, 2.0), "Mesh bounds: single vertex min.y");
    check(almost_equal(bounds.min.z, 3.0), "Mesh bounds: single vertex min.z");
    check(almost_equal(bounds.max.x, 1.0), "Mesh bounds: single vertex max.x");
    check(almost_equal(bounds.max.y, 2.0), "Mesh bounds: single vertex max.y");
    check(almost_equal(bounds.max.z, 3.0), "Mesh bounds: single vertex max.z");
}


/******************************************************************************
 * Test several vertices.
 *
 * The bounding box must contain:
 *
 *     min = component-wise minimum
 *
 *     max = component-wise maximum
 *****************************************************************************/

static void test_multiple_vertices() {
    Mesh mesh;

    mesh.positions.resize(4);

    mesh.positions[0] = Vec3( 1.0f,  2.0f,  3.0f);
    mesh.positions[1] = Vec3(-2.0f,  5.0f,  1.0f);
    mesh.positions[2] = Vec3( 4.0f, -1.0f,  7.0f);
    mesh.positions[3] = Vec3( 0.0f,  3.0f, -5.0f);

    Aabb bounds = compute_mesh_bounds(mesh);
    check(almost_equal(bounds.min.x, -2.0), "Mesh bounds: multiple vertices min.x");
    check(almost_equal(bounds.min.y, -1.0), "Mesh bounds: multiple vertices min.y");
    check(almost_equal(bounds.min.z, -5.0), "Mesh bounds: multiple vertices min.z");
    check(almost_equal(bounds.max.x, 4.0), "Mesh bounds: multiple vertices max.x");
    check(almost_equal(bounds.max.y, 5.0), "Mesh bounds: multiple vertices max.y");
    check(almost_equal(bounds.max.z, 7.0), "Mesh bounds: multiple vertices max.z");
}


int main() {
    LOG_MSG("Mesh bounds unit tests\n");
    LOG_MSG("----------------------\n");

    test_empty_mesh();
    test_single_vertex();
    test_multiple_vertices();

    if (!g_all_passed) {
        LOG_MSG("\nTEST FAILED\n");
        return 1;
    }

    LOG_MSG("\nTEST PASSED\n");

    return 0;
}