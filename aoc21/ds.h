#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>

#define SWAP(x, y) \
	do { \
		auto _swap = x; \
		x = y; \
		y = _swap; \
	} while (0)

#define MAKELT(x, y) \
	do { \
		if ((x) > (y)) { \
			SWAP(x, y); \
		} \
	} while (0)

#define MINIMIZE(x, y) \
	do { \
		if ((y) < (x)) { \
			x = y; \
		} \
	} while (0)

#define MAXIMIZE(x, y) \
	do { \
		if ((y) > (x)) { \
			x = y; \
		} \
	} while (0)

// Usage:
// uintptr_t currentIndex = 0;
// bool alreadyInArray = false;
// SEARCH(array.Length(), result = target - array[index].value;, currentIndex, alreadyInArray);
#define SEARCH(_count, _compar, _result, _found) \
	do { \
		if (_count) { \
			intptr_t low = 0; \
			intptr_t high = _count - 1; \
			\
			while (low <= high) { \
				uintptr_t index = ((high - low) >> 1) + low; \
				int result; \
				_compar \
				\
				if (result < 0) { \
					high = index - 1; \
				} else if (result > 0) { \
					low = index + 1; \
				} else { \
					_result = index; \
					_found = true; \
					break; \
				} \
			} \
			\
			if (high < low) { \
				_result = low; \
				_found = false; \
			} \
		} else { \
			_result = 0; \
			_found = false; \
		} \
	} while (0)

template <class T>
struct Array {
	T *array;
	size_t length, allocated;

	void Insert(T item, uintptr_t index) {
		if (length == allocated) {
			allocated = length * 2 + 1;
			array = (T *) realloc(array, allocated * sizeof(T));
		}

		length++;
		memmove(array + index + 1, array + index, (length - index - 1) * sizeof(T));
		array[index] = item;
	}

	void Delete(uintptr_t index, size_t count = 1) { 
		memmove(array + index, array + index + count, (length - index - count) * sizeof(T)); 
		length -= count;
	}

	void Add(T item) { Insert(item, length); }
	void Free() { free(array); array = nullptr; length = allocated = 0; }
	int Length() { return length; }
	T &First() { return array[0]; }
	T &Last() { return array[length - 1]; }
	T &operator[](uintptr_t index) { assert(index < length); return array[index]; }
	void Pop() { length--; }
	void DeleteSwap(uintptr_t index) { if (index != length - 1) array[index] = Last(); Pop(); }
};

template <class K, class V>
struct MapShort {
	struct { K key; V value; } *array;
	size_t used, capacity;

	uint64_t Hash(uint8_t *key, size_t keyBytes) {
		uint64_t hash = 0xCBF29CE484222325;
		for (uintptr_t i = 0; i < keyBytes; i++) hash = (hash ^ key[i]) * 0x100000001B3;
		return hash;
	}

	V *At(K key, bool createIfNeeded) {
		if (used + 1 > capacity / 2) {
			MapShort grow = {};
			grow.capacity = capacity ? (capacity + 1) * 2 - 1 : 15;
			*(void **) &grow.array = calloc(sizeof(array[0]), grow.capacity);
			for (uintptr_t i = 0; i < capacity; i++) if (array[i].key) grow.Put(array[i].key, array[i].value);
			free(array); *this = grow;
		}

		uintptr_t slot = Hash((uint8_t *) &key, sizeof(key)) % capacity;
		while (array[slot].key && array[slot].key != key) slot = (slot + 1) % capacity;

		if (!array[slot].key && createIfNeeded) {
			used++;
			array[slot].key = key;
		}

		return &array[slot].value;
	}

	bool Has(K key) {
		if (!capacity) return false;
		uintptr_t slot = Hash((uint8_t *) &key, sizeof(key)) % capacity;
		while (array[slot].key && array[slot].key != key) slot = (slot + 1) % capacity;
		return array[slot].key;
	}

	V Get(K key) { return *At(key, false); }
	void Put(K key, V value) { *At(key, true) = value; }
	void Free() { free(array); array = nullptr; used = capacity = 0; }
};

template <class T>
void ReverseEuclideanAlgorithm(T m, T n, T *a, T *b) {
	bool swap = false;
	if (m < n) { T t = n; n = m; m = t; swap = true; }

	T q = m / n;
	T r = m % n;

	if (r == 1) {
		*a = 1;
		*b = -q;
	} else {
		T a0, b0;
		ReverseEuclideanAlgorithm(n, r, &a0, &b0);
		*a = b0;
		*b = a0 - b0 * q;
	}

	if (swap) { T t = *a; *a = *b; *b = t; }
}

int CompareIntegersDescending(const void *_a, const void *_b) {
	const int *a = (const int *) _a;
	const int *b = (const int *) _b;
	return *a > *b ? -1 : *a < *b ? 1 : 0;
}

int CompareIntegers64Descending(const void *_a, const void *_b) {
	const int64_t *a = (const int64_t *) _a;
	const int64_t *b = (const int64_t *) _b;
	return *a > *b ? -1 : *a < *b ? 1 : 0;
}

int CompareIntegersAscending(const void *_a, const void *_b) {
	const int *a = (const int *) _a;
	const int *b = (const int *) _b;
	return *a > *b ? 1 : *a < *b ? -1 : 0;
}

int CompareIntegers64Ascending(const void *_a, const void *_b) {
	const int64_t *a = (const int64_t *) _a;
	const int64_t *b = (const int64_t *) _b;
	return *a > *b ? 1 : *a < *b ? -1 : 0;
}
