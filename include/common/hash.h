#pragma once

/******************************************************************************
 * Hash functions
 *
 * This file provides two variants of the MurmurHash2 function:
 *
 *     murmur2_32
 *         Hashes 32-bit values, processing 4 bytes at a time.
 *
 *     murmur2_64
 *         Hashes 64-bit values, processing 8 bytes at a time.
 *
 * Both functions take the current hash value and a new key as arguments.
 * This makes it possible to hash data made of several words by feeding the
 * output of one call into the next one.
 *
 * For the first call, the initial hash value can safely be set to 0.
 *
 * These functions are intended to produce a well-distributed hash value,
 * which is useful for hash tables and other data structures requiring
 * efficient key lookup.
 *
 *****************************************************************************/

#include <stdint.h>
#include <stdio.h>

// See https://en.wikipedia.org/wiki/MurmurHash


/******************************************************************************
 * MurmurHash2 for 32-bit values.
 *
 * The function processes one 32-bit key and combines it with the current
 * hash value.
 *
 * The returned value can be passed as the first argument of the next call
 * when hashing a sequence of 32-bit values.
 *
 *****************************************************************************/
inline uint32_t murmur2_32(uint32_t hash, uint32_t key) {
	const uint32_t m = 0x5bd1e995;
	const int r = 24;

	key *= m;
	key ^= key >> r;
	key *= m;

	hash *= m;
	hash ^= key;

	return hash;
}


/******************************************************************************
 * MurmurHash2 for 64-bit values.
 *
 * The function processes one 64-bit key and combines it with the current
 * hash value.
 *
 * The returned value can be passed as the first argument of the next call
 * when hashing a sequence of 64-bit values.
 *
 *****************************************************************************/
inline uint64_t murmur2_64(uint64_t hash, uint64_t key) {
	const uint64_t m = 0xc6a4a7935bd1e995llu;
	const int r = 47;

	key *= m;
	key ^= key >> r;
	key *= m;

	hash *= m;
	hash ^= key;

	return hash;
}