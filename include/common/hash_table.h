#pragma once

/******************************************************************************
 * HashTable : Open-addressing hash table.
 *
 * This file provides a simple hash table implementation supporting:
 *
 *     - insertion and update,
 *     - key lookup,
 *     - clearing,
 *     - reserving storage,
 *     - automatic table growth.
 *
 * Deletion is not supported.
 *
 * The table uses open addressing: keys and values are stored directly inside
 * two contiguous arrays. When several keys map to the same bucket, a probing
 * scheme is used to find another available bucket.
 *
 * The table capacity is always a power of two. This allows the bucket index
 * to be computed efficiently using a bit mask.
 *
 * The key and value types are templated:
 *
 *     K : key type
 *     V : value type
 *     H : hasher type
 *
 * The Hasher type must provide:
 *
 *     hash(key)      -> hash value
 *     is_empty(key)  -> tells whether a slot is empty
 *     is_equal(a,b)  -> compares two keys
 *
 * It must also provide an "empty_key" value used to mark unused slots.
 *
 *****************************************************************************/

#ifdef DEBUG
	#include <stdio.h>
#endif

#include <assert.h>
#include <stdlib.h>

#include "sys_utils.h"


/******************************************************************************
 * DefaultHasher
 *
 * A simple hasher intended for arithmetic types such as integers.
 *
 * The key itself is used as the hash value. More sophisticated hash functions
 * can be provided by defining a custom Hasher type.
 *
 * The value "empty_key" is reserved to mark unused entries in the table.
 *
 *****************************************************************************/

template <typename K> struct DefaultHasher {
	static constexpr K empty_key = ~static_cast<K>(0);

	size_t hash(K key) const
	{
		return static_cast<size_t>(key);
	}

	bool is_empty(K key) const
	{
		return (key == empty_key);
	}

	bool is_equal(K key1, K key2) const
	{
		return (key1 == key2);
	}
};


/******************************************************************************
 * HashTable
 *
 * Generic open-addressing hash table associating keys of type K with values
 * of type V.
 *
 * The table stores keys and values in two separate contiguous arrays:
 *
 *     keys[0]   <->   vals[0]
 *     keys[1]   <->   vals[1]
 *     ...
 *
 * The table automatically grows when its load factor becomes too large.
 *
 *****************************************************************************/

template <typename K, typename V, class H = DefaultHasher<K>> struct HashTable {

public:

	/* Public interface */

	HashTable(size_t expected_nkeys = 8, H hasher = H());
	~HashTable();

	/* Return the number of keys currently stored in the table. */
	size_t size() const;

	/* Remove all entries from the table. */
	void clear();

	/*
	 * Ensure that the table has enough buckets for the expected number
	 * of keys.
	 */
	void reserve(size_t expected_nkeys);

	/*
	 * Look up a key.
	 *
	 * Return a pointer to the associated value if the key exists,
	 * otherwise return nullptr.
	 */
	V *get(K key) const;

	/*
	 * Look up a key and insert it with alt_val if it does not exist.
	 *
	 * Return nullptr when a new entry was inserted. If the key already
	 * exists, return a pointer to its existing value.
	 */
	V *get_or_set(K key, V alt_val);

	/* Insert a key/value pair or update the value of an existing key. */
	void set_at(K key, V val);

	/* Return the fraction of buckets currently occupied by entries. */
	float load_factor() const;


protected:

	/* Internal storage */

	size_t _size;
	size_t _buckets;

	/* Contiguous arrays storing keys and their corresponding values. */
	K *keys;
	V *vals;

	/* Object responsible for hashing and comparing keys. */
	H hasher;


	/* Internal methods */

	/*
	 * Increase the number of buckets and reinsert all existing entries.
	 */
	void grow(size_t buckets);

	/*
	 * Check whether the current load factor is low enough to keep
	 * lookup and insertion efficient.
	 */
	bool load_factor_ok() const;
};


/******************************************************************************
 * Find the bucket associated with a key.
 *
 * The function first computes the initial bucket from the hash value.
 * If the bucket is occupied by another key, probing is used to search for
 * another bucket.
 *
 * The table uses quadratic probing to resolve collisions.
 *
 * The function returns either:
 *
 *     - an empty bucket where the key can be inserted, or
 *     - the bucket containing the requested key.
 *
 * The number of buckets must be a power of two.
 *
 *****************************************************************************/

template <typename K, typename H> static inline size_t hash_lookup(K *keys, size_t buckets, H hasher, K key) {
	assert(((buckets - 1) & buckets) == 0);

	size_t mask = buckets - 1;

	/* Compute the initial bucket from the hash value. */
	size_t bucket = hasher.hash(key) & mask;

	for (size_t probe = 0; probe < buckets; probe++) {

		/*
		 * Stop when an empty bucket is found or when the requested
		 * key is found.
		 */
		if (hasher.is_empty(keys[bucket]) ||
		    hasher.is_equal(keys[bucket], key)) {
			return bucket;
		}

		/* Quadratic probing to resolve a collision. */
		bucket = (bucket + probe + 1) & mask;
	}

	/* The table should never be completely full. */
	assert(false && "Table is full !\n");
	return 0;
}


/******************************************************************************
 * Constructor
 *
 * Allocate enough buckets for the expected number of keys while keeping
 * the initial load factor below the maximum allowed value.
 *
 * The number of buckets is rounded up to the next power of two.
 *
 *****************************************************************************/

template <typename K, typename V, typename H> HashTable<K, V, H>::HashTable(
	            size_t expected_keys, H hasher) : _size(0), _buckets(1), hasher(hasher) {
	while (_buckets < (3 * expected_keys / 2)) {
		_buckets *= 2;
	}

	keys = static_cast<K *>(
		malloc(_buckets * sizeof(K)));

	vals = static_cast<V *>(
		malloc(_buckets * sizeof(V)));

	assert(keys != nullptr && vals != nullptr);

	clear();
}


/******************************************************************************
 * Destructor
 *
 * Release the memory allocated for the key and value arrays.
 *
 *****************************************************************************/

template <typename K, typename V, typename H> HashTable<K, V, H>::~HashTable() {
	_buckets = 0;
	_size = 0;

	free(keys);
	keys = nullptr;

	free(vals);
	vals = nullptr;
}


/******************************************************************************
 * Return the number of entries currently stored in the table.
 *
 *****************************************************************************/

template <typename K, typename V, typename H> inline size_t HashTable<K, V, H>::size() const {return (_size);}


/******************************************************************************
 * Clear the table.
 *
 * All key slots are marked as empty and the number of stored entries
 * is reset to zero.
 *
 *****************************************************************************/

template <typename K, typename V, typename H> void HashTable<K, V, H>::clear() {
	for (size_t i = 0; i < _buckets; ++i) {
		keys[i] = hasher.empty_key;
		assert(hasher.is_empty(keys[i]));
	}

	_size = 0;
}


/******************************************************************************
 * Reserve storage for an expected number of keys.
 *
 * The number of buckets is chosen so that the expected number of keys
 * does not exceed the target load factor.
 *
 *****************************************************************************/

template <typename K, typename V, typename H> void HashTable<K, V, H>::reserve(size_t expected_keys) {
	size_t buckets = 1;

	while (buckets < (3 * expected_keys / 2)) {
		buckets *= 2;
	}

	grow(buckets);
}


/******************************************************************************
 * Look up a key.
 *
 * Return a pointer to the associated value if the key is present.
 * Return nullptr if the key is not present in the table.
 *
 *****************************************************************************/

template <typename K, typename V, typename H> inline V *HashTable<K, V, H>::get(K key) const {
	size_t bucket = hash_lookup(
		keys, _buckets, hasher, key);

	return hasher.is_empty(keys[bucket])
		? nullptr
		: &vals[bucket];
}


/******************************************************************************
 * Look up a key and insert it if it does not already exist.
 *
 * If the key is absent, a new entry is created with alt_val and nullptr
 * is returned.
 *
 * If the key already exists, no insertion is performed and a pointer to
 * the existing value is returned.
 *
 *****************************************************************************/

template <typename K, typename V, typename H> inline V *HashTable<K, V, H>::get_or_set(K key, V alt_val) {
	size_t bucket = hash_lookup(
		keys, _buckets, hasher, key);

	if (hasher.is_empty(keys[bucket])) {

		keys[bucket] = key;
		vals[bucket] = alt_val;
		_size++;

		if UNLIKELY (!load_factor_ok()) {
			grow(2 * _buckets);
			assert(load_factor_ok());
		}

		return nullptr;

	} else {

		return &vals[bucket];
	}
}


/******************************************************************************
 * Insert or update a key/value pair.
 *
 * If the key already exists, its associated value is replaced.
 * Otherwise, a new entry is inserted.
 *
 *****************************************************************************/

template <typename K, typename V, typename H> inline void HashTable<K, V, H>::set_at(K key, V val) {
	size_t bucket = hash_lookup(
		keys, _buckets, hasher, key);

	vals[bucket] = val;

	if (hasher.is_empty(keys[bucket])) {

		keys[bucket] = key;
		_size++;

		if UNLIKELY (!load_factor_ok()) {
			grow(2 * _buckets);
			assert(load_factor_ok());
		}
	}
}


/******************************************************************************
 * Return the current load factor.
 *
 * The load factor is the ratio between the number of stored entries and
 * the total number of buckets:
 *
 *     load_factor = size / buckets
 *
 * A high load factor generally results in more collisions and therefore
 * more probing during lookup.
 *
 *****************************************************************************/

template <typename K, typename V, typename H>
float HashTable<K, V, H>::load_factor() const
{
	return static_cast<float>(_size) / _buckets;
}


/******************************************************************************
 * Grow the hash table.
 *
 * Allocate larger key and value arrays and reinsert all existing entries
 * into the new table.
 *
 * Existing entries must be reinserted because changing the number of
 * buckets changes the mapping between hash values and bucket indices.
 *
 *****************************************************************************/

template <typename K, typename V, typename H> void HashTable<K, V, H>::grow(size_t new_buckets) {
	if (new_buckets <= _buckets)
		return;

#ifdef DEBUG
	printf("HashTable Grow to %zu!\n", new_buckets);
#endif

	assert((new_buckets & (new_buckets - 1)) == 0);

	K *newk = (K *)malloc(
		new_buckets * sizeof(*newk));

	V *newv = (V *)malloc(
		new_buckets * sizeof(*newv));

	/* Mark all buckets in the new table as empty. */
	for (size_t i = 0; i < new_buckets; ++i) {
		newk[i] = hasher.empty_key;
	}

	/*
	 * Reinsert all existing entries using the new bucket count.
	 */
	for (size_t probe = 0; probe < _buckets; ++probe) {

		const K key = keys[probe];

		if (hasher.is_empty(key))
			continue;

		size_t new_idx = hash_lookup(
			newk, new_buckets, hasher, key);

		assert(hasher.is_empty(newk[new_idx]));

		newk[new_idx] = key;
		newv[new_idx] = vals[probe];
	}

	/* Release the old storage. */
	free(keys);
	free(vals);

	/* Replace it with the new storage. */
	keys = newk;
	vals = newv;

	_buckets = new_buckets;
}


/******************************************************************************
 * Check whether the current load factor is acceptable.
 *
 * The implementation limits the load factor to approximately 66%.
 * Keeping some buckets empty reduces collisions and keeps lookup and
 * insertion efficient.
 *
 *****************************************************************************/

template <typename K, typename V, typename H> inline bool HashTable<K, V, H>::load_factor_ok() const {
	/* 66% load factor limit. */
	return (_buckets > _size + _size / 2);
}