#include <stddef.h>
#include <stdint.h>

#include "array.h"
#include "hash.h"
#include "hash_table.h"
#include "mesh.h"
#include "vec3.h"


/******************************************************************************
 * Hash function used to identify vertices from their positions.
 *
 * The hash table stores vertex indices as keys. Two keys are considered equal
 * when the corresponding vertices have identical positions.
 *
 * The hasher therefore:
 *
 *     - hashes the three components of a vertex position;
 *     - compares vertex positions when checking key equality;
 *     - defines a special invalid key used for empty hash table entries.
 *****************************************************************************/
struct PositionHasher {
	const Vec3 *pos;

	/*
	 * Reserved key used by HashTable to identify an empty entry.
	 */
	static constexpr uint32_t empty_key =
	    ~static_cast<uint32_t>(0);


	/******************************************************************************
	 * Compute a hash from the three components of a vertex position.
	 *
	 * The vertex index is first converted into a pointer to its corresponding
	 * Vec3 position. The three coordinate values are then processed by the
	 * MurmurHash function.
	 *
	 * @param key
	 *     Vertex index.
	 *
	 * @return
	 *     Hash value representing the vertex position.
	 *****************************************************************************/
	size_t hash(uint32_t key) const
	{
		uint32_t hash = 0;

		/*
		 * Access the three components of pos[key] as 32-bit values.
		 */
		const uint32_t *p =
		    reinterpret_cast<const uint32_t *>(pos + key);

		/*
		 * Incrementally hash x, y and z.
		 */
		hash = murmur2_32(hash, p[0]);
		hash = murmur2_32(hash, p[1]);
		hash = murmur2_32(hash, p[2]);

		return hash;
	}


	/******************************************************************************
	 * Check whether a key represents an empty hash table entry.
	 *****************************************************************************/
	bool is_empty(uint32_t key) const
	{
		return key == empty_key;
	}


	/******************************************************************************
	 * Compare two vertices by comparing their positions.
	 *
	 * Two different vertex indices are considered equal when they reference
	 * identical coordinates.
	 *****************************************************************************/
	bool is_equal(uint32_t key1, uint32_t key2) const
	{
		return pos[key1] == pos[key2];
	}
};


/******************************************************************************
 * Build a remapping from duplicated vertex indices to unique vertex indices.
 *
 * For every original vertex i:
 *
 *     remap[i] = index of the corresponding unique vertex
 *
 * Example:
 *
 * Original vertices:
 *
 *     index : 0  1  2  3  4
 *     pos   : A  B  A  C  B
 *
 * Result:
 *
 *     remap : 0  1  0  2  1
 *
 * and the number of unique vertices is:
 *
 *     new_count = 3
 *
 * A hash table is used to efficiently determine whether a vertex with the same
 * position has already been encountered.
 *
 * @param pos
 *     Array containing the original vertex positions.
 *
 * @param count
 *     Number of original vertices.
 *
 * @param remap
 *     Output array of size count receiving the vertex remapping.
 *
 * @return
 *     Number of unique vertices.
 *****************************************************************************/
size_t build_position_remap(Vec3 *pos, size_t count, uint32_t *remap) {
	/*
	 * The hasher compares vertices using their positions.
	 */
	PositionHasher hasher{pos};

	/*
	 * Map:
	 *
	 *     original vertex index
	 *             ->
	 *     unique vertex index
	 */
	HashTable<uint32_t, uint32_t, PositionHasher>
	    vtx_remap(count, hasher);

	size_t new_count = 0;

	for (size_t i = 0; i < count; ++i) {

		/*
		 * Look for vertex i in the hash table.
		 *
		 * If an identical position already exists, p points to the
		 * corresponding unique vertex index.
		 *
		 * Otherwise, the vertex is inserted with new_count as its value.
		 */
		uint32_t *p =
		    vtx_remap.get_or_set(i, new_count);

		if (p) {

			/*
			 * A vertex with the same position already exists.
			 */
			remap[i] = *p;

		} else {

			/*
			 * This is a new unique vertex.
			 */
			remap[i] = new_count;
			new_count++;
		}
	}

	return new_count;
}


/******************************************************************************
 * Remove duplicate vertices from a mesh.
 *
 * The operation is performed in three steps:
 *
 *     1. Build a mapping from every original vertex to a unique vertex.
 *
 *     2. Compact the vertex position array.
 *
 *     3. Update all triangle indices using the vertex mapping.
 *
 * Example:
 *
 * Before:
 *
 *     positions = [A, B, A, C]
 *
 * After:
 *
 *     positions = [A, B, C]
 *
 * with:
 *
 *     remap = [0, 1, 0, 2]
 *
 * Every triangle index referencing the old vertices is then replaced by its
 * corresponding unique index.
 *****************************************************************************/

void remove_duplicate_vertices(Mesh & m) {
	Vec3 *pos = m.positions.data;
	size_t vtx_count = m.vertex_count();

	/*
	 * remap[i] gives the new index of old vertex i.
	 */
	TArray<uint32_t> remap(vtx_count);

	/*
	 * Identify unique vertices and build the index remapping.
	 */
	size_t new_count =
	    build_position_remap(pos, vtx_count, remap.data);


	/******************************************************************************
	 * Compact the vertex array.
	 *
	 * Each unique vertex is copied to its new compact position.
	 *
	 * The condition:
	 *
	 *     remap[i] <= i
	 *
	 * guarantees that vertices are only moved toward the beginning of the
	 * array, so this in-place compaction is safe.
	 *****************************************************************************/
	for (size_t i = 0; i < vtx_count; ++i) {
		assert(remap[i] <= i);
		pos[remap[i]] = pos[i];
	}

	m.positions.resize(new_count);


	/******************************************************************************
	 * Update mesh connectivity.
	 *
	 * Every old vertex index is replaced by its corresponding unique index.
	 *****************************************************************************/
	size_t idx_count = m.index_count();
	uint32_t *idx = m.indices.data;

	for (size_t i = 0; i < idx_count; ++i) {
		idx[i] = remap[idx[i]];
	}
}