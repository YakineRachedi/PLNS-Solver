#ifdef DEBUG
#include <stdio.h>
#endif
#include <string.h>

#define FAST_OBJ_IMPLEMENTATION 1
#include "fast_obj/fast_obj.h"
#undef FAST_OBJ_IMPLEMENTATION

#include "array.h"
#include "hash.h"
#include "hash_table.h"
#include "mesh.h"
#include "mesh_io.h"
#include "vec2.h"
#include "vec3.h"

/******************************************************************************
 * Hashing utility for OBJ vertices.
 *
 * A Wavefront OBJ vertex can reference several independent attributes:
 *
 *     p
 *         Position index.
 *
 *     n
 *         Normal index.
 *
 *     t
 *         Texture coordinate index.
 *
 * Therefore, an OBJ vertex is identified by:
 *
 *     (p,n,t)
 *
 * ObjVertexHasher is used by HashTable to detect whether a vertex has already
 * been inserted into the internal Mesh.
 *
 *****************************************************************************/

struct ObjVertexHasher {


    // Indicates whether normal vectors are used.
    bool has_normals;

    // Indicates whether texture coordinates are used.
    bool has_uv;

    // Arrays containing the OBJ attributes.
    const Vec3 *pos;
    const Vec3 *nml;
    const Vec2 *uv;


    
    // Empty hash table key
    // This is the value used to represent an unused hash table entry.
     
    static constexpr fastObjIndex empty_key = { 0, 0, 0 };

    // Compute the hash of an OBJ vertex.
    size_t hash(fastObjIndex key) const;

    // Test whether a key represents an empty hash table entry.
    bool is_empty(fastObjIndex key) const;

    // Test whether two OBJ vertices represent identical vertices.
    bool is_equal(fastObjIndex key1, fastObjIndex key2) const;
};

// Hash table mapping:     OBJ vertex --- to ---> internal Mesh vertex index

typedef HashTable<fastObjIndex, uint32_t, ObjVertexHasher> ObjVertexTable;
inline size_t ObjVertexHasher::hash(fastObjIndex key) const {
    
    // Access the position associated with the OBJ index.
    const uint32_t *p = reinterpret_cast<const uint32_t *>(pos + key.p);

    // Initialize the hash.
    uint32_t hash = 0;


    /*
     * Hash the three floating-point components:
     *
     *     x
     *     y
     *     z
     */
    hash = murmur2_32(hash, p[0]);
    hash = murmur2_32(hash, p[1]);
    hash = murmur2_32(hash, p[2]);

    return hash;
}

bool ObjVertexHasher::is_empty(fastObjIndex key) const {return (key.p == 0);}

bool ObjVertexHasher::is_equal(fastObjIndex key1, fastObjIndex key2) const {
    // First compare positions
	bool res = (pos[key1.p] == pos[key2.p]);
    // If normals are present compare them as well.
	if (res && has_normals)
		res &= (nml[key1.n] == nml[key2.n]);
    // If UV coordinates are present compare them as well.    
	if (res && has_uv)
		res &= (uv[key1.t] == uv[key2.t]);

	return (res);
}

/******************************************************************************
 * Convert a fastObjMesh into the internal Mesh representation.
 *
 * The conversion performs the following operations:
 *
 *     1. Count the number of triangle indices required.
 *     2. Allocate the index array.
 *     3. Discover unique vertices.
 *     4. Convert polygonal faces into triangles.
 *     5. Build the internal indexed mesh representation.
 *
 *****************************************************************************/

static int load_obj(const fastObjMesh & obj, Mesh & mesh) {
    /**************************************************************************
     * Count the number of indices required.
     *
     * A polygon containing N vertices is triangulated into:
     *
     *     N - 2
     *
     * triangles.
     *
     * Since every triangle has three indices:
     *
     *     index_count = 3 * (N - 2)
     *
     *************************************************************************/

    size_t index_count = 0;


    for (unsigned int i = 0; i < obj.face_count; ++i) index_count += 3 * (obj.face_vertices[i] - 2);

    // Allocate the complete triangle index array.
    mesh.indices.resize(index_count);


    /**************************************************************************
     * OBJ attributes.
     *
     * The current implementation does not import normals or texture
     * coordinates.
     *************************************************************************/

    bool has_normals = false;
    bool has_uv = false;


    /**************************************************************************
     * Estimate the number of unique vertices.
     *
     * This is only used to reserve memory for mesh.positions and the hash
     * table.
     *************************************************************************/

    size_t vertex_count_guess = index_count / 6;
    vertex_count_guess += vertex_count_guess / 2;
    mesh.positions.reserve(vertex_count_guess);

    // Start with an empty position array.
    mesh.positions.resize(0);


    /**************************************************************************
     * Discover unique vertices and build triangle indices.
     *************************************************************************/

    size_t idx = 0;
    // Offset inside obj.indices.
    size_t idx_offset = 0;

    // Number of unique vertices discovered so far.
    size_t vertex_count = 0;


    /**************************************************************************
     * Create the OBJ vertex hasher.
     *************************************************************************/

    ObjVertexHasher hasher{has_normals, has_uv, (const Vec3 *)obj.positions, 
                        (const Vec3 *)obj.normals, (const Vec2 *)obj.texcoords};

    /**************************************************************************
     * Create a hash table:
     *
     *     OBJ vertex -> internal Mesh vertex index
     *
     *************************************************************************/

    ObjVertexTable vertices(vertex_count_guess, hasher);


    /**************************************************************************
     * Process every OBJ face.
     *************************************************************************/

    for (size_t i = 0; i < obj.face_count; ++i){

        // Remember the first index of the current polygon.
        // This value is required for polygon triangulation.
        size_t idx_start = idx;


        /**********************************************************************
         * Process every vertex of the face.
         *********************************************************************/

        for (size_t j = 0; j < obj.face_vertices[i]; ++j) {
            /******************************************************************
             * Triangulate polygonal faces.
             *
             * For example, a quad:
             *
             *     0 ---- 1
             *     |    / |
             *     |  /   |
             *     |/     |
             *     3 ---- 2
             *
             * is converted into:
             *
             *     (0,1,2)
             *
             *     (0,2,3)
             *
             *****************************************************************/

            if (j >= 3) {
                mesh.indices[idx + 0] = mesh.indices[idx_start];
                mesh.indices[idx + 1] = mesh.indices[idx - 1];
                idx += 2;
            }

            // Get the current OBJ vertex.
            fastObjIndex pnt = obj.indices[idx_offset + j];


            /******************************************************************
             * Search for this vertex.
             *
             * If it already exists:
             *
             *     reuse its Mesh index.
             *
             * Otherwise:
             *
             *     create a new Mesh vertex.
             *****************************************************************/

            uint32_t *p = vertices.get_or_set(pnt, vertex_count);


            if (!p) {
                // New vertex.

                mesh.indices[idx] = vertex_count;
                // Copy the position from fast_obj into the internal Mesh.
                {
                    float *v = obj.positions + 3 * pnt.p;
                    Vec3 new_vertex(v[0], v[1], v[2]);
                    mesh.positions.push_back(new_vertex);
                }

                // A new unique vertex has been created.
                vertex_count++;

            } else {

                // The vertex already exists.
                // Reuse its previously assigned Mesh index.
                mesh.indices[idx] = *p;
            }

            // Advance to the next triangle index.
            idx++;
        }

        // Advance to the first OBJ index of the next face.
        idx_offset += obj.face_vertices[i];
    }

    // Verify that the number of created vertices matches the position array.
    assert(vertex_count == mesh.positions.size);

    return EXIT_SUCCESS;
}

/******************************************************************************
 * Load a Wavefront OBJ file.
 *
 * The function performs the following operations:
 *
 *     1. Read the OBJ file using fast_obj.
 *     2. Convert the fastObjMesh into the internal Mesh format.
 *     3. Destroy the temporary fast_obj representation.
 *
 *****************************************************************************/

int load_obj(const char *filename, Mesh & mesh) {
    
    //Read the OBJ file
    fastObjMesh *obj = fast_obj_read(filename);

    //The function returns NULL when the file cannot be loaded.
    if (obj == nullptr) return EXIT_FAILURE;

    // Convert the OBJ representation into the internal Mesh.
    int res = load_obj(*obj, mesh);

    // Release the memory allocated by fast_obj.
    fast_obj_destroy(obj);

    return res;
}