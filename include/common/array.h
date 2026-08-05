#pragma once

/******************************************************************************
 * TArray : Simple dynamically-sized array.
 *
 * TArray is a lightweight dynamic array similar to std::vector.
 * It stores a contiguous sequence of elements and automatically manages
 * the underlying memory as the number of elements changes.
 *
 * The array is described by three members:
 *
 *   - size     : number of elements currently stored in the array;
 *   - capacity : number of elements that can be stored in the currently
 *                allocated memory without reallocating;
 *   - data     : pointer to the contiguous memory block containing the
 *                elements.
 *
 * Example:
 *
 *     TArray<double> a;
 *     a.push_back(1.0);
 *     a.push_back(2.0);
 *
 *     // a.size >= 2
 *     // a.capacity >= a.size
 *     // a.data points to the allocated memory
 *
 * When push_back() reaches the current capacity, the capacity is increased
 * (typically doubled) and the underlying memory is reallocated.
 *
 * Main operations:
 *
 *   - push_back(t)    : append an element to the end of the array;
 *   - resize(n)       : change the number of stored elements;
 *   - reserve(n)      : allocate enough memory for at least n elements
 *                       without changing the current size;
 *   - shrink_to_fit() : reduce the allocated capacity to the current size;
 *   - clear()         : remove all elements logically without releasing
 *                       the allocated memory;
 *   - operator[](i)   : access the element at index i.
 *
 * Copy construction and copy assignment are explicitly disabled because
 * TArray directly owns the memory pointed to by data. A shallow copy would
 * make two TArray objects point to the same memory and could result in
 * double-free errors when they are destroyed.
 *
 * T must be trivially copyable because the implementation manages the
 * memory directly using malloc/realloc/free. It is therefore intended
 * primarily for simple numerical types and lightweight data structures.
 *****************************************************************************/
#include <assert.h>
#include "sys_utils.h"

template <typename T>
    struct TArray {
        size_t size;
        size_t capacity;
        T *data;
        TArray();
        TArray(size_t size);
        TArray(size_t size, T val);
        TArray(const TArray<T> & other) = delete;
        TArray & operator=(const TArray<T> & other) = delete;
        ~TArray();
        T & operator[](size_t i);
        const T & operator[](size_t i) const;
        void push_back(const T & t);
        void resize(size_t size);
        void reserve(size_t capacity);
        void shrink_to_fit();
        void clear();
    };

template <typename T> TArray<T>::TArray() : size{0}, capacity{0}, data{nullptr} {}
template <typename T> TArray<T>::TArray(size_t size) : size{size}, capacity{size} {
                                        data = static_cast<T *>(safe_malloc(size * sizeof(T)));};
template <typename T> TArray<T>::TArray(size_t size, T val) : size{size}, capacity{size} {
	data = static_cast<T *>(safe_malloc(size * sizeof(T)));
	for (size_t i = 0; i < size; ++i) {
		data[i] = val;
	}
};

template <typename T> inline TArray<T>::~TArray() {
	size = 0;
	capacity = 0;
	free(data);
	data = nullptr;
}

template <typename T> inline T & TArray<T>::operator[](size_t i) {assert(i < size); return (data[i]);}
template <typename T> inline const T & TArray<T>::operator[](size_t i) const {assert(i < size); return (data[i]);}
template <typename T> inline void TArray<T>::push_back(const T & t) {
	if (size >= capacity) {
		capacity = capacity ? 2 * capacity : 1;
		data = static_cast<T *>(safe_realloc(data, capacity * sizeof(T)));
	}
	data[size++] = t;
}

template <typename T> void TArray<T>::resize(size_t size) {
	if (size > capacity) {
		data = static_cast<T *>(safe_realloc(data, size * sizeof(T)));
		capacity = size;
	}
	this->size = size;
}

template <typename T> void TArray<T>::reserve(size_t capacity) {
	if (capacity > this->capacity) {
		data = static_cast<T *>(safe_realloc(data, capacity * sizeof(T)));
		this->capacity = capacity;
	}
}

template <typename T> void TArray<T>::shrink_to_fit() {
	if (capacity > size) {
		data = static_cast<T *>(safe_realloc(data, size * sizeof(T)));
		this->capacity = size;
	}
}

template <typename T> inline void TArray<T>::clear() {size = 0;}